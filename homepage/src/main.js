/*
 * QKiSS website: translations and the cuts of the page, the screenshot gallery and the download links.
 *
 * Loaded synchronously in <head> (not deferred), so that it can choose the
 * language and start downloading its translation before the page is first
 * painted. Everything that touches <body> waits for DOMContentLoaded.
 */
'use strict';

(() => {
  /** Registry of languages: `<script id="languages">` in index.html. */
  const LANGUAGES = JSON.parse(document.getElementById('languages').textContent);
  /** The language whose strings are written directly in index.html. */
  const SOURCE_LANGUAGE = LANGUAGES[0].id;
  const LANGUAGE_STORAGE_KEY = 'qkiss-homepage.language';
  /** The cut the visitor chose last (see currentCut). */
  const CUT_STORAGE_KEY = 'qkiss-homepage.cut';
  /** Set once the visitor agreed to the terms of the Director's Cut and their additional articles: then switching to it asks nothing again. */
  const TERMS_STORAGE_KEY = 'qkiss-homepage.cut-terms';
  /** Show the page anyway if the translation takes longer than this. */
  const LOADING_TIMEOUT_MS = 2000;
  const SCREENSHOTS_DIR = 'screenshots/';
  /** A screenshot with this extension is a video. It plays muted and looping, and only after the visitor picks it. */
  const VIDEO_EXTENSION = /\.webm$/i;
  /** The visitor turned on "reduce motion" in their system settings. Then a picked video does not start by itself: its play button starts it. */
  const REDUCED_MOTION = matchMedia('(prefers-reduced-motion: reduce)');
  /** The latest release of QKiSS, in the GitHub API. */
  const LATEST_RELEASE_API = 'https://api.github.com/repos/kuanyui/qkiss/releases/latest';
  /** The commits of QKiSS in the GitHub API. Followed by a tag name, the URL gives the commit of that tag. */
  const COMMITS_API = 'https://api.github.com/repos/kuanyui/qkiss/commits/';

  const languagesById = new Map(LANGUAGES.map((language) => [language.id, language]));
  const root = document.documentElement;

  // ---------------------------------------------------------------------------
  // Choosing the language
  // ---------------------------------------------------------------------------

  /** The registered language id equal to `tag`, ignoring case. */
  function findLanguage(tag) {
    const wanted = tag.toLowerCase();
    return LANGUAGES.find((language) => language.id.toLowerCase() === wanted)?.id;
  }

  /** The registered language id matching a browser language: `ja-JP` matches `ja`. */
  function matchBrowserLanguage(tag) {
    const wanted = tag.toLowerCase();
    return LANGUAGES.find((language) => {
      const id = language.id.toLowerCase();
      return wanted === id || wanted.startsWith(id + '-');
    })?.id;
  }

  /** A choice of the visitor, remembered from a previous visit, or null. */
  function readStored(key) {
    try {
      return localStorage.getItem(key);
    } catch {
      return null;  // Storage can be disabled by privacy settings.
    }
  }

  function store(key, value) {
    try {
      localStorage.setItem(key, value);
    } catch {
      // Then the choice is just not remembered for the next visit.
    }
  }

  /** `?lang=` in the URL, then the visitor's previous choice, then the browser's languages. */
  function chooseInitialLanguage() {
    for (const candidate of [new URLSearchParams(location.search).get('lang'), readStored(LANGUAGE_STORAGE_KEY)]) {
      const id = candidate && findLanguage(candidate);
      if (id) return id;
    }
    for (const tag of navigator.languages ?? [navigator.language]) {
      const id = matchBrowserLanguage(tag);
      if (id) return id;
    }
    return SOURCE_LANGUAGE;
  }

  /**
   * Same rule as ConfigFile::engOnly() of the app, applied to each browser
   * language. When it holds, the page stays in the source language (English)
   * and the language picker stays hidden.
   */
  function isEnglishOnly() {
    return (navigator.languages ?? [navigator.language]).some((tag) => {
      const l = [...tag.toLowerCase()].reverse().join('');
      return l.endsWith('hz') && l[0] === 'n' && l[1] === 'c';
    });
  }

  /** Where to look up a string: the language itself, its `fallback`s, then the source language. */
  function fallbackChain(id) {
    const chain = [];
    for (let language = languagesById.get(id);
         language && !chain.includes(language.id);
         language = languagesById.get(language.fallback)) {
      chain.push(language.id);
    }
    if (!chain.includes(SOURCE_LANGUAGE)) chain.push(SOURCE_LANGUAGE);
    return chain;
  }

  /** The BCP 47 tag for `<html lang>`. */
  function htmlLang(id) {
    return languagesById.get(id).lang ?? id;
  }

  // ---------------------------------------------------------------------------
  // Translations
  // ---------------------------------------------------------------------------

  /**
   * The cut of the page: "safe" (the Wholesome Cut, the default) or "danger" (the Director's Cut, once the visitor
   * agreed in #cut-dialog). A string can have one version per cut: an object `{ "safe": …, "danger": … }` in the
   * dictionaries. The dictionary of the source language has only "danger": its "safe" is the page itself.
   * See "The Director's Cut" in homepage/README.md.
   */
  let currentCut = readStored(CUT_STORAGE_KEY) === 'danger' ? 'danger' : 'safe';

  /** Language id -> Promise of its `{ key: translation }` dictionary. */
  const dictionaries = new Map();

  function loadDictionary(id) {
    // The strings of the source language are the page itself. Its dictionary has only the strings of the Director's
    // Cut, so it is downloaded only for the Director's Cut.
    if (id === SOURCE_LANGUAGE && currentCut === 'safe' && !dictionaries.has(id)) return Promise.resolve({});
    if (!dictionaries.has(id)) {
      dictionaries.set(id, fetch(`i18n/${id}.json`)
        .then((response) => {
          if (!response.ok) throw new Error(`HTTP ${response.status}`);
          return response.json();
        })
        .catch((error) => {
          console.error(`Cannot load the translation "${id}":`, error);
          dictionaries.delete(id);  // Try again the next time this language is chosen.
          return {};
        }));
    }
    return dictionaries.get(id);
  }

  /**
   * Every translatable string of the page: its key, its source-language value
   * (read from index.html before anything is translated), and how to set it.
   */
  const targets = [];

  /** Elements hidden by showUnlessEmpty(). */
  const emptied = new Set();

  /**
   * A string can be empty in one cut, like a footnote that only the Director's Cut has. Then its element is hidden:
   * for a footnote, the whole footnote. In index.html, such an element is empty and hidden (see homepage/README.md).
   */
  function showUnlessEmpty(element, value) {
    const box = element.closest('.footnotes > p') ?? element;
    if (value === '') {
      box.hidden = true;
      emptied.add(box);
    } else if (emptied.delete(box)) {
      box.hidden = false;
    }
  }

  function collectTargets() {
    for (const element of document.querySelectorAll('[data-i18n]')) {
      targets.push({
        key: element.dataset.i18n,
        source: element.textContent,
        apply: (value) => { element.textContent = value; showUnlessEmpty(element, value); },
      });
    }
    for (const element of document.querySelectorAll('[data-i18n-html]')) {
      targets.push({
        key: element.dataset.i18nHtml,
        source: element.innerHTML,
        apply: (value) => { element.innerHTML = value; showUnlessEmpty(element, value); },
      });
    }
    for (const element of document.querySelectorAll('[data-i18n-attr]')) {
      for (const item of element.dataset.i18nAttr.split(';')) {
        const [attribute, key] = item.split(':');
        targets.push({
          key,
          source: element.getAttribute(attribute),
          apply: (value) => { element.setAttribute(attribute, value); },
        });
      }
    }
  }

  /**
   * The string of `target` in the current cut: from the first dictionary of `chain` that has it, else the page itself.
   * An object without a version for the current cut does not count.
   */
  function lookUp(target, chain) {
    for (const dictionary of chain) {
      if (!Object.hasOwn(dictionary, target.key)) continue;
      const value = dictionary[target.key];
      if (typeof value === 'string') return value;
      if (Object.hasOwn(value, currentCut)) return value[currentCut];
    }
    return target.source;
  }

  /**
   * Number the footnotes of each `.footnotes` block from *1, skipping the hidden ones (a cut can hide some), on their
   * back links and on the references to them. A block whose footnotes are all hidden is hidden too.
   */
  function numberFootnotes() {
    for (const block of document.querySelectorAll('.footnotes')) {
      let count = 0;
      for (const footnote of block.querySelectorAll(':scope > p[id]')) {
        if (footnote.hidden) continue;
        const label = `*${++count}`;
        footnote.querySelector('.footnote-back').textContent = label;
        for (const ref of document.querySelectorAll(`.footnote-ref a[href="#${footnote.id}"]`)) ref.textContent = label;
      }
      block.hidden = count === 0;
    }
  }

  const englishOnly = isEnglishOnly();
  /** The language chosen last. A translation arriving for an earlier choice (of language or cut) is dropped. */
  let currentLanguage = englishOnly ? SOURCE_LANGUAGE : chooseInitialLanguage();

  /** Show the page in the language `id`, in the current cut. */
  async function setLanguage(id) {
    currentLanguage = id;
    const cut = currentCut;
    const chain = await Promise.all(fallbackChain(id).map(loadDictionary));
    if (currentLanguage !== id || currentCut !== cut) return;
    for (const target of targets) target.apply(lookUp(target, chain));
    numberFootnotes();
    root.lang = htmlLang(id);
    renderGallery();
    renderReleaseLinks();
    root.classList.remove('i18n-loading');
  }

  // ---------------------------------------------------------------------------
  // Language picker (@llselect/core)
  // ---------------------------------------------------------------------------

  /** A language the visitor picked: remember it, and put it in the URL for sharing. */
  function chooseLanguage(id) {
    store(LANGUAGE_STORAGE_KEY, id);
    const url = new URL(location.href);
    if (id === SOURCE_LANGUAGE) {
      url.searchParams.delete('lang');
    } else {
      url.searchParams.set('lang', id);
    }
    history.replaceState(history.state, '', url);
    setLanguage(id);
  }

  /** Flag + native name of a language: a row of the picker, and the picker when closed. */
  function createLanguageEl(language) {
    const el = document.createElement('span');
    el.className = 'language-option';
    el.lang = htmlLang(language.id);
    if (language.flag) {
      const flag = document.createElement('img');
      flag.className = 'language-flag';
      flag.src = `images/flags/${language.flag}.svg`;
      flag.alt = '';   // decorative: the name follows
      el.append(flag);
    }
    el.append(language.name);
    return el;
  }

  function setUpLanguagePicker() {
    const { llselect } = window;
    if (!llselect) {
      // The CDN is unreachable: no picker, but `?lang=` and the browser's languages still work.
      console.error('The language picker is unavailable: @llselect/core did not load.');
      return;
    }
    const picker = new llselect.LLSelectSingle(document.getElementById('language-select'), {
      ariaLabelledBy: 'language-label',
      compareFn: (a, b) => a.id === b.id,
      itemToStringFn: (language) => language.name,
      createItemContentElFn: createLanguageEl,
      createTriggerContentElFn: ({ chosenItem }) => (chosenItem ? createLanguageEl(chosenItem) : null),
      createTriggerArrowContentElFn: () => llselect.createChevronDownSvgEl(),
      onChange: (language, _previous, meta) => {
        // Only the visitor's own picks: the setChosenItem() below reports 'api'.
        if (meta.source === 'user' && language) chooseLanguage(language.id);
      },
    });
    picker.setItems(LANGUAGES);
    picker.setChosenItem(languagesById.get(currentLanguage));
    document.getElementById('language-picker').hidden = false;
  }

  // ---------------------------------------------------------------------------
  // Cut picker: the Wholesome Cut or the Director's Cut (see currentCut)
  // ---------------------------------------------------------------------------

  /** Show the current cut in the toggle of the picker, and mark it in the menu. */
  function renderCutPicker() {
    for (const option of document.querySelectorAll('#cut-toggle .cut-option')) {
      option.hidden = option.dataset.cut !== currentCut;
    }
    for (const item of document.querySelectorAll('#cut-menu .cut-menu-item')) {
      if (item.dataset.cut === currentCut) {
        item.setAttribute('aria-current', 'true');
      } else {
        item.removeAttribute('aria-current');
      }
    }
  }

  /** A cut the visitor chose (the Director's Cut only after agreeing): remember it, and show the page in it. */
  function chooseCut(cut) {
    currentCut = cut;
    store(CUT_STORAGE_KEY, cut);
    renderCutPicker();
    setLanguage(currentLanguage);
  }

  /** Name ("terms", "amendments") -> Promise of that document of the Director's Cut (legal/<name>.json), or of null. */
  const legalDocuments = new Map();

  function loadLegalDocument(name) {
    if (!legalDocuments.has(name)) {
      legalDocuments.set(name, fetch(`legal/${name}.json`)
        .then((response) => {
          if (!response.ok) throw new Error(`HTTP ${response.status}`);
          return response.json();
        })
        .catch((error) => {
          console.error(`Cannot load legal/${name}.json:`, error);
          legalDocuments.delete(name);  // Try again the next time.
          return null;
        }));
    }
    return legalDocuments.get(name);
  }

  /**
   * The HTML of a document of the Director's Cut in the current language: its chapters and sections, with either
   * articles written like a law (the terms: their paragraphs and subparagraphs are numbered and indented by style.css
   * as laws are), or numbered clauses (the additional articles: 1-2, 4-1-3…, numbered here from their position, with
   * the separator of the document in the current language, `numberSeparator`; a clause can write its own number as
   * `{number}`).
   * Its texts are HTML, like the strings of data-i18n-html, each one in the current language or else in its fallback
   * language (localize()). It starts with `titleHtml` and the date, as a contract does.
   */
  function renderLegalDocument(doc, titleHtml) {
    const title = (tag, value, number) => `<${tag}>`
      + (number ? `<span class="terms-number">${number}</span>` : '') + `${localize(value)}</${tag}>`;
    const article = (a, tag) => `<section class="terms-article">${title(tag, a.title)}`
      + a.body.map((block) => (block.items
        ? `<ol class="terms-items">${block.items.map((item) => `<li>${localize(item)}</li>`).join('')}</ol>`
        : `<p>${localize(block)}</p>`)).join('')
      + '</section>';
    const separator = doc.numberSeparator ? localize(doc.numberSeparator) : '-';
    const clauses = (list, prefix) => `<ol class="terms-clauses">${list.map((clause, i) => {
      const number = `${prefix}${separator}${i + 1}`;
      return `<li><span class="terms-number">${number}</span><div>${localize(clause).replaceAll('{number}', number)}`
        + (clause.clauses ? clauses(clause.clauses, number) : '') + '</div></li>';
    }).join('')}</ol>`;
    const content = (part, number, articleTag) => (part.clauses
      ? clauses(part.clauses, number)
      : part.articles.map((a) => article(a, articleTag)).join(''));
    const chapters = doc.chapters.map((chapter, c) => {
      const n = String(c + 1);
      const numbered = Boolean(chapter.clauses || chapter.sections?.some((section) => section.clauses));
      return `<section class="terms-chapter${numbered ? ' terms-outline' : ''}">${title('h3', chapter.title, numbered && n)}`
        + (chapter.sections
          ? chapter.sections.map((section, s) => `<section class="terms-section">${title('h4', section.title, numbered && `${n}${separator}${s + 1}`)}`
            + content(section, `${n}${separator}${s + 1}`, 'h5') + '</section>').join('')
          : content(chapter, n, 'h4'))
        + '</section>';
    });
    return `<header class="terms-head"><p class="terms-title">${titleHtml}</p>`
      + `<p class="terms-revised">${localize(doc.date)}</p></header>`
      + (doc.preamble ? `<p class="terms-preamble">${localize(doc.preamble)}</p>` : '')
      + chapters.join('')
      + (doc.closing ? `<p class="terms-closing">${localize(doc.closing)}</p>` : '');
  }

  /** Open a dialog of the cut picker. */
  function ask(dialog) {
    dialog.returnValue = '';   // what Escape and a click outside the dialog leave
    dialog.showModal();
  }

  /**
   * Set up a dialog that shows a document of the Director's Cut (legal/<name>.json), loaded when it opens. Its
   * button that continues (.terms-continue) waits until the end of the document is visible: after scrolling to it, or at once if the document
   * fits. Returns the function that loads the document, then opens the dialog at its top.
   */
  function setUpDocumentDialog(dialog, name) {
    const text = dialog.querySelector('.terms-text');
    const content = dialog.querySelector('.terms-content');
    const unavailable = dialog.querySelector('.terms-unavailable');
    const continueButton = dialog.querySelector('.terms-continue');
    const hint = dialog.querySelector('.terms-hint');
    const heading = dialog.querySelector('.cut-dialog-title [data-i18n]');   // also the title of the document
    /** Whether `content` holds the document: the button that continues waits for its end. */
    let loaded = false;

    new IntersectionObserver((entries) => {
      if (dialog.open && loaded && entries.some((entry) => entry.isIntersecting)) {
        continueButton.disabled = false;
        hint.hidden = true;
      }
    }, { root: text }).observe(dialog.querySelector('.terms-end'));

    return async () => {
      const doc = await loadLegalDocument(name);
      if (document.querySelector('dialog[open]')) return;   // Another dialog was opened in the meantime.
      if (doc) {
        content.innerHTML = renderLegalDocument(doc, heading.innerHTML);
      } else {
        content.replaceChildren();
      }
      loaded = doc !== null;
      unavailable.hidden = loaded;
      continueButton.disabled = true;
      hint.hidden = false;
      ask(dialog);
      text.scrollTop = 0;
    };
  }

  /**
   * The first switch to the Director's Cut asks three times: #cut-dialog, the terms (#terms-dialog), then their
   * additional articles (#amendments-dialog). Each one is a <form method="dialog">: only its first button closes it
   * with "danger". Escape, the other button and a click outside it (on the backdrop: the padding is on
   * .cut-dialog-body) close it without.
   */
  function setUpCutPicker() {
    const picker = document.getElementById('cut-picker');
    const toggle = document.getElementById('cut-toggle');
    const menu = document.getElementById('cut-menu');
    const warning = document.getElementById('cut-dialog');
    const terms = document.getElementById('terms-dialog');
    const amendments = document.getElementById('amendments-dialog');
    const openTerms = setUpDocumentDialog(terms, 'terms');
    const openAmendments = setUpDocumentDialog(amendments, 'amendments');

    function setMenuOpen(open) {
      toggle.setAttribute('aria-expanded', String(open));
      menu.hidden = !open;
    }

    toggle.addEventListener('click', () => setMenuOpen(menu.hidden));
    for (const item of menu.querySelectorAll('.cut-menu-item')) {
      item.addEventListener('click', () => {
        setMenuOpen(false);
        toggle.focus();
        if (item.dataset.cut === currentCut) return;
        if (item.dataset.cut === 'safe') {
          chooseCut('safe');
        } else if (readStored(TERMS_STORAGE_KEY) === 'agreed') {
          chooseCut('danger');
        } else {
          ask(warning);
          loadLegalDocument('terms');   // Start downloading the documents while the warning is read.
          loadLegalDocument('amendments');
        }
      });
    }
    for (const dialog of [warning, terms, amendments]) {
      dialog.addEventListener('click', (event) => {
        if (event.target === dialog) dialog.close();
      });
    }
    warning.addEventListener('close', () => {
      if (warning.returnValue === 'danger') openTerms();
    });
    terms.addEventListener('close', () => {
      if (terms.returnValue === 'danger') openAmendments();
    });
    amendments.addEventListener('close', () => {
      if (amendments.returnValue !== 'danger') return;
      store(TERMS_STORAGE_KEY, 'agreed');
      chooseCut('danger');
    });
    // A click anywhere else, or Escape, closes the menu.
    document.addEventListener('click', (event) => {
      if (!menu.hidden && !picker.contains(event.target)) setMenuOpen(false);
    });
    document.addEventListener('keydown', (event) => {
      if (event.key === 'Escape' && !menu.hidden) {
        setMenuOpen(false);
        toggle.focus();
      }
    });
    renderCutPicker();
    picker.hidden = false;
  }

  // ---------------------------------------------------------------------------
  // Screenshots: the main one above "What is KiSS?", the others in the showcase
  // ---------------------------------------------------------------------------

  /** Entries of screenshots/screenshots.json: null until loaded, [] when there are none. The first one is the main screenshot. */
  let screenshots = null;
  /** The screenshot shown in the showcase: an index in `screenshots`, never 0 (the main screenshot). */
  let showcaseIndex = 1;
  let lightbox = null;
  let lightboxIndex = 0;

  async function loadScreenshots() {
    try {
      const response = await fetch(`${SCREENSHOTS_DIR}screenshots.json`);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      screenshots = (await response.json()).screenshots;
    } catch (error) {
      console.warn('No screenshots:', error);
      screenshots = [];
    }
    renderGallery();
  }

  /** A value in screenshots.json: either a single string used for every language, or an object with one string per language ID. */
  function localize(value) {
    if (typeof value === 'string') return value;
    return value[fallbackChain(currentLanguage).find((id) => Object.hasOwn(value, id))];
  }

  /**
   * The media inside a screenshot button: an <img>, or for a video, a <video> that is never played.
   * It is decorative: the button is named after the caption.
   */
  function createThumbnail(fileName) {
    if (VIDEO_EXTENSION.test(fileName)) {
      const video = document.createElement('video');
      video.muted = true;
      video.playsInline = true;
      video.preload = 'metadata';   // so that the browser can show its first frame
      video.src = SCREENSHOTS_DIR + fileName;
      video.setAttribute('aria-hidden', 'true');
      return video;
    }
    const image = document.createElement('img');
    image.src = SCREENSHOTS_DIR + fileName;
    image.alt = '';
    image.loading = 'lazy';
    image.decoding = 'async';
    return image;
  }

  /** A button showing the screenshot at `index`, named after its caption. A video gets a play badge (see style.css). */
  function createScreenshotButton(index, onClick) {
    const fileName = localize(screenshots[index].image);
    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'screenshot-button';
    button.classList.toggle('is-video', VIDEO_EXTENSION.test(fileName));
    button.setAttribute('aria-label', localize(screenshots[index].caption));
    button.append(createThumbnail(fileName));
    button.addEventListener('click', onClick);
    return button;
  }

  function renderGallery() {
    if (screenshots === null) return;
    // The main screenshot. A click enlarges it.
    const main = document.getElementById('main-screenshot');
    if (screenshots.length > 0) main.replaceChildren(createScreenshotButton(0, () => openLightbox(0)));
    main.hidden = screenshots.length === 0;
    // The others: one thumbnail each. A click shows that screenshot in the showcase.
    const items = screenshots.slice(1).map((_, i) => {
      const item = document.createElement('li');
      item.append(createScreenshotButton(i + 1, () => showInShowcase(i + 1, true)));
      return item;
    });
    document.getElementById('gallery').replaceChildren(...items);
    document.getElementById('showcase').hidden = items.length === 0;
    document.getElementById('gallery-notice').hidden = screenshots.length > 0;
    if (items.length > 0) showInShowcase(showcaseIndex, false);
    if (lightbox.open) showScreenshot(lightboxIndex);
  }

  /**
   * Show a screenshot with one of two elements: `image` for a still image, `video` for a video.
   * The other one is hidden and emptied, so that a hidden video never keeps playing.
   * A video starts if `play` is true, unless the visitor prefers reduced motion (REDUCED_MOTION).
   */
  function showMedia(image, video, screenshot, play) {
    const url = SCREENSHOTS_DIR + localize(screenshot.image);
    const caption = localize(screenshot.caption);
    const isVideo = VIDEO_EXTENSION.test(url);
    image.hidden = isVideo;
    video.hidden = !isVideo;
    if (isVideo) {
      image.removeAttribute('src');
      // When it is the same video (for example after a change of language), it goes on instead of restarting.
      if (video.getAttribute('src') !== url) video.src = url;
      video.setAttribute('aria-label', caption);
      if (play && !REDUCED_MOTION.matches) video.play().catch(() => {});   // Else, its controls can start it.
    } else {
      video.removeAttribute('src');
      video.load();
      image.src = url;
      image.alt = caption;
    }
  }

  /** Show the screenshot at `index` in the showcase, and mark its thumbnail. `play` is true when the visitor picked it. */
  function showInShowcase(index, play) {
    showcaseIndex = index;
    const screenshot = screenshots[index];
    showMedia(document.getElementById('showcase-image'), document.getElementById('showcase-video'), screenshot, play);
    document.getElementById('showcase-caption').textContent = localize(screenshot.caption);
    // The thumbnails start at the second screenshot.
    document.querySelectorAll('#gallery button').forEach((button, i) => {
      if (i + 1 === index) {
        button.setAttribute('aria-current', 'true');
      } else {
        button.removeAttribute('aria-current');
      }
    });
  }

  function setUpShowcase() {
    // The showcase image and the Enlarge button both open the shown screenshot in the lightbox.
    document.getElementById('showcase-image').addEventListener('click', () => openLightbox(showcaseIndex));
    document.getElementById('showcase-enlarge').addEventListener('click', () => openLightbox(showcaseIndex));
  }

  function setUpLightbox() {
    lightbox = document.getElementById('lightbox');
    document.getElementById('lightbox-prev').addEventListener('click', () => showScreenshot(lightboxIndex - 1));
    document.getElementById('lightbox-next').addEventListener('click', () => showScreenshot(lightboxIndex + 1));
    document.getElementById('lightbox-close').addEventListener('click', () => lightbox.close());
    lightbox.addEventListener('click', (event) => {
      // A click anywhere but on the screenshot, its caption or a button closes it.
      if (!event.target.closest('img, video, figcaption, button')) lightbox.close();
    });
    lightbox.addEventListener('close', () => document.getElementById('lightbox-video').pause());
    lightbox.addEventListener('keydown', (event) => {
      if (event.target.closest('video')) return;   // When the video has the focus, the arrow keys seek in it.
      if (event.key === 'ArrowLeft') showScreenshot(lightboxIndex - 1);
      if (event.key === 'ArrowRight') showScreenshot(lightboxIndex + 1);
    });
  }

  function openLightbox(index) {
    document.getElementById('showcase-video').pause();   // Only one video plays at a time.
    showScreenshot(index);
    lightbox.showModal();
  }

  function showScreenshot(index) {
    const count = screenshots.length;
    lightboxIndex = (index + count) % count;
    const screenshot = screenshots[lightboxIndex];
    showMedia(document.getElementById('lightbox-image'), document.getElementById('lightbox-video'), screenshot, true);
    document.getElementById('lightbox-caption').textContent = localize(screenshot.caption);
    document.getElementById('lightbox-counter').textContent = `${lightboxIndex + 1} / ${count}`;
    document.getElementById('lightbox-prev').hidden = count < 2;
    document.getElementById('lightbox-next').hidden = count < 2;
  }

  // ---------------------------------------------------------------------------
  // Download links: the files of the latest release
  // ---------------------------------------------------------------------------

  /** The latest release, from the GitHub API. It is null until it is loaded, and stays null if loading fails. */
  let latestRelease = null;
  /**
   * The time of the commit of the latest release (ISO 8601), the time that the About dialog of the app shows too
   * (`git show --format=%ct`). It is null until it is loaded, and stays null if loading fails.
   */
  let latestReleaseCommitTime = null;

  async function loadLatestRelease() {
    try {
      const response = await fetch(LATEST_RELEASE_API);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      latestRelease = await response.json();
    } catch (error) {
      // Then the links keep opening the page of the latest release.
      console.warn('Cannot load the latest release:', error);
      return;
    }
    renderReleaseLinks();
    try {
      const response = await fetch(COMMITS_API + encodeURIComponent(latestRelease.tag_name));
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      latestReleaseCommitTime = (await response.json()).commit.committer.date;
    } catch (error) {
      // Then the version shows without the time.
      console.warn('Cannot load the commit of the latest release:', error);
      return;
    }
    renderReleaseLinks();
  }

  /**
   * A time in the time zone of the browser, in the format of the About dialog of the app:
   * "2026-10-11 (Sun) 21:45 GMT+8", with the weekday in the language of the page.
   */
  function formatLocalTime(isoTime) {
    const parts = Object.fromEntries(new Intl.DateTimeFormat(root.lang, {
      year: 'numeric', month: '2-digit', day: '2-digit', weekday: 'short',
      hour: '2-digit', minute: '2-digit', hourCycle: 'h23', timeZoneName: 'shortOffset',
    }).formatToParts(new Date(isoTime)).map((part) => [part.type, part.value]));
    return `${parts.year}-${parts.month}-${parts.day} (${parts.weekday}) ${parts.hour}:${parts.minute} ${parts.timeZoneName}`;
  }

  /** The file of the latest release whose name matches the regular expression in the `data-release-asset` of a link. */
  function releaseAsset(link) {
    const pattern = new RegExp(link.dataset.releaseAsset);
    return latestRelease.assets.find((candidate) => pattern.test(candidate.name));
  }

  /**
   * Point each `data-release-asset` link to its file in the latest release (releaseAsset()), and
   * show the toggle of its menu once the SHA-256 of that file is known. Also show the version.
   * setLanguage() calls it again, because a translated `data-i18n-html` string brings back
   * its links with their original href.
   */
  function renderReleaseLinks() {
    if (latestRelease === null) return;
    for (const link of document.querySelectorAll('a[data-release-asset]')) {
      const asset = releaseAsset(link);
      if (asset) link.href = asset.browser_download_url;
      link.parentElement.querySelector('.download-menu-toggle').hidden = !asset?.digest;
    }
    const versionLink = document.getElementById('download-version-link');
    versionLink.textContent = latestRelease.tag_name;
    versionLink.href = latestRelease.html_url;
    const commitTime = document.getElementById('download-version-time');
    commitTime.hidden = latestReleaseCommitTime === null;
    if (latestReleaseCommitTime !== null) {
      commitTime.dateTime = latestReleaseCommitTime;
      commitTime.textContent = formatLocalTime(latestReleaseCommitTime);
    }
    document.getElementById('download-version').hidden = false;
  }

  // ---------------------------------------------------------------------------
  // Download menus: the SHA-256 of each file of the latest release
  // ---------------------------------------------------------------------------

  /** The toggle whose menu is open, or null. */
  let openMenuToggle = null;

  /** Open or close the menu after `toggle`. Only one menu is open at a time. */
  function setMenuOpen(toggle, open) {
    if (open && openMenuToggle && openMenuToggle !== toggle) setMenuOpen(openMenuToggle, false);
    toggle.setAttribute('aria-expanded', String(open));
    toggle.nextElementSibling.hidden = !open;
    openMenuToggle = open ? toggle : null;
  }

  /**
   * Put a clone of #download-menu-template (a toggle and its menu) after each `data-release-asset`
   * link, joined to it as one split button. It runs before collectTargets(), so that the clones are
   * translated too. Their toggles stay hidden until renderReleaseLinks() knows each SHA-256.
   */
  function setUpDownloadMenus() {
    const template = document.getElementById('download-menu-template');
    for (const link of document.querySelectorAll('a[data-release-asset]')) {
      const split = document.createElement('span');
      split.className = 'split-button';
      link.replaceWith(split);
      split.append(link, template.content.cloneNode(true));
      const toggle = split.querySelector('.download-menu-toggle');
      toggle.classList.toggle('button-primary', link.classList.contains('button-primary'));
      toggle.addEventListener('click', () => setMenuOpen(toggle, toggle.getAttribute('aria-expanded') !== 'true'));
      split.querySelector('.download-menu-item').addEventListener('click', () => {
        setMenuOpen(toggle, false);
        showChecksum(releaseAsset(link));
      });
    }
    // A click anywhere else, or Escape, closes the open menu.
    document.addEventListener('click', (event) => {
      if (openMenuToggle && !event.target.closest('.split-button')) setMenuOpen(openMenuToggle, false);
    });
    document.addEventListener('keydown', (event) => {
      if (event.key === 'Escape' && openMenuToggle) {
        openMenuToggle.focus();
        setMenuOpen(openMenuToggle, false);
      }
    });
    const dialog = document.getElementById('checksum-dialog');
    document.getElementById('checksum-close').addEventListener('click', () => dialog.close());
    dialog.addEventListener('click', (event) => {
      if (event.target === dialog) dialog.close();   // a click on the backdrop (the padding is on .checksum-body)
    });
  }

  /** Show the SHA-256 of a file of the latest release. GitHub gives it as `digest`, like "sha256:df7523…". */
  function showChecksum(asset) {
    document.getElementById('checksum-file').textContent = asset.name;
    document.getElementById('checksum-value').textContent = asset.digest.replace(/^sha256:/, '');
    document.getElementById('checksum-dialog').showModal();
  }

  // ---------------------------------------------------------------------------
  // Navigation bar: underline the section being read
  // ---------------------------------------------------------------------------

  function setUpScrollSpy() {
    const header = document.querySelector('.site-header');
    const links = [...document.querySelectorAll('.site-nav a[href^="#"]')];
    const sections = links.map((link) => document.getElementById(link.hash.slice(1)));
    let scheduled = false;

    function update() {
      scheduled = false;
      // A section counts as "being read" once its top has scrolled above a line. The line is one
      // third of the way down the visible area under the header.
      const line = header.offsetHeight + (window.innerHeight - header.offsetHeight) / 3;
      let current = -1;
      sections.forEach((section, i) => {
        if (section.getBoundingClientRect().top <= line) current = i;
      });
      // At the bottom of the page, the last section may be too short to ever reach the line.
      const atBottom = window.scrollY > 0
        && window.scrollY + window.innerHeight >= document.documentElement.scrollHeight - 2;
      if (atBottom) current = sections.length - 1;
      links.forEach((link, i) => {
        if (i === current) {
          link.setAttribute('aria-current', 'true');
        } else {
          link.removeAttribute('aria-current');
        }
      });
    }

    function scheduleUpdate() {
      if (!scheduled) {
        scheduled = true;
        requestAnimationFrame(update);
      }
    }

    window.addEventListener('scroll', scheduleUpdate, { passive: true });
    window.addEventListener('resize', scheduleUpdate);
    // Translations and screenshots change the height of the sections.
    new ResizeObserver(scheduleUpdate).observe(document.getElementById('main'));
    update();
  }

  // ---------------------------------------------------------------------------
  // Q&A: a link to a question opens its answer
  // ---------------------------------------------------------------------------

  /** Open the <details> that is, or contains, the element with this id. */
  function openDetailsOf(id) {
    const details = id ? document.getElementById(id)?.closest('details') : null;
    if (details) details.open = true;
  }

  function setUpQuestionLinks() {
    // On click, before the browser scrolls to the target, so that it scrolls to the opened answer.
    document.addEventListener('click', (event) => {
      const link = event.target.closest('a[href^="#"]');
      if (link) openDetailsOf(link.hash.slice(1));
    });
    // A link from another page, like `…/#faq-llm`.
    openDetailsOf(location.hash.slice(1));
  }

  // ---------------------------------------------------------------------------
  // Start
  // ---------------------------------------------------------------------------

  if (currentLanguage !== SOURCE_LANGUAGE || currentCut === 'danger') {
    // Hide the page until it is translated, instead of flashing the source language (or the Wholesome Cut) first.
    root.classList.add('i18n-loading');
    setTimeout(() => root.classList.remove('i18n-loading'), LOADING_TIMEOUT_MS);
    fallbackChain(currentLanguage).forEach(loadDictionary);  // Start downloading right now.
  }

  document.addEventListener('DOMContentLoaded', () => {
    setUpDownloadMenus();   // before collectTargets(): the menus are translated too
    collectTargets();
    if (!englishOnly) setUpLanguagePicker();   // Otherwise it stays hidden.
    setUpCutPicker();
    setUpShowcase();
    setUpLightbox();
    setUpScrollSpy();
    setUpQuestionLinks();
    setLanguage(currentLanguage);
    loadScreenshots();
    loadLatestRelease();
  });
})();

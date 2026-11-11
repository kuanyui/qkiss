/*
 * QKiSS website: translations and the cuts of the page, the screenshot gallery and the download links.
 *
 * Loaded synchronously in <head> (not deferred), so that it can choose the
 * language and start downloading its translation before the page is first
 * painted. Everything that touches <body> waits for DOMContentLoaded.
 */
'use strict';

(() => {
  /**
   * Languages of this website. The first one is the source language: its strings are the ones written in index.html.
   * Every other language needs `i18n/<id>.json`. They are here rather than in index.html, so that the page itself
   * does not list them. build.py reads and checks the JSON between the two markers. See homepage/README.md.
   */
  const LANGUAGES = /* languages: */ [
    { "id": "en",    "name": "English",  "flag": "us" },
    { "id": "ja",    "name": "日本語",   "flag": "jp" },
    { "id": "zh-TW", "name": "台灣繁體", "flag": "tw" }
  ] /* :languages */;
  /** The language whose strings are written directly in index.html. */
  const SOURCE_LANGUAGE = LANGUAGES[0].id;
  const LANGUAGE_STORAGE_KEY = 'qkiss-homepage.language';
  /** The cut the visitor chose last (see currentCut). */
  const CUT_STORAGE_KEY = 'qkiss-homepage.cut';
  /** Set once the visitor agreed to the terms of the Director's Cut and their additional articles: then switching to it asks nothing again. */
  const TERMS_STORAGE_KEY = 'qkiss-homepage.cut-terms';
  /**
   * The additional articles of the Director's Cut take about 6 (English) to 10 minutes (Japanese, Taiwan Traditional)
   * to read. Scrolling through the whole of them in fewer seconds than this is skimming them (see setUpReadingCheck()).
   */
  const MIN_READING_SECONDS = 120;
  /** How far ahead of that pace the visitor may scroll, in seconds of reading: for a quick look at what follows. */
  const READING_HEAD_START_SECONDS = 10;
  /**
   * A visitor caught skimming the additional articles sooner than this is also told that even Kirito needs ten seconds
   * (#amendments-check-kirito).
   */
  const KIRITO_SECONDS = 10;
  /** For testing: typing these keys in the additional articles turns off the check of the pace until the page is loaded again. */
  const SKIP_READING_CHECK_KEYS = ['ArrowUp', 'ArrowUp', 'ArrowDown', 'ArrowDown', 'ArrowLeft', 'ArrowRight', 'ArrowLeft', 'ArrowRight', 'b', 'a'];
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
  /** The repository whose GitHub Actions attest the files of the releases, for `gh attestation verify -R`. */
  const ATTESTING_REPOSITORY = 'kuanyui/qkiss';
  /**
   * KaTeX, for the formulas of the Director's Cut. Like @llselect/core in index.html, the URLs name an exact version,
   * and `integrity` makes the browser refuse any other file (see homepage/README.md).
   */
  const KATEX_JS = 'https://cdn.jsdelivr.net/npm/katex@0.19.0/dist/katex.min.js';
  const KATEX_JS_INTEGRITY = 'sha384-QFFtAGzvvj+bfgCGxXJlNZZR1nXEZgvG8tDLCCY1F19xl20WlfTYgguB4VcNdxYk';
  const KATEX_CSS = 'https://cdn.jsdelivr.net/npm/katex@0.19.0/dist/katex.min.css';
  const KATEX_CSS_INTEGRITY = 'sha384-3rdsX6e5mueWyoweR9NIVmtEsUkokpBT/0ALqKKIBMr9j4qhHkaIkAcGgsE6uVlp';

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
  let currentCut = !isEnglishOnly() && readStored(CUT_STORAGE_KEY) === 'danger' ? 'danger' : 'safe';

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
   * for a footnote, the whole footnote, for a musing, the whole musing, and for a row of the chart of LLMs, the whole
   * row. In index.html, such an element is empty and hidden (see homepage/README.md).
   * collectTargets() also calls it with the strings of index.html, to put these elements in `emptied`. Otherwise, a
   * page that opens in the Director's Cut would never show them.
   */
  function showUnlessEmpty(element, value) {
    const box = element.closest('.footnotes > p, .musing, .llm-area') ?? element;
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
      showUnlessEmpty(element, element.textContent);
    }
    for (const element of document.querySelectorAll('[data-i18n-html]')) {
      targets.push({
        key: element.dataset.i18nHtml,
        source: element.innerHTML,
        apply: (value) => { element.innerHTML = value; showUnlessEmpty(element, value); },
      });
      showUnlessEmpty(element, element.innerHTML);
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
    if (cut === 'danger') typesetFormulas();
    root.lang = htmlLang(id);
    root.dataset.cut = cut;   // for the styles that differ between the cuts, like the banana of macOS in Download
    renderGallery();
    renderReleaseLinks();
    renderCutPicker();
    crtZoomPicker?.rerender();   // the name of the level "fit", in the language of the page
    fitHeroIntro();
    root.classList.remove('i18n-loading');
  }

  // ---------------------------------------------------------------------------
  // Formulas (KaTeX): only the Director's Cut has them
  // ---------------------------------------------------------------------------

  /** Load a script or a stylesheet from a CDN, refused by the browser unless it matches its `integrity`. */
  function loadChecked(tag, attributes) {
    return new Promise((resolve, reject) => {
      const element = Object.assign(document.createElement(tag), attributes, { crossOrigin: 'anonymous' });
      element.addEventListener('load', resolve);
      element.addEventListener('error', reject);
      document.head.append(element);
    });
  }

  /** Set once typesetFormulas() has started. */
  let formulasTypeset = null;

  /**
   * Typeset the formulas of the page: each element with `data-tex`, from the TeX in it. KaTeX is loaded only the first
   * time the page is shown in the Director's Cut. Until then, or if KaTeX cannot be loaded, an element keeps what
   * index.html has in it.
   */
  function typesetFormulas() {
    formulasTypeset ??= Promise.all([
      loadChecked('link', { rel: 'stylesheet', href: KATEX_CSS, integrity: KATEX_CSS_INTEGRITY }),
      loadChecked('script', { src: KATEX_JS, integrity: KATEX_JS_INTEGRITY }),
    ]).then(() => {
      for (const element of document.querySelectorAll('[data-tex]')) {
        window.katex.render(element.dataset.tex, element, { throwOnError: false });
      }
    }).catch(() => {});  // The elements keep what index.html has in them.
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
    const name = document.createElement('span');
    name.className = 'language-name';   // hidden in the closed picker when the header is too narrow (see fitHeader())
    name.textContent = language.name;
    el.append(name);
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
      createPopupListItemContentElFn: createLanguageEl,
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
  // Cut picker (@llselect/core): the Wholesome Cut or the Director's Cut (see currentCut)
  // ---------------------------------------------------------------------------

  /** The picker, once set up: null in an English-only browser, or when @llselect/core did not load. */
  let cutPicker = null;

  /** Icon + name of a cut, in the current language: a copy of its row in #cut-options. */
  function createCutEl(cut) {
    return document.querySelector(`#cut-options .cut-option[data-cut="${cut}"]`).cloneNode(true);
  }

  /** Show the current cut in the picker, in the current language. */
  function renderCutPicker() {
    if (!cutPicker) return;
    cutPicker.setChosenItem(currentCut);
    cutPicker.rerender();
    fitHeader();   // The names in both pickers, and the sections, may have changed.
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
   * fits. Returns `open`, the function that loads the document, then opens the dialog at its top, and `rewind`, which
   * shows the open dialog again at its top, or at the `scrollTop` given: its button that continues waits again, unless
   * the end of the document is visible there.
   */
  function setUpDocumentDialog(dialog, name) {
    const body = dialog.querySelector('.terms-body');
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

    /** Whether the end of the document is visible now. */
    function endVisible() {
      const end = dialog.querySelector('.terms-end').getBoundingClientRect();
      const box = text.getBoundingClientRect();
      return end.top < box.bottom && end.bottom > box.top;
    }

    function rewind(scrollTop = 0) {
      text.focus();   // The keyboard scrolls the document, as when the dialog opens.
      body.scrollTop = 0;   // On a short screen, the whole dialog scrolls (see style.css), and the focus scrolled it.
      text.scrollTop = scrollTop;
      // Checked here, because the IntersectionObserver above only reports changes: when the end was visible before
      // and still is, it says nothing.
      const waiting = !(loaded && endVisible());
      continueButton.disabled = waiting;
      hint.hidden = !waiting;
    }

    const open = async () => {
      const doc = await loadLegalDocument(name);
      if (document.querySelector('dialog[open]')) return;   // Another dialog was opened in the meantime.
      if (doc) {
        content.innerHTML = renderLegalDocument(doc, heading.innerHTML);
      } else {
        content.replaceChildren();
      }
      loaded = doc !== null;
      unavailable.hidden = loaded;
      ask(dialog);
      rewind();
    };
    return { open, rewind };
  }

  /**
   * Check that the visitor reads the document of a dialog of setUpDocumentDialog(), rather than scrolling through it:
   * once a second while it is open, and when its button that continues is pressed. The visitor skimmed it if they
   * scrolled further past where the check started than they could have read since then, at the pace of
   * MIN_READING_SECONDS for the whole of it, with a head start of READING_HEAD_START_SECONDS. Then the check stops,
   * and calls `onSkimmed` with the seconds since it started.
   * It measures the share of the document scrolled past, not pixels: so the pace depends neither on the language nor
   * on the size of the screen or of the text.
   * Returns `start`, which starts the check from where the document is shown now, and `readTop`, which returns how far
   * the visitor has surely read: the furthest `scrollTop` that a check found at the pace. For example, a visitor who
   * reads slowly to 20% of the document, then scrolls straight to its end, is caught, and `readTop` is at 20%.
   */
  function setUpReadingCheck(dialog, onSkimmed) {
    const text = dialog.querySelector('.terms-text');
    let startTime = 0;
    let startTop = 0;
    let readTop = 0;
    let timer = 0;
    /** Set by SKIP_READING_CHECK_KEYS. */
    let off = false;
    /** The last keys typed in the dialog, as many as SKIP_READING_CHECK_KEYS. */
    const typed = [];

    const seconds = () => (performance.now() - startTime) / 1000;
    const skimmed = () => !off && ((text.scrollTop - startTop) / text.scrollHeight) * MIN_READING_SECONDS
      > seconds() + READING_HEAD_START_SECONDS;

    function caught() {
      clearInterval(timer);
      onSkimmed(seconds());
    }

    dialog.querySelector('.terms-continue').addEventListener('click', (event) => {
      if (!skimmed()) return;
      event.preventDefault();   // The dialog stays open.
      caught();
    });
    dialog.addEventListener('close', () => clearInterval(timer));
    dialog.addEventListener('keydown', (event) => {
      typed.push(event.key.length === 1 ? event.key.toLowerCase() : event.key);
      if (typed.length > SKIP_READING_CHECK_KEYS.length) typed.shift();
      if (off || typed.join() !== SKIP_READING_CHECK_KEYS.join()) return;
      off = true;
      clearInterval(timer);
      console.info('The check of the reading pace is off until the page is loaded again.');
    });
    const start = () => {
      clearInterval(timer);
      if (off) return;
      startTime = performance.now();
      startTop = text.scrollTop;
      readTop = startTop;
      timer = setInterval(() => {
        if (skimmed()) {
          caught();
        } else {
          readTop = Math.max(readTop, text.scrollTop);
        }
      }, 1000);
    };
    return { start, readTop: () => readTop };
  }

  /**
   * The first switch to the Director's Cut asks three times: #cut-dialog, the terms (#terms-dialog), then their
   * additional articles (#amendments-dialog). Each one is a <form method="dialog">: only its first button closes it
   * with "danger". Escape, the other button and a click outside it (on the backdrop: the padding is on
   * .cut-dialog-body) close it without.
   * The additional articles also check that they are read (setUpReadingCheck()). A visitor caught skimming them is
   * asked whether they read them carefully (#amendments-check-dialog), then whether they are sure
   * (#amendments-recheck-dialog). Insisting twice refuses them (#amendments-refused-dialog): no more cut picker until
   * the page is loaded again. Any other answer sends them back to the additional articles: the first time they are
   * caught, to the top; after that, only to what they have surely read (readingCheck.readTop()).
   */
  function setUpCutPicker() {
    const { llselect } = window;
    if (!llselect) return;   // setUpLanguagePicker() reports it.
    const warning = document.getElementById('cut-dialog');
    const terms = document.getElementById('terms-dialog');
    const amendments = document.getElementById('amendments-dialog');
    const check = document.getElementById('amendments-check-dialog');
    const recheck = document.getElementById('amendments-recheck-dialog');
    const refused = document.getElementById('amendments-refused-dialog');
    const openTerms = setUpDocumentDialog(terms, 'terms').open;
    const { open: openAmendments, rewind: rewindAmendments } = setUpDocumentDialog(amendments, 'amendments');
    /** How many times the visitor was caught skimming the additional articles, since the page was loaded. */
    let timesCaught = 0;
    const readingCheck = setUpReadingCheck(amendments, (seconds) => {
      timesCaught++;
      document.getElementById('amendments-check-kirito').hidden = seconds >= KIRITO_SECONDS;
      ask(check);
    });

    cutPicker = new llselect.LLSelectSingle(document.getElementById('cut-select'), {
      ariaLabelledBy: 'cut-label',
      itemToStringFn: (cut) => createCutEl(cut).textContent,
      createPopupListItemContentElFn: createCutEl,
      createTriggerContentElFn: ({ chosenItem }) => (chosenItem ? createCutEl(chosenItem) : null),
      createTriggerArrowContentElFn: () => llselect.createChevronDownSvgEl(),
      onChange: (cut, _previous, meta) => {
        // Only the visitor's own picks: renderCutPicker() reports 'api'.
        if (meta.source !== 'user' || !cut || cut === currentCut) return;
        if (cut === 'safe') {
          chooseCut('safe');
        } else if (readStored(TERMS_STORAGE_KEY) === 'agreed') {
          chooseCut('danger');
        } else {
          renderCutPicker();   // still the Wholesome Cut, until the visitor agrees
          queueMicrotask(() => ask(warning));   // once the picker has closed its list
          loadLegalDocument('terms');   // Start downloading the documents while the warning is read.
          loadLegalDocument('amendments');
        }
      },
    });
    cutPicker.setItems(['safe', 'danger']);
    for (const dialog of [warning, terms, amendments, check, recheck, refused]) {
      dialog.addEventListener('click', (event) => {
        if (event.target === dialog) dialog.close();
      });
    }
    warning.addEventListener('close', () => {
      if (warning.returnValue === 'danger') openTerms();
    });
    terms.addEventListener('close', async () => {
      if (terms.returnValue !== 'danger') return;
      await openAmendments();
      if (amendments.open) readingCheck.start();
    });
    amendments.addEventListener('close', () => {
      if (amendments.returnValue !== 'danger') return;
      store(TERMS_STORAGE_KEY, 'agreed');
      chooseCut('danger');
    });
    /**
     * Back to the additional articles, to read them again: the first time the visitor is caught, from their top; after
     * that, from what they have surely read. The check of the pace starts again from there.
     */
    function readAgain() {
      // From now on, until the page is loaded again, the button that turns back points at itself five times as hard.
      // No-break spaces between the arrows: they stay on one line.
      amendments.querySelector('.pointing-arrow').textContent = '←\u00a0←\u00a0←\u00a0←\u00a0←';
      rewindAmendments(timesCaught === 1 ? 0 : readingCheck.readTop());
      readingCheck.start();
    }
    check.addEventListener('close', () => {
      if (check.returnValue === 'danger') {
        ask(recheck);
      } else {
        readAgain();
      }
    });
    recheck.addEventListener('close', () => {
      if (recheck.returnValue === 'danger') {
        ask(refused);
      } else {
        readAgain();
      }
    });
    refused.addEventListener('close', () => {
      amendments.close();
      // No more Director's Cut for this visitor, until the page is loaded again.
      document.getElementById('cut-picker').hidden = true;
      fitHeader();
    });
    renderCutPicker();
    document.getElementById('cut-picker').hidden = false;
  }

  // ---------------------------------------------------------------------------
  // Screenshots: the main one in "Why QKiSS?", the others in the showcase
  // ---------------------------------------------------------------------------

  /** Entries of screenshots/screenshots.json: null until loaded, [] when there are none. The first one is the main screenshot. */
  let screenshots = null;
  /** The screenshot shown in the showcase: an index in `screenshots`, never 0 (the main screenshot). */
  let showcaseIndex = 1;
  let lightbox = null;
  /**
   * What the lightbox goes through: the main screenshot alone, the screenshots of the showcase, or the windows of
   * #platforms (see setUpPlatforms()).
   */
  let lightboxItems = [];
  let lightboxIndex = 0;
  /** A touch that moves sideways at least this far (in CSS pixels) in the lightbox is a swipe, as over #platforms. */
  const LIGHTBOX_SWIPE_PX = 40;

  async function loadScreenshots() {
    let blocks = {};
    try {
      const response = await fetch(`${SCREENSHOTS_DIR}screenshots.json`);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const manifest = await response.json();
      screenshots = manifest.screenshots;
      blocks = manifest.blocks ?? {};
    } catch (error) {
      console.warn('No screenshots:', error);
      screenshots = [];
    }
    renderGallery();
    showBlocks(blocks);
  }

  /** What each block of the page with images from the wiki needs, once its images are set (see showBlocks()). */
  const BLOCK_SETUPS = {
    platforms: setUpPlatforms,
    crt: setUpCrt,
  };

  /**
   * A 2 x 2 AVIF at full color resolution (chroma 4:4:4): red and blue, in a checkerboard. A browser that decodes AVIF
   * only at half the color resolution blends them into purple, and it would lose the 1-pixel color dots of the CRT
   * Simulation (#crt) the same way.
   */
  const AVIF_444_PROBE = 'data:image/avif;base64,AAAAHGZ0eXBhdmlmAAAAAGF2aWZtaWYxbWlhZgAAANZtZXRhAAAAAAAAACFoZGxyAAAAAAAAAABwaWN0AAAAAAAAAAAAAAAAAAAAAA5waXRtAAAAAAABAAAAImlsb2MAAAAAREAAAQABAAAAAAD6AAEAAAAAAAAAoAAAACNpaW5mAAAAAAABAAAAFWluZmUCAAAAAAEAAGF2MDEAAAAAVmlwcnAAAAA4aXBjbwAAAAxhdjFDgUBiAAAAABRpc3BlAAAAAAAAAAIAAAACAAAAEHBpeGkAAAAAAwwMDAAAABZpcG1hAAAAAAAAAAEAAQOBAgMAAACobWRhdBIACghYADY0BDQaIDKRARAAAPzOpf96f///6h992SStTqqjopLnqjotTqr7pr7pnzmpLnmblpLnr7ppLnpLnp0o/////////3g5kjaL77C++wv/8b77C++wv/8b//G++wv/8b//G//xv/8b//G//xv44T/////9JdERSyg2Gg2Gge2Q2Gg2Gge2Qe2Q2Gge2Qe2Qe2Qe2Qe2Qe2Q3Wxj9Q=';
  /** Whether this browser shows AVIF at full color resolution (see AVIF_444_PROBE): a promise, made when first needed. */
  let avif444Supported = null;

  function supportsAvif444() {
    avif444Supported ??= (async () => {
      try {
        const image = new Image();
        image.src = AVIF_444_PROBE;
        await image.decode();
        const canvas = document.createElement('canvas');
        canvas.width = canvas.height = 2;
        const context = canvas.getContext('2d', { willReadFrequently: true });
        context.drawImage(image, 0, 0);
        const [red0, , blue0, , red1, , blue1] = context.getImageData(0, 0, 2, 1).data;
        // Roughly, since a browser may change the pixels of a canvas a little, against fingerprinting.
        return red0 > 160 && blue0 < 96 && blue1 > 160 && red1 < 96;
      } catch {
        return false;
      }
    })();
    return avif444Supported;
  }

  /** The URL of an image of a block: the first of its paths that this browser can show, or null. */
  async function blockImageUrl(paths) {
    for (const path of Array.isArray(paths) ? paths : [paths]) {
      if (!/\.avif$/i.test(path) || await supportsAvif444()) return SCREENSHOTS_DIR + path;
    }
    return null;
  }

  /**
   * The blocks of the page that show images from the wiki outside the gallery: `blocks` in screenshots.json, like
   * `{"platforms": {"linux": "os/linux.png"}}`. A block is an element with `data-block="platforms"`, hidden until its
   * images are set; each of its images is an `<img data-block-image="platforms.linux">`. build.py checks that the page
   * and screenshots.json name the same blocks and images. Without screenshots.json (a build without screenshots), the
   * blocks stay hidden.
   * An image may have several paths, the best first: the first one that this browser can show is used. An AVIF is used
   * only if the browser shows AVIF at full color resolution (see supportsAvif444()).
   * An image with `data-block-defer` is not loaded here: its URL goes to `data-block-src`, for the setup of its block.
   */
  async function showBlocks(blocks) {
    for (const block of document.querySelectorAll('[data-block]')) {
      const name = block.dataset.block;
      if (!Object.hasOwn(blocks, name)) continue;
      const images = [...block.querySelectorAll('img[data-block-image]')];
      const urls = await Promise.all(images.map((image) => blockImageUrl(blocks[name][image.dataset.blockImage.split('.')[1]])));
      if (urls.includes(null)) continue;   // a picture that this browser cannot show: the block stays hidden
      images.forEach((image, i) => {
        if (image.hasAttribute('data-block-defer')) image.dataset.blockSrc = urls[i];
        else image.src = urls[i];
      });
      block.hidden = false;
      BLOCK_SETUPS[name]?.();
    }
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
    // The main screenshot. A click enlarges it alone: it is not one of the screenshots of the showcase.
    const main = document.getElementById('main-screenshot');
    if (screenshots.length > 0) main.replaceChildren(createScreenshotButton(0, () => openLightbox(0, screenshots.slice(0, 1))));
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
    const url = screenshot.url ?? SCREENSHOTS_DIR + localize(screenshot.image);   // `url`: a window of #platforms
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
    // The showcase image and the Enlarge button both open the shown screenshot in the lightbox, which then goes
    // through the screenshots of the showcase, without the main one.
    const enlarge = () => openLightbox(showcaseIndex - 1, screenshots.slice(1));
    document.getElementById('showcase-image').addEventListener('click', enlarge);
    document.getElementById('showcase-enlarge').addEventListener('click', enlarge);
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
    lightbox.addEventListener('close', () => {
      // One video file in one video element at a time (see openLightbox()): the lightbox lets go of its video, and
      // the showcase gets its own back.
      const video = document.getElementById('lightbox-video');
      video.removeAttribute('src');
      video.load();
      if (!document.getElementById('showcase').hidden) showInShowcase(showcaseIndex, false);
    });
    lightbox.addEventListener('keydown', (event) => {
      if (event.target.closest('video')) return;   // When the video has the focus, the arrow keys seek in it.
      if (event.key === 'ArrowLeft') showScreenshot(lightboxIndex - 1);
      if (event.key === 'ArrowRight') showScreenshot(lightboxIndex + 1);
    });

    // A swipe on a touch screen: to the left for the next screenshot, to the right for the previous one.
    // Touch events, not pointer events as in #platforms: pointer events would need `touch-action: pan-y`, and then a
    // visitor who zoomed in on a screenshot could not move it sideways.
    // Also on the video: the videos are short, so going to another one matters more than seeking in it. Only with
    // one finger, and not on a button or while zoomed in (a move then pans the page).
    let swipeStart = null;
    lightbox.addEventListener('touchstart', (event) => {
      const zoomedIn = (window.visualViewport?.scale ?? 1) > 1.01;
      const touch = event.touches[0];
      swipeStart = event.touches.length === 1 && !zoomedIn && !event.target.closest('button')
        ? { x: touch.clientX, y: touch.clientY }
        : null;
    }, { passive: true });
    lightbox.addEventListener('touchcancel', () => {
      swipeStart = null;
    });
    lightbox.addEventListener('touchend', (event) => {
      if (swipeStart === null) return;
      const touch = event.changedTouches[0];
      const dx = touch.clientX - swipeStart.x;
      const dy = touch.clientY - swipeStart.y;
      swipeStart = null;
      if (Math.abs(dx) < LIGHTBOX_SWIPE_PX || Math.abs(dx) < Math.abs(dy)) return;
      showScreenshot(lightboxIndex + (dx < 0 ? 1 : -1));
    });
  }

  function openLightbox(index, items) {
    // Only one video plays at a time, and only one element holds a video file: the showcase lets go of its video until
    // the lightbox closes. In Firefox, while the showcase still held the same file, the lightbox could show only the
    // controls of the video (seen with an AV1 video on Linux).
    const showcaseVideo = document.getElementById('showcase-video');
    showcaseVideo.removeAttribute('src');
    showcaseVideo.load();
    lightboxItems = items;
    showScreenshot(index);
    // FIXME: A known bug (Firefox 156 only): right after refreshing the page WITHOUT ANY SCROLL, open the lightbox modal, and the page, dunno why, scrolls to another place.
    lightbox.showModal();
  }

  function showScreenshot(index) {
    const count = lightboxItems.length;
    lightboxIndex = (index + count) % count;
    const screenshot = lightboxItems[lightboxIndex];
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
  // Download menus: the SHA-256 and the artifact attestation of each file of the latest release
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
      split.querySelector('.download-menu-checksum').addEventListener('click', () => {
        setMenuOpen(toggle, false);
        showChecksum(releaseAsset(link));
      });
      split.querySelector('.download-menu-attestation').addEventListener('click', () => {
        setMenuOpen(toggle, false);
        showAttestation(releaseAsset(link));
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
    for (const dialog of document.querySelectorAll('.checksum-dialog')) {
      dialog.querySelector('.checksum-close').addEventListener('click', () => dialog.close());
      dialog.addEventListener('click', (event) => {
        if (event.target === dialog) dialog.close();   // a click on the backdrop (the padding is on .checksum-body)
      });
    }
    for (const radio of document.querySelectorAll('input[name="checksum-system"]')) {
      radio.addEventListener('change', () => {
        checksumSystem = radio.value;
        renderChecksumCommand();
      });
    }
  }

  /**
   * The system of the command in #checksum-dialog: the one of the browser at first, then the one chosen there, for the
   * other files too. Phones and the other systems get the one of Linux.
   */
  let checksumSystem = /Windows/.test(navigator.userAgent) ? 'windows'
    : /Mac/.test(navigator.userAgent) ? 'macos' : 'linux';

  /** The file of the latest release shown in #checksum-dialog. */
  let checksumAsset = null;

  /**
   * Show the SHA-256 of a file of the latest release, and a command that checks the downloaded file against it. GitHub
   * gives the SHA-256 as `digest`, like "sha256:df7523…".
   */
  function showChecksum(asset) {
    checksumAsset = asset;
    document.getElementById('checksum-file').textContent = asset.name;
    document.getElementById('checksum-value').textContent = asset.digest.replace(/^sha256:/, '');
    document.querySelector(`input[name="checksum-system"][value="${checksumSystem}"]`).checked = true;
    renderChecksumCommand();
    document.getElementById('checksum-dialog').showModal();
  }

  /** Write the command of #checksum-dialog for `checksumSystem`: PowerShell on Windows, `shasum` on macOS, `sha256sum` on the others. */
  function renderChecksumCommand() {
    const { name } = checksumAsset;
    const hash = checksumAsset.digest.replace(/^sha256:/, '');
    const powershell = checksumSystem === 'windows';
    document.getElementById('checksum-terminal').hidden = powershell;
    document.getElementById('checksum-powershell').hidden = !powershell;
    document.getElementById('checksum-command').textContent = powershell
      ? `(Get-FileHash '${name}' -Algorithm SHA256).Hash -eq '${hash}'`
      : `echo '${hash}  ${name}' | ${checksumSystem === 'macos' ? 'shasum -a 256' : 'sha256sum'} --check`;
  }

  /** Show the command that checks the artifact attestation of a file of the latest release, with the GitHub CLI. */
  function showAttestation(asset) {
    document.getElementById('attestation-file').textContent = asset.name;
    document.getElementById('attestation-command').textContent =
      `gh attestation verify ${asset.name} -R ${ATTESTING_REPOSITORY}`;
    document.getElementById('attestation-dialog').showModal();
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
  // Footer: Donate shows the author's note about donations
  // ---------------------------------------------------------------------------

  /** #donation-dialog closes like the dialogs of the download menus (see setUpDownloadMenus()). */
  function setUpDonation() {
    document.getElementById('donation-open').addEventListener('click', () => {
      document.getElementById('donation-dialog').showModal();
    });
  }

  // ---------------------------------------------------------------------------
  // The pickers and the dialogs of the cuts (controls.html)
  // ---------------------------------------------------------------------------

  /**
   * Put controls.html into the page: the two pickers into the header, the dialogs at the end of <body>. Returns
   * whether it did. Without them, this page load stays in the Wholesome Cut, because nothing could switch back from
   * the Director's Cut.
   */
  async function insertControls() {
    if (!controlsLoading) return false;
    const html = await controlsLoading;
    if (html === null) {
      currentCut = 'safe';
      return false;
    }
    const template = document.createElement('template');
    template.innerHTML = html;
    const headerTools = document.querySelector('.header-tools');
    for (const element of [...template.content.children]) {
      (element.localName === 'dialog' ? document.body : headerTools).append(element);
    }
    return true;
  }

  /**
   * The logo, the sections and the two pickers share one row of the header when they fit on it. Otherwise, as on a
   * phone, the sections go to the next row (.is-stacked, see style.css). When the logo and the pickers still do not
   * fit, hide the name in the closed pickers: first the name of the language, leaving its flag, then the name of the
   * cut, leaving its icon.
   */
  function fitHeader() {
    const bar = document.querySelector('.header-bar');
    const [brand, nav, tools] = ['.brand', '.site-nav', '.header-tools'].map((selector) => bar.querySelector(selector));
    const width = (element) => element.getBoundingClientRect().width;
    const gap = parseFloat(getComputedStyle(bar).columnGap);
    const pickers = [document.getElementById('language-picker'), document.getElementById('cut-picker')];
    // Measured on one row, with both names, first.
    bar.classList.remove('is-stacked');
    for (const picker of pickers) picker.classList.remove('is-name-hidden');
    if (width(brand) + gap + width(nav) + gap + width(tools) <= width(bar)) return;
    bar.classList.add('is-stacked');
    for (const picker of pickers) {
      if (width(brand) + gap + width(tools) <= width(bar)) return;
      picker.classList.add('is-name-hidden');
    }
  }

  // ---------------------------------------------------------------------------
  // Hero: the speech bubble over the name
  // ---------------------------------------------------------------------------

  /**
   * Make the bubble only as wide as its longest line. When its text wraps, `text-wrap: balance` makes the lines
   * shorter than the box, which still takes all the width it has: style.css cannot shrink it to them.
   */
  function fitHeroIntro() {
    const intro = document.querySelector('.hero-intro');
    intro.style.width = '';
    const range = document.createRange();
    range.selectNodeContents(intro);
    const lines = [...range.getClientRects()];
    if (lines.length < 2) return;   // on one line, `width: fit-content` fits it already
    const style = getComputedStyle(intro);
    const padding = parseFloat(style.paddingLeft) + parseFloat(style.paddingRight);
    intro.style.width = `${Math.ceil(Math.max(...lines.map((line) => line.width)) + padding)}px`;
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

  /**
   * The text of controls.html, downloaded right now too. It is null if it could not be downloaded, or not within
   * LOADING_TIMEOUT_MS: the same time limit as the page hidden above, so that the page never waits longer for it.
   * Not for an English-only browser.
   */
  const controlsLoading = englishOnly ? null : Promise.race([
    fetch('controls.html')
      .then((response) => {
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        return response.text();
      })
      .catch((error) => {
        console.error('controls.html:', error);
        return null;
      }),
    new Promise((resolve) => setTimeout(() => resolve(null), LOADING_TIMEOUT_MS)),
  ]);

  /** How long a window of #platforms takes to fade out, before it comes back at the other end of the stack (see style.css). */
  const PLATFORM_FADING_MS = 300;
  /** How long a window of #platforms takes to move in the stack, or to fade in (see style.css). The next move waits for it. */
  const PLATFORM_MOVING_MS = 350;
  /** A touch that moves sideways at least this far (in CSS pixels) over #platforms is a swipe. */
  const PLATFORM_SWIPE_PX = 40;
  /** When nobody touches #platforms, its front window goes to the back by itself this often. */
  const PLATFORM_AUTO_MS = 7000;

  /**
   * The windows of #platforms, stacked like Flip 3D of Windows Vista. A click on the stack, or a swipe to the left,
   * sends the front window to the back. A right click, or a swipe to the right, brings the back window to the front.
   * When nobody touches it, it goes on by itself every PLATFORM_AUTO_MS, except for a visitor who prefers reduced
   * motion.
   * The other windows move with the transitions of style.css. The button under the stack opens the front window in
   * the lightbox, where the arrows go through the three.
   */
  function setUpPlatforms() {
    const stage = document.querySelector('.platforms-stage');
    const platforms = [...stage.querySelectorAll('.platform')];
    const order = [...platforms];   // front to back
    let moving = false;

    const place = (platform, slot) => platform.style.setProperty('--slot', slot);

    const sendFrontToBack = () => {
      if (moving) return;
      const front = order.shift();
      order.push(front);
      if (REDUCED_MOTION.matches) {
        order.forEach(place);
        return;
      }
      moving = true;
      front.classList.add('platform-leaving');
      order.slice(0, -1).forEach(place);
      setTimeout(() => {
        // Jump to the back while invisible, then fade in there.
        front.classList.add('platform-entering');
        front.classList.remove('platform-leaving');
        place(front, order.length - 1);
        front.getBoundingClientRect();   // apply that before the fade in starts
        front.classList.remove('platform-entering');
        setTimeout(() => {
          moving = false;
        }, PLATFORM_MOVING_MS);
      }, PLATFORM_FADING_MS);
    };

    const sendBackToFront = () => {
      if (moving) return;
      const back = order.pop();
      order.unshift(back);
      if (REDUCED_MOTION.matches) {
        order.forEach(place);
        return;
      }
      moving = true;
      back.classList.add('platform-fading');
      order.slice(1).forEach((platform, i) => place(platform, i + 1));
      setTimeout(() => {
        // Jump under the front while invisible, then slide up to the front and fade in.
        back.classList.add('platform-entering', 'platform-leaving');
        back.classList.remove('platform-fading');
        place(back, 0);
        back.getBoundingClientRect();   // apply that before the slide starts
        back.classList.remove('platform-entering', 'platform-leaving');
        setTimeout(() => {
          moving = false;
        }, PLATFORM_MOVING_MS);
      }, PLATFORM_FADING_MS);
    };

    // A swipe on a touch screen. The click that may follow it is part of it.
    let swipeStart = null;
    let swiped = false;
    stage.addEventListener('pointerdown', (event) => {
      swiped = false;
      swipeStart = event.pointerType === 'touch' ? { x: event.clientX, y: event.clientY } : null;
    });
    stage.addEventListener('pointercancel', () => {
      swipeStart = null;
    });
    stage.addEventListener('pointerup', (event) => {
      if (swipeStart === null) return;
      const dx = event.clientX - swipeStart.x;
      const dy = event.clientY - swipeStart.y;
      swipeStart = null;
      if (Math.abs(dx) < PLATFORM_SWIPE_PX || Math.abs(dx) < Math.abs(dy)) return;
      swiped = true;
      if (dx < 0) sendFrontToBack();
      else sendBackToFront();
    });

    stage.addEventListener('click', (event) => {
      if (swiped || event.target.closest('a')) return;   // a link, like the banana, goes where it points
      sendFrontToBack();
    });
    stage.addEventListener('contextmenu', (event) => {
      if (event.target.closest('a')) return;   // a link keeps its menu (open in a new tab, copy the link…)
      event.preventDefault();
      sendBackToFront();
    });
    // The lightbox shows each window with the text of its image, in the language of the page at that time.
    document.querySelector('.platforms-enlarge').addEventListener('click', () => {
      const items = platforms.map((platform) => {
        const image = platform.querySelector(':scope > img');
        return { url: image.getAttribute('src'), caption: image.alt };
      });
      openLightbox(platforms.indexOf(order[0]), items);
    });

    // Going on by itself: only while the stack is on the screen, and nobody points at it, has the keyboard focus in
    // it, or looks at the lightbox. Pointing at it and leaving it starts the wait again.
    const container = document.getElementById('platforms');
    let onScreen = false;
    let touched = false;
    let autoTimer = null;
    const restartAuto = () => {
      clearTimeout(autoTimer);
      autoTimer = setTimeout(() => {
        if (onScreen && !touched && !document.hidden && !lightbox.open && !REDUCED_MOTION.matches) sendFrontToBack();
        restartAuto();
      }, PLATFORM_AUTO_MS);
    };
    new IntersectionObserver(([entry]) => {
      onScreen = entry.isIntersecting;
    }).observe(container);
    const touch = () => {
      touched = true;
    };
    const release = () => {
      touched = false;
      restartAuto();
    };
    container.addEventListener('pointerenter', touch);
    container.addEventListener('pointerleave', release);
    container.addEventListener('focusin', touch);
    container.addEventListener('focusout', release);
    restartAuto();
  }

  /**
   * The zoom levels of #crt, in its picker: 'fit' shows the whole picture; a number is how many CSS pixels show each
   * pixel of the screenshots.
   */
  const CRT_ZOOMS = ['fit', 1, 2, 3, 4];
  /** The zoom of #crt at first: enough to see the dots of the CRT Simulation. */
  const CRT_FIRST_ZOOM = 2;
  /** How far an arrow key moves the line of #crt, as a share of its width. */
  const CRT_KEY_STEP = 0.05;
  /** The zoom picker of #crt, once set up: null when @llselect/core did not load. */
  let crtZoomPicker = null;

  /**
   * #crt: the CRT Simulation, compared with the original like in an image editor. None is on the left of the line, the
   * chosen preset on its right. The line follows a drag (a pointer, or the arrow keys once it has the keyboard focus).
   * The zoom picker enlarges both pictures; then they scroll in the window, by dragging them with a mouse, or as usual
   * (wheel, finger). A preset shows once its picture is decoded. Soft and Glow load when the block comes near (or,
   * when the browser is asked to save data, when they are chosen).
   */
  function setUpCrt() {
    const block = document.getElementById('crt');
    const view = block.querySelector('.crt-view');
    const frame = block.querySelector('.crt-frame');
    const stage = block.querySelector('.crt-stage');
    const original = block.querySelector('.crt-original');
    const presets = Object.fromEntries([...block.querySelectorAll('.crt-preset')].map((image) => [image.dataset.preset, image]));
    const buttons = [...block.querySelectorAll('.crt-preset-button')];
    const sideNames = [...block.querySelectorAll('.crt-side-right [data-preset]')];
    const divider = block.querySelector('.crt-divider');
    const pictureWidth = Number(original.getAttribute('width'));
    const pictureHeight = Number(original.getAttribute('height'));
    let wanted = 'sharp';   // the preset chosen last: one that finishes loading later is shown only if still wanted
    let split = 0.5;        // where the line is, as a share of the width of the window
    let zoom = 'fit';

    const load = (image) => {
      if (!image.getAttribute('src')) image.src = image.dataset.blockSrc;
      return image.decode().catch(() => {});
    };

    // The line, and the clip of the preset in the stage (which may be scrolled): drawn once per frame at most.
    let frameRequest = 0;
    const drawSplit = () => {
      cancelAnimationFrame(frameRequest);
      frameRequest = requestAnimationFrame(() => {
        const x = split * frame.clientWidth;
        view.style.setProperty('--split', `${x}px`);
        stage.style.setProperty('--split', `${frame.scrollLeft + x}px`);
        divider.setAttribute('aria-valuenow', String(Math.round(split * 100)));
      });
    };
    const clamp = (value) => Math.min(1, Math.max(0, value));

    const choose = async (next) => {
      wanted = next;
      buttons.forEach((button) => button.setAttribute('aria-pressed', String(button.dataset.preset === next)));
      await load(presets[next]);
      if (wanted !== next) return;
      for (const [name, image] of Object.entries(presets)) image.hidden = name !== next;
      sideNames.forEach((name) => {
        name.hidden = name.dataset.preset !== next;
      });
    };
    buttons.forEach((button) => button.addEventListener('click', () => choose(button.dataset.preset)));

    if (!navigator.connection?.saveData) {
      const observer = new IntersectionObserver((entries) => {
        if (!entries.some((entry) => entry.isIntersecting)) return;
        observer.disconnect();
        Object.values(presets).forEach(load);
      }, { rootMargin: '600px 0px' });
      observer.observe(block);
    }

    // The zoom keeps the center of the window where it is.
    const setZoom = (next) => {
      const centerX = (frame.scrollLeft + frame.clientWidth / 2) / stage.offsetWidth;
      const centerY = (frame.scrollTop + frame.clientHeight / 2) / stage.offsetHeight;
      zoom = next;
      stage.style.width = zoom === 'fit' ? '' : `${pictureWidth * zoom}px`;
      stage.style.height = zoom === 'fit' ? '' : `${pictureHeight * zoom}px`;
      block.classList.toggle('crt-zoomed', zoom !== 'fit');
      frame.scrollLeft = centerX * stage.offsetWidth - frame.clientWidth / 2;
      frame.scrollTop = centerY * stage.offsetHeight - frame.clientHeight / 2;
      drawSplit();
    };

    const moveLineTo = (clientX) => {
      split = clamp((clientX - frame.getBoundingClientRect().left) / frame.clientWidth);
      drawSplit();
    };
    divider.addEventListener('pointerdown', (event) => {
      divider.setPointerCapture(event.pointerId);
      moveLineTo(event.clientX);
      event.preventDefault();   // no text selection, no scrolling
    });
    divider.addEventListener('pointermove', (event) => {
      if (divider.hasPointerCapture(event.pointerId)) moveLineTo(event.clientX);
    });
    divider.addEventListener('keydown', (event) => {
      const steps = { ArrowLeft: -CRT_KEY_STEP, ArrowDown: -CRT_KEY_STEP, ArrowRight: CRT_KEY_STEP, ArrowUp: CRT_KEY_STEP };
      if (event.key === 'Home') split = 0;
      else if (event.key === 'End') split = 1;
      else if (Object.hasOwn(steps, event.key)) split = clamp(split + steps[event.key]);
      else return;
      event.preventDefault();
      drawSplit();
    });

    // Dragging the enlarged pictures with a mouse scrolls them, like a hand tool. A finger scrolls them by itself.
    let pan = null;
    frame.addEventListener('pointerdown', (event) => {
      if (event.pointerType !== 'mouse' || event.button !== 0 || zoom === 'fit') return;
      pan = { x: event.clientX, y: event.clientY, left: frame.scrollLeft, top: frame.scrollTop };
      frame.setPointerCapture(event.pointerId);
      frame.classList.add('crt-panning');
      event.preventDefault();
    });
    frame.addEventListener('pointermove', (event) => {
      if (pan === null) return;
      frame.scrollLeft = pan.left - (event.clientX - pan.x);
      frame.scrollTop = pan.top - (event.clientY - pan.y);
    });
    const endPan = () => {
      pan = null;
      frame.classList.remove('crt-panning');
    };
    frame.addEventListener('pointerup', endPan);
    frame.addEventListener('pointercancel', endPan);
    frame.addEventListener('scroll', drawSplit, { passive: true });
    window.addEventListener('resize', drawSplit);

    // The zoom picker shows each level as its copy in #zoom-options; its trigger adds a 16 px magnifier.
    const { llselect } = window;
    if (llselect) {
      const zoomEl = (level) => block.querySelector(`#zoom-options [data-zoom="${level}"]`).cloneNode(true);
      crtZoomPicker = new llselect.LLSelectSingle(document.getElementById('zoom-select'), {
        ariaLabelledBy: 'zoom-label',
        itemToStringFn: (level) => zoomEl(level).textContent,
        createPopupListItemContentElFn: zoomEl,
        createTriggerContentElFn: ({ chosenItem }) => {
          if (chosenItem == null) return null;
          const el = zoomEl(chosenItem);
          el.prepend(Object.assign(document.createElement('img'), { src: 'images/icons/16/system-search.svg', alt: '', width: 16, height: 16 }));
          return el;
        },
        createTriggerArrowContentElFn: () => llselect.createChevronDownSvgEl(),
        onChange: (level, _previous, meta) => {
          if (meta.source === 'user' && level != null) setZoom(level);
        },
      });
      crtZoomPicker.setItems(CRT_ZOOMS);
      crtZoomPicker.setChosenItem(CRT_FIRST_ZOOM);
      block.querySelector('.crt-zoom').hidden = false;
    }
    setZoom(CRT_FIRST_ZOOM);
    frame.scrollTo(0, 0);   // at first, the top left corner of the window
  }

  document.addEventListener('DOMContentLoaded', async () => {
    setUpDownloadMenus();   // before collectTargets(): the menus are translated too
    const hasControls = await insertControls();   // before collectTargets() too, for the same reason
    collectTargets();
    if (hasControls) {
      setUpLanguagePicker();
      setUpCutPicker();
      window.addEventListener('resize', fitHeader);
    }
    window.addEventListener('resize', fitHeroIntro);
    setUpShowcase();
    setUpLightbox();
    setUpScrollSpy();
    setUpQuestionLinks();
    setUpDonation();
    setLanguage(currentLanguage);
    loadScreenshots();
    loadLatestRelease();
  });
})();

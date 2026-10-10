/*
 * QKiSS website: translations and the screenshot gallery.
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
  const STORAGE_KEY = 'qkiss-homepage.language';
  /** Show the page anyway if the translation takes longer than this. */
  const LOADING_TIMEOUT_MS = 2000;
  const SCREENSHOTS_DIR = 'screenshots/';

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

  function readStoredLanguage() {
    try {
      return localStorage.getItem(STORAGE_KEY);
    } catch {
      return null;  // Storage can be disabled by privacy settings.
    }
  }

  function storeLanguage(id) {
    try {
      localStorage.setItem(STORAGE_KEY, id);
    } catch {
      // Then the choice is just not remembered for the next visit.
    }
  }

  /** `?lang=` in the URL, then the visitor's previous choice, then the browser's languages. */
  function chooseInitialLanguage() {
    for (const candidate of [new URLSearchParams(location.search).get('lang'), readStoredLanguage()]) {
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

  /** Language id -> Promise of its `{ key: translation }` dictionary. */
  const dictionaries = new Map();

  function loadDictionary(id) {
    if (id === SOURCE_LANGUAGE) return Promise.resolve({});  // Its strings are the page itself.
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

  function collectTargets() {
    for (const element of document.querySelectorAll('[data-i18n]')) {
      targets.push({
        key: element.dataset.i18n,
        source: element.textContent,
        apply: (value) => { element.textContent = value; },
      });
    }
    for (const element of document.querySelectorAll('[data-i18n-html]')) {
      targets.push({
        key: element.dataset.i18nHtml,
        source: element.innerHTML,
        apply: (value) => { element.innerHTML = value; },
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

  const englishOnly = isEnglishOnly();
  /** The language chosen last. A translation arriving for an earlier choice is dropped. */
  let currentLanguage = englishOnly ? SOURCE_LANGUAGE : chooseInitialLanguage();

  async function setLanguage(id) {
    currentLanguage = id;
    const chain = await Promise.all(fallbackChain(id).map(loadDictionary));
    if (currentLanguage !== id) return;
    for (const target of targets) {
      const dictionary = chain.find((candidate) => Object.hasOwn(candidate, target.key));
      target.apply(dictionary ? dictionary[target.key] : target.source);
    }
    root.lang = htmlLang(id);
    renderGallery();
    root.classList.remove('i18n-loading');
  }

  // ---------------------------------------------------------------------------
  // Language picker (@llselect/core)
  // ---------------------------------------------------------------------------

  /** A language the visitor picked: remember it, and put it in the URL for sharing. */
  function chooseLanguage(id) {
    storeLanguage(id);
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
  // Screenshot gallery
  // ---------------------------------------------------------------------------

  /** Entries of screenshots/screenshots.json: null until loaded, [] when there are none. */
  let screenshots = null;
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

  function renderGallery() {
    if (screenshots === null) return;
    const items = screenshots.map((screenshot, index) => {
      const caption = localize(screenshot.caption);
      const image = document.createElement('img');
      image.src = SCREENSHOTS_DIR + localize(screenshot.image);
      image.alt = caption;
      image.loading = 'lazy';
      image.decoding = 'async';
      const button = document.createElement('button');
      button.type = 'button';
      button.className = 'gallery-thumb';
      button.append(image);
      button.addEventListener('click', () => openLightbox(index));
      const figcaption = document.createElement('figcaption');
      figcaption.textContent = caption;
      const figure = document.createElement('figure');
      figure.append(button, figcaption);
      const item = document.createElement('li');
      item.append(figure);
      return item;
    });
    document.getElementById('gallery').replaceChildren(...items);
    document.getElementById('gallery-notice').hidden = screenshots.length > 0;
    if (lightbox.open) showScreenshot(lightboxIndex);
  }

  function setUpLightbox() {
    lightbox = document.getElementById('lightbox');
    document.getElementById('lightbox-prev').addEventListener('click', () => showScreenshot(lightboxIndex - 1));
    document.getElementById('lightbox-next').addEventListener('click', () => showScreenshot(lightboxIndex + 1));
    document.getElementById('lightbox-close').addEventListener('click', () => lightbox.close());
    lightbox.addEventListener('click', (event) => {
      // A click anywhere but on the screenshot, its caption or a button closes it.
      if (!event.target.closest('img, figcaption, button')) lightbox.close();
    });
    lightbox.addEventListener('keydown', (event) => {
      if (event.key === 'ArrowLeft') showScreenshot(lightboxIndex - 1);
      if (event.key === 'ArrowRight') showScreenshot(lightboxIndex + 1);
    });
  }

  function openLightbox(index) {
    showScreenshot(index);
    lightbox.showModal();
  }

  function showScreenshot(index) {
    const count = screenshots.length;
    lightboxIndex = (index + count) % count;
    const screenshot = screenshots[lightboxIndex];
    const caption = localize(screenshot.caption);
    const image = document.getElementById('lightbox-image');
    image.src = SCREENSHOTS_DIR + localize(screenshot.image);
    image.alt = caption;
    document.getElementById('lightbox-caption').textContent = caption;
    document.getElementById('lightbox-counter').textContent = `${lightboxIndex + 1} / ${count}`;
    document.getElementById('lightbox-prev').hidden = count < 2;
    document.getElementById('lightbox-next').hidden = count < 2;
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
  // Start
  // ---------------------------------------------------------------------------

  if (currentLanguage !== SOURCE_LANGUAGE) {
    // Hide the page until it is translated, instead of flashing the source language first.
    root.classList.add('i18n-loading');
    setTimeout(() => root.classList.remove('i18n-loading'), LOADING_TIMEOUT_MS);
    fallbackChain(currentLanguage).forEach(loadDictionary);  // Start downloading right now.
  }

  document.addEventListener('DOMContentLoaded', () => {
    collectTargets();
    if (!englishOnly) setUpLanguagePicker();   // Otherwise it stays hidden.
    setUpLightbox();
    setUpScrollSpy();
    setLanguage(currentLanguage);
    loadScreenshots();
  });
})();

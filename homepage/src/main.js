/*
 * QKiSS website: translations, the screenshot gallery and the download links.
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
  /** A screenshot with this extension is a video. It plays muted and looping, and only after the visitor picks it. */
  const VIDEO_EXTENSION = /\.webm$/i;
  /** The visitor turned on "reduce motion" in their system settings. Then a picked video does not start by itself: its play button starts it. */
  const REDUCED_MOTION = matchMedia('(prefers-reduced-motion: reduce)');
  /** The latest release of QKiSS, in the GitHub API. */
  const LATEST_RELEASE_API = 'https://api.github.com/repos/kuanyui/qkiss/releases/latest';

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
    renderReleaseLinks();
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
  }

  /**
   * Point each `data-release-asset` link to the file of the latest release whose name matches
   * the regular expression in that attribute. Also show the version.
   * setLanguage() calls it again, because a translated `data-i18n-html` string brings back
   * its links with their original href.
   */
  function renderReleaseLinks() {
    if (latestRelease === null) return;
    for (const link of document.querySelectorAll('a[data-release-asset]')) {
      const pattern = new RegExp(link.dataset.releaseAsset);
      const asset = latestRelease.assets.find((candidate) => pattern.test(candidate.name));
      if (asset) link.href = asset.browser_download_url;
    }
    const versionLink = document.getElementById('download-version-link');
    versionLink.textContent = latestRelease.tag_name;
    versionLink.href = latestRelease.html_url;
    document.getElementById('download-version').hidden = false;
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
    setUpShowcase();
    setUpLightbox();
    setUpScrollSpy();
    setLanguage(currentLanguage);
    loadScreenshots();
    loadLatestRelease();
  });
})();

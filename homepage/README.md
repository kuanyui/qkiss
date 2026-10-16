# QKiSS Homepage

The official website of QKiSS: a single static page in plain HTML, CSS and JavaScript, without frameworks or npm. `build.py` checks and assembles it. It uses the Python standard library, and `rsvg-convert` for the link-preview image.
Its one dependency, [@llselect/core](https://gitlab.com/kuanyui/llselect) for the language and cut pickers, is loaded from a CDN (see [The language picker](#the-language-picker)).
It is published on both GitHub Pages and GitLab Pages, by the same script.

```
homepage/
├── build.py        # Checks and assembles the site, and is the dev server; uses the Python standard library and rsvg-convert
└── src/            # Copied into the site as is
    ├── index.html  # The page. English strings are written here.
    ├── style.css
    ├── main.js     # Translations, the language and cut pickers, the screenshot gallery and the download links
    ├── i18n/       # One JSON file per language other than English, and en.json for the English of the Director's Cut
    ├── legal/      # The documents of the Director's Cut: terms.json, and amendments.json for their additional articles
    └── images/
        ├── distros/              # Logos of the Linux distributions, from Simple Icons. See images/distros/README.md.
        ├── icons/                # Icons from Tango and Nome, and the mark of Tango Remixer. See images/icons/README.md.
        ├── marks/                # GitHub and GitLab marks for the buttons, from Simple Icons. See images/marks/README.md.
        ├── tech/                 # Logos of the technologies in Q&A (Python, C++, CMake, GitHub Actions). See images/tech/README.md.
        ├── og-image.svg          # The link-preview image (see below)
        └── windows.svg           # The icon of the Windows card in Download, drawn for this website
```

The download links point to the files of the latest release on GitHub. `main.js` looks them up with the GitHub API when the page opens. If that fails, the links open the page of the latest release instead.
Each of these download buttons also gets a menu, which shows the SHA-256 of its file, also from the GitHub API (`digest`).
The page also shows the version of that release, and the time of its commit: in the time zone of the visitor, and in the format of the About dialog of the app.

At build time, `build.py` also copies:

- The app icons it uses from `resources/appicon/` (see `APP_ICONS` in `build.py`), so they are never duplicated.
- The flags of the languages from `resources/flags/svg/`, with the MIT license notice of flag-icons.
- The screenshot images, from the wiki (see below).

It also converts `src/images/og-image.svg`, the link-preview image (`og:image`), into `images/og-image.png` with a white background, because link previews do not show SVG:

- It uses `rsvg-convert`, from the package `librsvg2-bin` (Debian, Ubuntu), `rsvg-convert` (openSUSE), `librsvg2-tools` (Fedora) or `librsvg` (Arch Linux).
- The PNG must be 1280 × 640 pixels, matching `og:image:width` and `og:image:height` in `index.html`. To use another size, change those values and `OG_IMAGE_SIZE` in `build.py`.
- Some platforms cache link-preview images, so a link shared again may still show the old image.

## Local preview

```sh
make homepage-dev-server
```

It builds the site, serves it on port 8000 on every interface, and prints the URLs to open (LAN addresses included, so the host of a VM can open it too).
A change to `homepage/src/`, `homepage/screenshots.json`, the app icons, the flags or the screenshot images rebuilds it: reload the page.
When a change breaks a check, the last good build stays served and the errors are printed.
Ctrl-C stops it.

The screenshots come from the wiki by default. `HOMEPAGE_SCREENSHOTS` picks another source:

| Command | Screenshots |
|---|---|
| `make homepage-dev-server` | From the wiki. Each run downloads it (or updates it) into `generated/wiki/` first. |
| `make homepage-dev-server HOMEPAGE_SCREENSHOTS=` | None. Nothing is downloaded, so it works offline. |
| `make homepage-dev-server HOMEPAGE_SCREENSHOTS=~/proj/qkiss.wiki/screenshots` | The screenshots in that folder, used as they are. For example, use your own clone of the wiki to see new screenshots before you push them. |

`make homepage-build` writes the site into `generated/homepage/` instead, the way CI builds it. `HOMEPAGE_SCREENSHOTS` works the same way there.
`make help` lists the variables (`HOMEPAGE_OUT`, `HOMEPAGE_PORT`, `HOMEPAGE_SCREENSHOTS`).
Opening `src/index.html` directly (`file://`) does not work, because translations and screenshots are fetched.

## Screenshots

Screenshot images are not stored in this repository, so they never bloat its history, however often they are replaced:

- `homepage/screenshots.json`, here, lists them: their order and captions.
- The images live in the `screenshots/` folder of the wiki, a separate Git repository on each host: https://github.com/kuanyui/qkiss.wiki.git and https://gitlab.com/kuanyui/qkiss.wiki.git. The web pages of a wiki only list its pages, not these files.

At deploy time, CI clones the wiki of its own host with `--depth 1`, and `build.py` copies `homepage/screenshots.json` and the images it lists into the site.
To clone another URL, set the CI variable `SCREENSHOTS_REPO`.

The wiki must exist before the first deployment: create its first page on the web, then clone it and add `screenshots/`.

### `homepage/screenshots.json`

```json
{
  "screenshots": [
    {
      "image": "main-window.png",
      "caption": {
        "en": "Main window",
        "ja": "メインウィンドウ",
        "zh-TW": "主視窗"
      }
    },
    {
      "image": {
        "en": "preferences-en.png",
        "ja": "preferences-ja.png"
      },
      "caption": "Preferences"
    }
  ]
}
```

- The first screenshot is the main one. It is shown large, above “What is KiSS?”.
- The others go to the Screenshots section, in the order of the array. There, a showcase shows one screenshot large. At first, it is the second one of the array. The thumbnails under the showcase pick another one.
- There must be at least 2 screenshots: the main one, and one for the Screenshots section.
- `image` and `caption` are both required. Each one is either a single string, used for every language, or an object with one string per language ID, like `{ "en": "...", "ja": "..." }`, which must include `en`. A language missing from the object falls back like any other string (see [Translations](#translations)).
- The caption is also what screen readers read for the screenshot.
- File names may only contain letters, digits, `.`, `_` and `-`, with one of the extensions `png`, `jpg`, `jpeg`, `webp`, `avif`, `gif`, `webm`. They must be regular files in the `screenshots/` folder of the wiki.
- A `webm` file is a video. Keep it short: it plays muted, in a loop. Its thumbnail is the video, paused at its start, with a play badge. A video plays only after the visitor picks it, so only one video plays at a time. If the visitor turned on “reduce motion” in their system settings, a picked video does not start by itself: its play button starts it.
- The page shows the same files scaled down, so keep them reasonably small (for example about 1600 px wide, WebP or an optimized PNG).

`build.py` rejects anything else, and then nothing is deployed. It checks `homepage/screenshots.json` even without `--screenshots`, so a local preview catches its mistakes too.

### Adding or replacing a screenshot

1. Push the image to `screenshots/` in the wiki, on both hosts. Push it first: a `screenshots.json` listing an image the wiki does not have stops the build.
2. Then:
   - To add a screenshot, list it in `homepage/screenshots.json` and push this repository. That deploys.
   - To replace an image under the same name, deploy again by hand. GitHub: Actions → Homepage → Run workflow (an edit of a wiki page on the web also deploys, through the `gollum` event; whether a `git push` to the wiki does is not documented). GitLab: Build → Pipelines → Run pipeline, on `master`.

## Translations

English is the source language: its strings are written in `index.html`, marked with a key:

| Attribute | Replaces | Example |
|---|---|---|
| `data-i18n="key"` | the text | `<h2 data-i18n="features.title">Highlights</h2>` |
| `data-i18n-html="key"` | the inner HTML, keeping links and markup | `<p data-i18n-html="hero.lead">… <a href="…">KiSS</a> …</p>` |
| `data-i18n-attr="attr:key;attr:key"` | attributes | `<nav aria-label="Sections" data-i18n-attr="aria-label:nav.label">` |

Every other language has `src/i18n/<id>.json`: a flat object `{ "key": "translation" }`.
A translation of a `data-i18n-html` key is HTML too, so links can point to localized pages.
A translation can also have one version per cut of the page: see [The Director's Cut](#the-directors-cut).

The page picks the language from, in order:

1. `?lang=<id>` in the URL.
2. The visitor's previous choice in the language picker (stored in `localStorage`).
3. The browser's preferred languages: `ja-JP` matches `ja`.
4. Otherwise, English.

### Adding a language

1. Add it to `<script id="languages">` in `index.html`:

   ```json
   { "id": "zh-HK", "name": "香港繁體", "fallback": "zh-TW" }
   ```

   - `id`: used in `?lang=`, in the file name `i18n/<id>.json`, and in `screenshots.json`.
   - `name`: shown in the language picker, written in that language.
   - `lang` (optional): the BCP 47 tag for `<html lang>`, which also selects the right CJK fonts. Defaults to `id`.
   - `fallback` (optional): the language shown for untranslated strings, before English.
   - `flag` (optional): the flag shown in the language picker, a file name in `resources/flags/svg/` without `.svg` (e.g. `jp`). To add one, copy it there from [flag-icons](https://github.com/lipis/flag-icons) first.

2. Create `src/i18n/<id>.json`. Copying an existing one is the easiest start.
3. Optionally, translate the documents of the Director's Cut: each text of `src/legal/*.json` (see [The Director's Cut](#the-directors-cut)).
4. Run `build.py` (or keep `make homepage-dev-server` running). It rejects unknown keys and malformed files, and lists the untranslated keys.

### The language picker

The picker is [@llselect/core](https://gitlab.com/kuanyui/llselect), loaded from jsDelivr in `index.html`.

- Its API may change before 0.1.0, so the URL names an exact version.
- `integrity` makes the browser refuse any file other than the one checked.
- Its classes are styled in `style.css` from the colors of the page (light and dark), instead of loading one of its themes.
- If it does not load, both pickers stay hidden: the language is still picked as above, and the cut is the one chosen last.

To upgrade it, change the version in the URL, and set `integrity` to `sha384-` followed by the hash of the new file:

```sh
curl -fsSL https://cdn.jsdelivr.net/npm/@llselect/core@VERSION/dist/index.umd.js | openssl dgst -sha384 -binary | openssl base64 -A
```

### The Director's Cut

The page has two cuts, and the cut picker next to the language picker (an @llselect/core too) switches between them:

- The Wholesome Cut, shown by default.
- The Director's Cut, with the author's swearing, dark political jokes and strong opinions. The first switch to it asks three times: a warning (`#cut-dialog` in `index.html`), the terms of the Director's Cut (`#terms-dialog`), then their additional articles (`#amendments-dialog`). In each document, the button that continues (“I have actually read, and fully agree to, all of the provisions above”) is enabled only once the visitor has scrolled to its end. The cut, and the agreement, are remembered in `localStorage` like the language: after agreeing once, switching asks nothing.

A string that differs between the cuts is an object with one version per cut, instead of a string. For example, in `i18n/ja.json` and `i18n/zh-TW.json` (in their languages):

```json
"footer.disclaimer": {
  "safe":   "<strong>Disclaimer:</strong> … FITNESS FOR A PARTICULAR PURPOSE.",
  "danger": "<strong>Disclaimer:</strong> … NONETHELESS, POLITICAL INCORRECTNESS IS ABSOLUTELY GUARANTEED. …"
},
```

- The two versions are always next to each other, so that they are easy to compare, and neither one is forgotten.
- English: the Wholesome Cut is written in `index.html`, like every English string. The Director's Cut is in `i18n/en.json`, whose strings are all such objects with only `"danger"`. `main.js` downloads it only for the Director's Cut, so crawlers, link previews and visitors without JavaScript only ever see the Wholesome Cut.
- Only the strings of `data-i18n` and `data-i18n-html` can have a version per cut, not attributes.
- An empty version hides its element. For example, `"safe": ""` for a paragraph or a footnote that only the Director's Cut has. In `index.html`, such an element is empty and `hidden` (for a footnote, its `<p>`), so that it stays hidden without JavaScript too.
- `main.js` numbers the footnotes of each `.footnotes` block (`*1`, `*2`, …), skipping the hidden ones, so the numbers written in the strings do not matter. Only the numbers in `index.html` must be right for the Wholesome Cut, for visitors without JavaScript.
- `build.py` rejects malformed versions, and warns about a string that has a version per cut in one language but not in another.

The terms and their additional articles are documents, not strings of the page: `src/legal/terms.json` and `src/legal/amendments.json`. `main.js` downloads one when its dialog opens, and writes it into the `.terms-content` of the dialog, in the language of the page. So the page does not grow with them.

- Each one has `chapters`. A chapter has a `title`, and its content, or `sections` that have a `title` and their content. The content is of one of two kinds:
  - `articles`, written like a law of Taiwan (the terms). Each article has a `title` and a `body`: its paragraphs, and lists of subparagraphs (`{ "items": [...] }`). `style.css` numbers and indents them as laws do in each language.
  - `clauses` (the additional articles). `main.js` numbers them from their position: `1-2`, or `4-1-3` in a section. A clause can have its own `clauses`, numbered like `1-3-1`. `numberSeparator` sets the character between the numbers in each language: `4.1.3` in English, as is usual there, and `4-1-3` in Taiwan Traditional and Japanese. So clauses can be moved or added without renumbering anything by hand. That is also why a clause refers to another one by position (“the preceding clause”), not by number, and writes its own number as `{number}`.
- A document also has a `date`, and optionally a `closing` after its chapters. It starts with the title of its dialog and its date, centered over a double rule, as a contract does.
- Every text is an object with one string per language, side by side, like the captions of `screenshots.json`: `{ "en": "...", "ja": "...", "zh-TW": "..." }`. So the structure is written once for every language, and the translations of a text are easy to compare. Each string is HTML, like the strings of `data-i18n-html`.
- Every text needs English. A text missing another language shows its fallback language.
- `build.py` rejects a malformed document, and lists the texts that miss a language.
- The dialog around them (its title, the buttons…) is translated like the rest of the page.

## Deployment

Both hosts run `homepage/build.py` on every push to `master` that touches `homepage/`, `resources/appicon/` or `resources/flags/`, and can also be run by hand.
The page only uses relative URLs, so it works under any domain or path. The one exception is `og:image`, because link previews need a full URL: both sites use the image on GitHub Pages.

### GitHub Pages

- Workflow: `.github/workflows/homepage.yaml`.
- One-time setup: Settings → Pages → Build and deployment → Source: GitHub Actions.
- URL: https://kuanyui.github.io/qkiss/

### GitLab Pages

- Job `homepage` in `.gitlab-ci.yml`.
- By default, GitLab gives a new project a unique, randomly named domain.
  For https://kuanyui.gitlab.io/qkiss/, clear Deploy → Pages → Use unique domain.

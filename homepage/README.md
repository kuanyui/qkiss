# QKiSS Homepage

The official website of QKiSS: a single static page, without frameworks or build tools.
Its one dependency, the language picker [@llselect/core](https://gitlab.com/kuanyui/llselect), is loaded from a CDN (see [The language picker](#the-language-picker)).
It is published on both GitHub Pages and GitLab Pages, by the same script.

```
homepage/
├── build.py        # Checks and assembles the site; also the dev server (Python standard library only)
└── src/            # Copied into the site as is
    ├── index.html  # The page. English strings are written here.
    ├── style.css
    ├── main.js     # Translations, the language picker and the screenshot gallery
    └── i18n/       # One JSON file per language other than English
```

At build time, `build.py` also copies:

- The app icons it uses from `resources/appicon/` (see `APP_ICONS` in `build.py`), so they are never duplicated.
- The flags of the languages from `resources/flags/svg/`, with the MIT license notice of flag-icons.
- The screenshot images, from the wiki (see below).

## Local preview

```sh
make homepage-dev-server
```

It builds the site, serves it on port 8000 on every interface, and prints the URLs to open (LAN addresses included, so the host of a VM can open it too).
A change to `homepage/src/`, `homepage/screenshots.json`, the app icons, the flags or the screenshot images rebuilds it: reload the page.
When a change breaks a check, the last good build stays served and the errors are printed.
Ctrl-C stops it.

To see the screenshots too, clone the wiki once, and pass its `screenshots/` folder:

```sh
git clone https://github.com/kuanyui/qkiss.wiki.git ~/proj/qkiss.wiki
make homepage-dev-server HOMEPAGE_SCREENSHOTS=~/proj/qkiss.wiki/screenshots
```

`make homepage-build` writes the site into `generated/homepage/` instead, exactly as CI publishes it.
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

- The order of the array is the order on the page.
- `image` and `caption` are both required. Each one is either a string for every language, or an object `{ "language id": "..." }` that includes `en`. A missing language falls back like any other string (see [Translations](#translations)).
- The caption is also the alt text of the image.
- Image file names may only contain letters, digits, `.`, `_` and `-`, with one of the extensions `png`, `jpg`, `jpeg`, `webp`, `avif`, `gif`. They must be regular files in the `screenshots/` folder of the wiki.
- The gallery shows the same files scaled down, so keep them reasonably small (for example about 1600 px wide, WebP or an optimized PNG).

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

The page picks the language from, in order:

1. `?lang=<id>` in the URL.
2. The visitor's previous choice in the language picker (stored in `localStorage`).
3. The browser's preferred languages: `ja-JP` matches `ja`.
4. Otherwise, English.

As in the app (`ConfigFile::engOnly()`), a browser for which `isEnglishOnly()` in `main.js` holds always gets English, without the language picker.

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
3. Run `build.py` (or keep `make homepage-dev-server` running). It rejects unknown keys and malformed files, and lists the untranslated keys.

### The language picker

The picker is [@llselect/core](https://gitlab.com/kuanyui/llselect), loaded from jsDelivr in `index.html`.

- Its API may change before 0.1.0, so the URL names an exact version.
- `integrity` makes the browser refuse any file other than the one checked.
- Its classes are styled in `style.css` from the colors of the page (light and dark), instead of loading one of its themes.
- If it does not load, the picker stays hidden; the language is still picked as above.

To upgrade it, change the version in the URL, and set `integrity` to `sha384-` followed by the hash of the new file:

```sh
curl -fsSL https://cdn.jsdelivr.net/npm/@llselect/core@VERSION/dist/index.umd.js | openssl dgst -sha384 -binary | openssl base64 -A
```

## Deployment

Both hosts run `homepage/build.py` on every push to `master` that touches `homepage/`, `resources/appicon/` or `resources/flags/`, and can also be run by hand.
The page only uses relative URLs, so it works under any domain or path.

### GitHub Pages

- Workflow: `.github/workflows/homepage.yaml`.
- One-time setup: Settings → Pages → Build and deployment → Source: GitHub Actions.
- URL: https://kuanyui.github.io/qkiss/

### GitLab Pages

- Job `homepage` in `.gitlab-ci.yml`.
- By default, GitLab gives a new project a unique, randomly named domain.
  For https://kuanyui.gitlab.io/qkiss/, clear Deploy → Pages → Use unique domain.

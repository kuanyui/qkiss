# QKiSS Homepage

The official website of QKiSS: a single static page, without frameworks, build tools or dependencies.
It is published on both GitHub Pages and GitLab Pages, by the same script.

```
homepage/
├── build.py        # Checks and assembles the site; also the dev server (Python standard library only)
└── src/            # Copied into the site as is
    ├── index.html  # The page. English strings are written here.
    ├── style.css
    ├── main.js     # Translations and the screenshot gallery
    └── i18n/       # One JSON file per language other than English
```

At build time, `build.py` also copies:

- The app icons it uses from `resources/appicon/` (see `APP_ICONS` in `build.py`), so they are never duplicated.
- The screenshots, from outside this repository (see below).

## Local preview

```sh
make homepage-dev-server
```

It builds the site, serves it on port 8000 on every interface, and prints the URLs to open (LAN addresses included, so the host of a VM can open it too).
A change to `homepage/src/`, the app icons or the screenshots rebuilds it: reload the page.
When a change breaks a check, the last good build stays served and the errors are printed.
Ctrl-C stops it.

To see the screenshots too, clone the screenshots repository once, and pass the clone:

```sh
git clone https://github.com/kuanyui/qkiss-screenshots ~/proj/qkiss-screenshots
make homepage-dev-server HOMEPAGE_SCREENSHOTS=~/proj/qkiss-screenshots
```

`make homepage-build` writes the site into `generated/homepage/` instead, exactly as CI publishes it.
`make help` lists the variables (`HOMEPAGE_OUT`, `HOMEPAGE_PORT`, `HOMEPAGE_SCREENSHOTS`).
Opening `src/index.html` directly (`file://`) does not work, because translations and screenshots are fetched.

## Screenshots

Screenshots are not stored in this repository, so they never bloat its history, however often they are replaced.
They live in a separate repository, `qkiss-screenshots`, in the same namespace on each host:

- https://github.com/kuanyui/qkiss-screenshots
- https://gitlab.com/kuanyui/qkiss-screenshots

At deploy time, CI clones it with `--depth 1`, and `build.py` copies `screenshots.json` and the images it lists into the site.
To clone another URL, set the CI variable `SCREENSHOTS_REPO`.

### `screenshots.json`

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
- Image file names may only contain letters, digits, `.`, `_` and `-`, with one of the extensions `png`, `jpg`, `jpeg`, `webp`, `avif`, `gif`. They must be regular files next to `screenshots.json`.
- The gallery shows the same files scaled down, so keep them reasonably small (for example about 1600 px wide, WebP or an optimized PNG).

`build.py` rejects anything else, and then nothing is deployed.

### Updating screenshots

1. Commit to `qkiss-screenshots`, and push it to both hosts.
2. Deploy again, because pushing to `qkiss-screenshots` does not trigger it:
   - GitHub: Actions → Homepage → Run workflow.
   - GitLab: Build → Pipelines → Run pipeline, on `master`.

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

2. Create `src/i18n/<id>.json`. Copying an existing one is the easiest start.
3. Run `build.py` (or keep `make homepage-dev-server` running). It rejects unknown keys and malformed files, and lists the untranslated keys.

## Deployment

Both hosts run `homepage/build.py` on every push to `master` that touches `homepage/` or `resources/appicon/`, and can also be run by hand.
The page only uses relative URLs, so it works under any domain or path.

### GitHub Pages

- Workflow: `.github/workflows/homepage.yaml`.
- One-time setup: Settings → Pages → Build and deployment → Source: GitHub Actions.
- URL: https://kuanyui.github.io/qkiss/

### GitLab Pages

- Job `homepage` in `.gitlab-ci.yml`.
- By default, GitLab gives a new project a unique, randomly named domain.
  For https://kuanyui.gitlab.io/qkiss/, clear Deploy → Pages → Use unique domain.

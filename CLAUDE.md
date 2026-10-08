# Raaj Draw desktop — project context

Fork of **Inkscape 1.4.4** (GPL-2.0-or-later) rebranded as **Raaj Draw** for Raaj Software (owner Taha Tahir Ali).
Website, accounts and billing live in the separate project `D:\raajdraw` (draw.raajsoftware.com). Keep them separate.

## Repository
- Branch `raaj-draw` on top of upstream tag `INKSCAPE_1_4_4` (shallow clone from gitlab.com/inkscape/inkscape).
  No GitHub remote yet: the owner must create a public repo (GPL requires the source to be public with binaries).
- Submodules stay upstream's (po, share/extensions, share/themes, src/3rdparty/*). Never commit changes inside them.

## What we changed (keep changes small and contained for upstream merges)
- `src/raaj/` — `account.{h,cpp}`: browser device sign-in, demo/plan check at start and every 10 min, dialogs
  (sign in, checking, plan ended, can't reach server, Help → Raaj Draw Account). `http.{h,cpp}`: WinHTTP on Windows,
  libcurl elsewhere (with CA-bundle lookup for the AppImage). Server replies in `?format=kv` key=value lines.
  Only the GUI is gated (end of `InkscapeApplication::on_startup`); CLI use (also by extensions) is not.
  State file: `<profile>/raaj-account.ini` (token, access_until, checked_at). Offline grace: 7 days, never past plan end.
  `RAAJDRAW_SITE` env var overrides https://draw.raajsoftware.com (for testing against `npm run dev` on :8790).
- `raaj/rebrand.py` — idempotent file edits (app ID `com.raajsoftware.RaajDraw`, settings/cache folder `raajdraw`,
  help links, "created with" notes, About/Welcome screens, desktop/AppStream, NSIS/MSI/AppImage packaging with Raaj Draw's
  own App Paths, ProgIDs and MSI upgrade code so an installed Inkscape is never touched) **and** translation branding:
  "Inkscape" → "Raaj Draw" in every po/*.po msgstr plus a generated po/en.po. Source strings stay upstream's.
  CI runs `python3 raaj/rebrand.py --translations` before CMake (po/ is a submodule, branded catalogs aren't committed).
  Technical/credit strings keep "Inkscape" (KEEP_PATTERNS). Re-run the full script after merging a new upstream release.
- `raaj/make-assets.py` — draws icons, .ico, start/welcome/About screens and NSIS bitmaps with Pillow (results committed).
- Programs: `raajdraw`, `raajdraw-view` (+ `.com` console variants on Windows) via OUTPUT_NAME; CMake targets keep
  upstream names. `INKSCAPE_COMMAND` is set to the raajdraw executable so extensions call the right program.
- `.github/workflows/build.yml` — Windows (Inkscape's prebuilt MSYS2 bundle r168 from gitlab project 46863172, NSIS from
  choco, `ninja dist-win-exe dist-win-7z`) and Linux AppImage (container registry.gitlab.com/inkscape/inkscape-ci-docker/ubuntu-2004,
  `packaging/appimage/generate.sh`). Tag `v*` → GitHub Release; the website's /download/* points at its assets.

## Rules
- GPL: keep copyright notices, COPYING/LICENSES, author credits; say "based on Inkscape"; don't call it Inkscape.
- Don't rename the `inkscape:`/`sodipodi:` SVG namespaces or icon file names (`org.inkscape.Inkscape*`).
- Nothing has been compiled locally (no toolchain on this PC); CI is the first compile. Expect fixes after the first run.

## Status (2026-10-08)
- 3 commits on `raaj-draw`; untested build. Waiting on: GitHub repo from the owner, first CI run, code-signing decision.
- Plan limits: computers per plan 1/2/3/5 (placeholders, in the website's shared/plans.js).

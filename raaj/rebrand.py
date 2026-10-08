#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Raaj Draw: apply the Raaj Draw branding on top of Inkscape.

Run from the repository root after merging a new Inkscape release:  python raaj/rebrand.py
The builds run "python3 raaj/rebrand.py --translations" before CMake, because po/ is Inkscape's
translations submodule and the branded catalogs are not committed. Works with Python 3.8+.
It is idempotent: edits that are already applied are skipped. An edit whose original text is
missing (because upstream changed it) stops the script, so nothing is silently left unbranded.

Two kinds of changes:
1. File edits: application ID, settings folder, links, installer and AppImage packaging.
2. Translations: the user-visible strings are NOT changed in the source (that would break every
   translation and make upstream merges painful). Instead "Inkscape" becomes "Raaj Draw" in the
   translated texts of every po/*.po, and po/en.po is generated so English shows "Raaj Draw" too.
   Technical and credit strings (SVG namespace, legacy files, licences, credits) keep "Inkscape".

Artwork is separate: python raaj/make-assets.py
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
NAME = "Raaj Draw"
SITE = "https://draw.raajsoftware.com"
CONTACT = "https://raajsoftware.com/contact"
APP_ID = "com.raajsoftware.RaajDraw"

problems = []


def edit(path, pairs):
    """Replace each (old, new) exactly once in path. Skips pairs that are already applied."""
    p = ROOT / path
    text = p.read_text(encoding="utf-8", errors="surrogateescape")
    changed = False
    for old, new, *count in pairs:
        expected = count[0] if count else 1
        n = text.count(old)
        if n == 0 and new in text:
            continue  # already applied
        if n != expected:
            problems.append(f"{path}: expected {expected}× {old[:70]!r}, found {n}")
            continue
        text = text.replace(old, new)
        changed = True
    if changed:
        with open(p, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
            f.write(text)
        print("edited", path)


def rename(old, new):
    o, n = ROOT / old, ROOT / new
    if o.exists() and not n.exists():
        o.rename(n)
        print("renamed", old, "->", new)
    elif not n.exists():
        problems.append(f"missing {old}")


# --------------------------------------------------------------------------- 1. file edits

def file_edits():
    # Application ID: must differ from Inkscape's, or opening Raaj Draw while Inkscape runs would
    # hand the request to Inkscape (GApplication single instance). Icon names stay Inkscape's.
    edit("src/inkscape-application.cpp", [
        ('auto app_id = Glib::ustring("org.inkscape.Inkscape");', f'auto app_id = Glib::ustring("{APP_ID}");'),
        ('Glib::ustring app_id = "org.inkscape.Inkscape." + id_tag;', f'Glib::ustring app_id = "{APP_ID}." + id_tag;'),
        ('Glib::ustring app_id = "org.inkscape.Inkscape.p" + std::to_string(getpid());',
         f'Glib::ustring app_id = "{APP_ID}.p" + std::to_string(getpid());'),
    ])

    # Settings and cache folders: %APPDATA%\raajdraw, ~/.config/raajdraw (not Inkscape's).
    edit("src/io/resource.cpp", [
        ('#define INKSCAPE_PROFILE_DIR "inkscape"', '#define INKSCAPE_PROFILE_DIR "raajdraw"'),
        ('return g_build_filename(g_get_user_cache_dir(), "inkscape", filename, extra, nullptr);',
         'return g_build_filename(g_get_user_cache_dir(), "raajdraw", filename, extra, nullptr);'),
    ])
    edit("src/auto-save.cpp", [
        ('autosave_dir = Glib::build_filename(Glib::get_user_cache_dir(), "inkscape");',
         'autosave_dir = Glib::build_filename(Glib::get_user_cache_dir(), "raajdraw");'),
    ])

    # Help menu links: support goes to Raaj Software; the manual, FAQ and references stay Inkscape's.
    edit("src/actions/actions-help-url.cpp", [
        ('Glib::ustring url = Glib::ustring::compose("https://inkscape.org/%1/community/", lang);',
         f'Glib::ustring url = "{CONTACT}";'),
        ('Glib::ustring url = Glib::ustring::compose("https://inkscape.org/%1/contribute/report-bugs/", lang);',
         f'Glib::ustring url = "{CONTACT}";'),
        ('Glib::ustring url = Glib::ustring::compose("https://inkscape.org/%1/release/inkscape-%2", lang, development_version ? "master" : version);',
         f'Glib::ustring url = "{SITE}/";'),
        ('Glib::ustring url = Glib::ustring::compose("https://inkscape.org/%1/donate#lang=%1&version=%2", lang, version);',
         f'Glib::ustring url = "{SITE}/account";'),
    ])

    # "Created with" notes written into files.
    edit("src/xml/repr.cpp", [
        ('createComment(" Created with Inkscape (http://www.inkscape.org/) ")',
         f'createComment(" Created with {NAME} ({SITE}/), based on Inkscape ")'),
    ])
    edit("src/helper/png-write.cpp", [
        ('textList.add("Software", "www.inkscape.org");', f'textList.add("Software", "{NAME} ({SITE})");'),
    ])
    edit("src/extension/internal/cairo-renderer.cpp", [
        ('Glib::ustring::compose("Inkscape %1 (https://inkscape.org)",', f'Glib::ustring::compose("{NAME} %1 ({SITE})",'),
    ])
    edit("src/inkscape.cpp", [
        ('please file a bug at https://inkscape.org/report', f'please report it at {CONTACT}'),
    ])

    # About dialog: version button and footer link.
    edit("src/ui/dialog/about.cpp", [
        ('#include "inkscape-version-info.h"\n', '#include "inkscape-version-info.h"\n#include "inkscape-version.h" // Raaj Draw\n'),
        ('    auto text = Inkscape::inkscape_version();',
         f'    auto text = std::string("{NAME} ") + Inkscape::version_string + " (based on Inkscape)";'),
    ])
    edit("share/ui/inkscape-about.glade", [
        ('<property name="label">www.inkscape.org</property>', '<property name="label">draw.raajsoftware.com</property>'),
        ('<property name="uri">https://inkscape.org/?about-screen=1</property>', f'<property name="uri">{SITE}/</property>'),
    ])

    # Welcome screen: the "Supported by You" page becomes the account/plan page.
    edit("share/ui/inkscape-welcome.glade", [
        ('<property name="uri">https://inkscape.org/splash/contribute/</property>', f'<property name="uri">{SITE}/#pricing</property>'),
        ('<property name="uri">https://inkscape.org/splash/support/</property>', f'<property name="uri">{SITE}/account</property>'),
    ])

    # Linux desktop entry and AppStream data.
    edit("org.inkscape.Inkscape.desktop.template", [
        ("Name=Inkscape\n", f"Name={NAME}\n"),
        ("X-GNOME-FullName=Inkscape Vector Graphics Editor", f"X-GNOME-FullName={NAME} Vector Graphics Editor"),
        ("Exec=inkscape %F", "Exec=raajdraw %F"),
        ("TryExec=inkscape", "TryExec=raajdraw"),
        ("Exec=inkscape\n", "Exec=raajdraw\n"),
    ])
    edit("org.inkscape.Inkscape.appdata.xml.in", [
        ("<developer_name>The Inkscape Community</developer_name>", "<developer_name>Raaj Software (based on Inkscape)</developer_name>"),
        ("<name>Inkscape</name>", f"<name>{NAME}</name>"),
        ('<url type="homepage">https://inkscape.org</url>', f'<url type="homepage">{SITE}/</url>'),
        ('<url type="bugtracker">https://inkscape.org/contribute/report-bugs</url>', f'<url type="bugtracker">{CONTACT}</url>'),
        ('<url type="donation">https://inkscape.org/support-us/donate</url>\n', ''),
    ])

    # Packaging: names, links, executables, and nothing that touches an installed Inkscape.
    edit("CMakeScripts/ConfigCPack.cmake", [
        ('set(CPACK_PACKAGE_NAME "Inkscape")', f'set(CPACK_PACKAGE_NAME "{NAME}")'),
        ('set(CPACK_PACKAGE_VENDOR "Inkscape")', 'set(CPACK_PACKAGE_VENDOR "Raaj Software")'),
        ('set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-source vector graphics editor")',
         'set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Raaj Draw vector graphics editor, based on Inkscape")'),
        ('set(CPACK_PACKAGE_HOMEPAGE_URL "https://inkscape.org")', f'set(CPACK_PACKAGE_HOMEPAGE_URL "{SITE}")'),
        ('set(CPACK_PACKAGE_CONTACT "Inkscape developers <inkscape-devel@lists.inkscape.org>")',
         'set(CPACK_PACKAGE_CONTACT "Raaj Software <taha@raajsoftware.com>")'),
        ('set(CPACK_PACKAGE_EXECUTABLES "inkscape;Inkscape;inkview;Inkview")',
         f'set(CPACK_PACKAGE_EXECUTABLES "raajdraw;{NAME}")'),
        ('set(CPACK_CREATE_DESKTOP_LINKS "inkscape")', 'set(CPACK_CREATE_DESKTOP_LINKS "raajdraw")'),
        ('    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inkscape")', f'    set(CPACK_PACKAGE_INSTALL_DIRECTORY "{NAME}")'),
        ('    set(CPACK_PACKAGE_INSTALL_DIRECTORY "inkscape")', '    set(CPACK_PACKAGE_INSTALL_DIRECTORY "raajdraw")'),
        ('set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/inkscape.exe")', 'set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/raajdraw.exe")'),
        ('set(CPACK_NSIS_MENU_LINKS "${CPACK_PACKAGE_HOMEPAGE_URL}" "Inkscape Homepage")',
         f'set(CPACK_NSIS_MENU_LINKS "${{CPACK_PACKAGE_HOMEPAGE_URL}}" "{NAME} website")'),
        ('set(CPACK_NSIS_MUI_FINISHPAGE_RUN "inkscape")', 'set(CPACK_NSIS_MUI_FINISHPAGE_RUN "raajdraw")'),
        # App Paths and file types: Raaj Draw's own names, so Inkscape's registration is left alone.
        ("App Paths\\\\\\\\inkscape.exe' '' '$INSTDIR\\\\\\\\bin\\\\\\\\inkscape.exe'",
         "App Paths\\\\\\\\raajdraw.exe' '' '$INSTDIR\\\\\\\\bin\\\\\\\\raajdraw.exe'"),
        ("App Paths\\\\\\\\inkscape.exe' 'Path'", "App Paths\\\\\\\\raajdraw.exe' 'Path'"),
        ("!insertmacro APP_ASSOCIATE 'svg' 'Inkscape.SVG' 'Scalable Vector Graphics' '$INSTDIR\\\\\\\\bin\\\\\\\\inkscape.exe,0' 'Open with Inkscape' '$INSTDIR\\\\\\\\bin\\\\\\\\inkscape.exe",
         f"!insertmacro APP_ASSOCIATE 'svg' 'RaajDraw.SVG' 'Scalable Vector Graphics' '$INSTDIR\\\\\\\\bin\\\\\\\\raajdraw.exe,0' 'Open with {NAME}' '$INSTDIR\\\\\\\\bin\\\\\\\\raajdraw.exe"),
        ("!insertmacro APP_ASSOCIATE 'svgz' 'Inkscape.SVGZ' 'Compressed Scalable Vector Graphics' '$INSTDIR\\\\\\\\bin\\\\\\\\inkscape.exe,0' 'Open with Inkscape' '$INSTDIR\\\\\\\\bin\\\\\\\\inkscape.exe",
         f"!insertmacro APP_ASSOCIATE 'svgz' 'RaajDraw.SVGZ' 'Compressed Scalable Vector Graphics' '$INSTDIR\\\\\\\\bin\\\\\\\\raajdraw.exe,0' 'Open with {NAME}' '$INSTDIR\\\\\\\\bin\\\\\\\\raajdraw.exe"),
        ("DeleteRegKey SHCTX 'SOFTWARE\\\\\\\\Microsoft\\\\\\\\Windows\\\\\\\\CurrentVersion\\\\\\\\App Paths\\\\\\\\inkscape.exe'",
         "DeleteRegKey SHCTX 'SOFTWARE\\\\\\\\Microsoft\\\\\\\\Windows\\\\\\\\CurrentVersion\\\\\\\\App Paths\\\\\\\\raajdraw.exe'"),
        ("!insertmacro APP_UNASSOCIATE 'svg' 'Inkscape.SVG'", "!insertmacro APP_UNASSOCIATE 'svg' 'RaajDraw.SVG'"),
        ("!insertmacro APP_UNASSOCIATE 'svgz' 'Inkscape.SVGZ'", "!insertmacro APP_UNASSOCIATE 'svgz' 'RaajDraw.SVGZ'"),
        # A new MSI upgrade code, so an MSI of Raaj Draw never treats Inkscape as an older version.
        ('set(CPACK_WIX_UPGRADE_GUID "4d5fedaa-84a0-48be-bd2a-08246398361a")',
         'set(CPACK_WIX_UPGRADE_GUID "7c1f2b8e-5a3d-4f6b-9e21-raajdraw0001")'.replace("raajdraw0001", "3b6d0c9a8f12")),
        ('DISPLAY_NAME "Inkscape SVG Editor"', f'DISPLAY_NAME "{NAME}"'),
        ('DESCRIPTION "Inkscape core files and dependencies"', f'DESCRIPTION "{NAME} core files and dependencies"'),
        ('DESCRIPTION "Python interpreter (required to run Inkscape extensions)"', 'DESCRIPTION "Python interpreter (required to run extensions)"'),
        ('DISPLAY_NAME "Inkscape Data"', f'DISPLAY_NAME "{NAME} data"'),
        ('DESCRIPTION "Inkscape extensions (including many import and export plugins)"', 'DESCRIPTION "Extensions (including many import and export plugins)"'),
        ('DESCRIPTION "Inkscape themes (look and feel including icons)"', 'DESCRIPTION "Themes (look and feel including icons)"'),
        ('DESCRIPTION "Example files created in Inkscape"', 'DESCRIPTION "Example drawings"'),
        ('DESCRIPTION "Tutorials teaching Inkscape usage"', 'DESCRIPTION "Tutorials"'),
    ])
    edit("CMakeScripts/Dist.cmake", [
        ('set(INKSCAPE_DIST_PREFIX "${PROJECT_NAME}-${INKSCAPE_VERSION}")', 'set(INKSCAPE_DIST_PREFIX "raajdraw-${INKSCAPE_VERSION}")'),
    ])
    edit("packaging/nsis/uninstall-old-versions.nsh", [
        ('; Check for previous Inkscape installation', '; Check for a previous Raaj Draw installation'),
        ('IfFileExists "$INSTDIR\\bin\\inkscape.exe" previousInstallFound noPreviousInstallFound',
         'IfFileExists "$INSTDIR\\bin\\raajdraw.exe" previousInstallFound noPreviousInstallFound'),
        ('nsExec::Exec \'"wmic" product where Name="Inkscape" uninstall\'', f'nsExec::Exec \'"wmic" product where Name="{NAME}" uninstall\''),
        ('; Now check again if Inkscape is already installed', '; Now check again if Raaj Draw is already installed'),
        ('IfFileExists "$INSTDIR\\bin\\inkscape.exe" previousInstallStillFound previousInstallSuccessfullyRemoved',
         'IfFileExists "$INSTDIR\\bin\\raajdraw.exe" previousInstallStillFound previousInstallSuccessfullyRemoved'),
        ('"A previous installation of Inkscape was found.', f'"A previous installation of {NAME} was found.'),
    ])

    # Windows convenience launchers in the install folder.
    rename("packaging/win32/Run Inkscape !.bat", "packaging/win32/Run Raaj Draw !.bat")
    rename("packaging/win32/Run Inkscape with GTK Inspector.bat", "packaging/win32/Run Raaj Draw with GTK Inspector.bat")
    rename("packaging/win32/Run Inkscape and create debug trace.bat", "packaging/win32/Run Raaj Draw and create debug trace.bat")
    edit("CMakeScripts/InstallMSYS2.cmake", [
        ('"packaging/win32/Run Inkscape and create debug trace.bat"', '"packaging/win32/Run Raaj Draw and create debug trace.bat"'),
        ('"packaging/win32/Run Inkscape !.bat"', '"packaging/win32/Run Raaj Draw !.bat"'),
        ('"packaging/win32/Run Inkscape with GTK Inspector.bat"', '"packaging/win32/Run Raaj Draw with GTK Inspector.bat"'),
    ])
    edit("packaging/win32/Run Raaj Draw !.bat", [("start inkscape.exe", "start raajdraw.exe")])
    edit("packaging/win32/Run Raaj Draw with GTK Inspector.bat", [("start inkscape.exe", "start raajdraw.exe")])
    edit("packaging/win32/gdb_create_backtrace.bat", [
        ("rem Execute this to create a debug backtrace of an Inkscape crash.", f"rem Execute this to create a debug backtrace of a {NAME} crash."),
        ('set TRACEFILE="%USERPROFILE%\\inkscape_backtrace.txt"', 'set TRACEFILE="%USERPROFILE%\\raajdraw_backtrace.txt"'),
        ("echo After Inkscape starts, try to force the crash.", f"echo After {NAME} starts, try to force the crash."),
        ("echo --- INKSCAPE VERSION --- > %TRACEFILE%", "echo --- RAAJ DRAW VERSION --- > %TRACEFILE%"),
        ("inkscape.com --debug-info >> %TRACEFILE%", "raajdraw.com --debug-info >> %TRACEFILE%"),
        ("echo Launching Inkscape, please wait...", f"echo Launching {NAME}, please wait..."),
        ('-ex "bt" inkscape.exe >> %TRACEFILE%', '-ex "bt" raajdraw.exe >> %TRACEFILE%'),
        ("reporting the issue at https://inkscape.org/report", f"reporting the issue at {CONTACT}"),
    ])

    # AppImage
    edit("packaging/appimage/AppRun", [
        ("# Custom AppRun script for Inkscape", f"# Custom AppRun script for {NAME} (based on Inkscape's)"),
        ('    MAIN="$HERE/usr/bin/inkscape"', '    MAIN="$HERE/usr/bin/raajdraw"'),
    ])
    edit("packaging/appimage/generate.sh", [
        ("-appimage -unsupported-bundle-everything -executable=appdir/usr/bin/inkview \\",
         "-appimage -unsupported-bundle-everything -executable=appdir/usr/bin/raajdraw-view \\"),
        ("mv Inkscape*.AppImage* ../", "mv Raaj_Draw*.AppImage* ../"),
    ])


# --------------------------------------------------------------------------- 2. translations

WORD = re.compile(r"\bInkscape\b")

# These keep "Inkscape": they name Inkscape's SVG extensions, its history, its manual and credits.
KEEP_PATTERNS = [
    r"Namespace", r"Inkscape-specific", r"[Ll]egacy Inkscape", r"older version of Inkscape",
    r"updated Inkscape to follow", r"involved with Inkscape", r"Inkscape licen[cs]e", r"Inkscape Manual",
    r"Classic Inkscape", r"inkscape\.org", r"Inkscape [Dd]evelopers", r"Inkscape [Cc]ommunity",
    r"Inkscape [Pp]roject", r"translation activities",
]
KEEP = re.compile("|".join(KEEP_PATTERNS))

# Replaced with this English text in every language.
OVERRIDES = {
    "Inkscape. Draw freely.": f"{NAME} — based on Inkscape",
    "© %1 Inkscape Developers": "© %1 Raaj Software · based on Inkscape, © Inkscape Developers",
    "Supported by You": "Your account",
    "Thanks!": "Continue",
    "Learn how to\nContribute Time": "See the\nplans",
    "Learn how to\nFund Inkscape": "Manage your\naccount",
}
OVERRIDE_PREFIX = {
    "<b>The Inkscape project is supported by users like you.</b>":
        f"<b>{NAME} is made by Raaj Software.</b> Your free 15-minute demo starts when you first sign in. "
        f"After that, choose a plan at draw.raajsoftware.com — plans start at ₹300 a month, and yearly plans "
        f"get two months free.\n<b>{NAME} is based on Inkscape, free software made by the Inkscape community.</b>",
}


def po_unescape(s):
    return re.sub(r'\\(["\\nt])', lambda m: {"n": "\n", "t": "\t"}.get(m.group(1), m.group(1)), s)


def po_escape(s):
    return s.replace("\\", "\\\\").replace('"', '\\"').replace("\t", "\\t").replace("\n", "\\n")


def po_quote_block(keyword, value):
    if "\n" not in value[:-1] and len(value) < 70:
        return [f'{keyword} "{po_escape(value)}"']
    lines = [f'{keyword} ""']
    parts = value.split("\n")
    for i, part in enumerate(parts):
        piece = part + ("\n" if i < len(parts) - 1 else "")
        if piece:
            lines.append(f'"{po_escape(piece)}"')
    return lines


def parse_po(lines):
    """Yield entries: dict with flags_idx, msgctxt, msgid, msgid_plural, and msgstr blocks {key: (start, end, value)}."""
    entries, cur, field = [], None, None

    def start_entry(i):
        return {"start": i, "flags_idx": None, "msgctxt": None, "msgid": None, "msgid_plural": None, "msgstr": {}}

    for i, raw in enumerate(lines):
        line = raw.rstrip("\n")
        if line.startswith("#~"):
            continue
        if not line.strip():
            if cur:
                entries.append(cur)
            cur, field = None, None
            continue
        if cur is None:
            cur = start_entry(i)
        if line.startswith("#,"):
            cur["flags_idx"] = i
            continue
        if line.startswith("#"):
            continue
        m = re.match(r'^(msgctxt|msgid_plural|msgid|msgstr(?:\[\d+\])?)\s+"(.*)"\s*$', line)
        if m:
            key, val = m.group(1), po_unescape(m.group(2))
            if key.startswith("msgstr"):
                cur["msgstr"][key] = [i, i + 1, val]
                field = ("msgstr", key)
            else:
                cur[key] = val
                field = ("plain", key)
            continue
        m = re.match(r'^"(.*)"\s*$', line)
        if m and field:
            val = po_unescape(m.group(1))
            if field[0] == "msgstr":
                block = cur["msgstr"][field[1]]
                block[1] = i + 1
                block[2] += val
            else:
                cur[field[1]] += val
    if cur:
        entries.append(cur)
    return entries


def branded(msgid):
    """The text to show for msgid, or None when it keeps Inkscape's wording."""
    if msgid in OVERRIDES:
        return OVERRIDES[msgid]
    for prefix, text in OVERRIDE_PREFIX.items():
        if msgid.startswith(prefix):
            return text
    if not WORD.search(msgid) or KEEP.search(msgid):
        return None
    return WORD.sub(NAME, msgid)


def brand_po(path):
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    edits = []  # (start, end, new_lines)
    for e in parse_po(lines):
        msgid = e["msgid"]
        if not msgid or not e["msgstr"]:
            continue
        target = branded(msgid)
        if target is None:
            continue
        fuzzy = e["flags_idx"] is not None and "fuzzy" in lines[e["flags_idx"]]
        forced = msgid in OVERRIDES or any(msgid.startswith(p) for p in OVERRIDE_PREFIX)
        for key, (s, t, val) in e["msgstr"].items():
            if forced or not val or fuzzy:
                new = target if key in ("msgstr", "msgstr[0]") or not e["msgid_plural"] else WORD.sub(NAME, e["msgid_plural"])
            else:
                new = WORD.sub(NAME, val)
            if new != val:
                edits.append((s, t, [l + "\n" for l in po_quote_block(key, new)]))
        if fuzzy:
            flags = lines[e["flags_idx"]]
            rest = [f.strip() for f in flags[2:].split(",") if f.strip() and f.strip() != "fuzzy"]
            edits.append((e["flags_idx"], e["flags_idx"] + 1, [("#, " + ", ".join(rest) + "\n")] if rest else []))
    if not edits:
        return 0
    for s, t, new in sorted(edits, key=lambda x: x[0], reverse=True):
        lines[s:t] = new
    with open(path, "w", encoding="utf-8", newline="") as f:
        f.write("".join(lines))
    return len(edits)


def write_en_po():
    """po/en.po: English texts that differ from the source (so English shows Raaj Draw)."""
    pot = (ROOT / "po/inkscape.pot").read_text(encoding="utf-8").splitlines(keepends=True)
    out = [
        "# Raaj Draw: English branding catalog, generated by raaj/rebrand.py from po/inkscape.pot.\n",
        "# SPDX-License-Identifier: GPL-2.0-or-later\n",
        'msgid ""\n', 'msgstr ""\n',
        '"Project-Id-Version: raajdraw\\n"\n', '"Language: en\\n"\n',
        '"MIME-Version: 1.0\\n"\n', '"Content-Type: text/plain; charset=UTF-8\\n"\n',
        '"Content-Transfer-Encoding: 8bit\\n"\n', '"Plural-Forms: nplurals=2; plural=(n != 1);\\n"\n', "\n",
    ]
    count = 0
    for e in parse_po(pot):
        msgid = e["msgid"]
        if not msgid:
            continue
        target = branded(msgid)
        if target is None:
            continue
        if e["msgctxt"] is not None:
            out += [l + "\n" for l in po_quote_block("msgctxt", e["msgctxt"])]
        out += [l + "\n" for l in po_quote_block("msgid", msgid)]
        if e["msgid_plural"] is not None:
            out += [l + "\n" for l in po_quote_block("msgid_plural", e["msgid_plural"])]
            out += [l + "\n" for l in po_quote_block("msgstr[0]", target)]
            out += [l + "\n" for l in po_quote_block("msgstr[1]", WORD.sub(NAME, e["msgid_plural"]))]
        else:
            out += [l + "\n" for l in po_quote_block("msgstr", target)]
        out.append("\n")
        count += 1
    with open(ROOT / "po/en.po", "w", encoding="utf-8", newline="\n") as f:
        f.write("".join(out))
    print(f"wrote po/en.po ({count} strings)")


def translations():
    total = 0
    for po in sorted((ROOT / "po").glob("*.po")):
        if po.name == "en.po":
            continue
        n = brand_po(po)
        total += n
    print(f"branded translations: {total} changes")
    write_en_po()


if __name__ == "__main__":
    if "--translations" not in sys.argv:
        file_edits()
    translations()
    if problems:
        print("\nSTOPPED — these edits did not match (upstream changed?):", file=sys.stderr)
        for p in problems:
            print("  " + p, file=sys.stderr)
        sys.exit(1)
    print("done")

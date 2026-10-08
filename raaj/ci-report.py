#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Raaj Draw CI: after a failed build, publish the useful part of the log.

GitHub only shows job logs to signed-in users, but annotations and the job summary are public.
Usage: python3 raaj/ci-report.py build.log
Prints error lines (with a little context) and the end of the log as one ::error:: annotation and
appends them to $GITHUB_STEP_SUMMARY.
"""

import os
import re
import sys

PATTERN = re.compile(r"(error:|Error \d|\*\*\* |FAILED:|fatal|undefined reference|No such file|not found|CMake Error|ninja: build stopped)", re.I)


def main(path):
    try:
        lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
    except OSError as e:
        print(f"::error title=Build log::could not read {path}: {e}")
        return
    picked = []
    for i, line in enumerate(lines):
        if PATTERN.search(line):
            picked.extend(range(max(0, i - 3), min(len(lines), i + 4)))
    seen = set()
    errors = [lines[i] for i in picked if not (i in seen or seen.add(i))][:120]
    tail = lines[-60:]
    text = "== error lines ==\n" + "\n".join(errors) + "\n\n== last lines ==\n" + "\n".join(tail)
    text = text[-60000:]
    escaped = text.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
    print(f"::error title=Build log::{escaped}")
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as f:
            f.write("### Build log (errors and last lines)\n\n```\n" + text.replace("```", "'''") + "\n```\n")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "build.log")

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
# UI Policy consistancy:
#
#  * Do all the toolbars have the right can-focus set?
#
# Author: Martin Owens <doctormo@geek-2.com>
# Licensed under GPL version 2 or any later version, read the file "COPYING" for more information.

import fnmatch
import os
import sys

from glob import glob
from copy import deepcopy
from collections import defaultdict

from lxml import etree

UI_PATH = os.path.join(os.path.dirname(__file__), '..', 'share', 'ui')

IGNORE_TOOLBARS = ['toolbar-tool-prefs.ui']

ERRORS = {
    'parse': ("Parser Error", "Found something unusual in the XML"),
    'button-focus1': ("Button Takes Focus", "A toolbar button can have focus and will take that focus when clicked. Add focus-on-click=False to fix this."),
    'button-focus2': ("Button Refuses Focus", "A toolbar button is refusing focus, which makes it inaccessable to keyboard navigation. Remove can-focus=False"),
    'entry-focus': ("Entry Refuses Focus", "A toolbar entry doesn't allow itself to be in focus, stopping text from being entered. Change can-focus to True and focus-on-click to True (or remove them)"),
}

class Glade:
    """Open and parse a glade/ui file"""
    def __init__(self, filename):
        self.fn = os.path.basename(filename)
        self.objects = []
        self.chain = []
        self.parse(etree.parse(filename))

    def parse(self, doc):
        """Parse the document into checkable concerns"""
        self.parse_child(doc.getroot())

    def parse_child(self, elem):
        template = {'class': None, 'properties': {}, 'id': "NOID"}
        if elem.tag == "object":
            name = elem.attrib['class']
            self.chain.append(deepcopy(template))
            self.chain[-1]['class'] = elem.attrib['class']
            self.chain[-1]['id'] = elem.attrib.get('id', None)
            self.objects.append(self.chain[-1])
        elif elem.tag == "property" and self.chain and self.chain[-1]:
            name = elem.attrib['name']
            if name in self.chain[-1]['properties']:
                self.chain[-1]['error'] = f"Duplicate property '{name}'"
            self.chain[-1]['properties'][name] = elem.text
            self.chain.append(None)
        else:
            self.chain.append(None)

        for child in elem.getchildren():
            self.parse_child(child)

        self.chain.pop()


if __name__ == '__main__':
    sys.stderr.write("\n\n==== CHECKING TOOLBAR FILES ====\n\n")

    errors = defaultdict(list)

    for file in glob(os.path.join(UI_PATH, "toolbar-*.ui")):
        ui = Glade(file)
        if ui.fn in IGNORE_TOOLBARS:
            continue
        for obj in ui.objects:
            props = obj["properties"]
            _id = f"{ui.fn}: {obj['class']}:{obj['id']}"
            foc = props.get('focus-on-click', "True")
            cf = props.get('can-focus', "True")
            if 'error' in obj:
                errors['parse'].append((_id, obj['error']))
            # Policy 1. All Buttons should have can-focus: False
            if obj['class'] in ("GtkButton", "GtkMenuButton", "GtkToggleButton", "GtkRadioButton"):
                if foc == "True":
                    errors['button-focus1'].append((_id, f"can-focus={cf}"))
                elif cf == "False":
                    errors['button-focus2'].append((_id, f"focus-on-click={foc}"))
                
            # Policy 2. All Entries, SpinButtons should have can-focus: True
            if obj['class'] in ("GtkEntry", "GtkSpinButton", "GtkComboBoxText"):
                if foc == "False" or cf == "False":
                    errors['entry-focus'].append((_id, f"can-focus={cf} focus-on-click={foc}"))

    for code, instances in errors.items():
        name, desc = ERRORS[code]
        sys.stderr.write(f"\n == {name} ==\n\n  {desc}\n\n")
        for _id, props in instances:
            sys.stderr.write(f" * {_id}: {props}\n")

    if errors:
        sys.exit(-5)
    else:
        sys.stderr.write("COMPLETE, NO PROBLEMS FOUND\n")

# vi:sw=4:expandtab:

"""scripts/kicad-pin-types.py (plan-68-U): the golden reader and the comparison of
our ERC pin types with the recorded KiCad symbols in fixtures/kicad-pin-types/."""

import contextlib
import io
import os
import sys
import tempfile
import unittest
import warnings
from unittest import mock

import support

kpt = support.load('kicad-pin-types.py', 'kicad_pin_types')

HERE = support.fixture('kicad-pin-types')
SHARED = os.path.join(HERE, 'SharedSupport')
GOLDEN = os.path.join(HERE, 'erc_pins.txt')

ROW = '%-16s %-16s %8s'
EXPECTED = '\n'.join([
    'parts matched to a KiCad symbol: 8; compared (same pin count): 7; skipped (another package): 1; pins compared: 27',
    '',
    ROW % ('ours', 'KiCad', 'pins'),
    ROW % ('power_in', 'power_in', 12),
    ROW % ('input', 'input', 7),
    ROW % ('tri_state', 'output', 5) + '   *',
    ROW % ('output', 'output', 1),
    ROW % ('no_connect', 'no_connect', 1),
    ROW % ('passive', 'passive', 1),
    '',
    'power_in (ours) vs power_in (KiCad): 12 pins; by kind: gate 12',
    '    gate TST1000A pin 3 GND',
    '    gate TST1000B pin 3 GND',
    '    gate TST1000C pin 3 GND',
    '    gate TST1000D pin 3 GND',
    '    gate TST1000E pin 3 GND',
    '',
    'free (ours) vs free (KiCad): 0 pins; by kind: ',
    '',
    'tri_state vs output disagreements, by kind (parts, first three):',
    '  gate           5  TST1000A, TST1000B, TST1000C',
    ''])


def run_main(*argv):
    out, err = io.StringIO(), io.StringIO()
    with mock.patch.object(kpt.kc, 'PARTS_DIR', os.path.join(HERE, 'Parts')), \
            mock.patch.object(sys, 'argv', ['kicad-pin-types.py', *argv]), \
            contextlib.redirect_stdout(out), contextlib.redirect_stderr(err), warnings.catch_warnings():
        # load_ours leaves its part files to the garbage collector; that warning is not the script's output.
        warnings.simplefilter('ignore', ResourceWarning)
        kpt.main()
    return out.getvalue(), err.getvalue()


class GoldenTests(unittest.TestCase):
    def test_read_golden(self):
        golden = kpt.read_golden(GOLDEN)
        self.assertEqual(len(golden), 9)
        self.assertEqual(golden['TST1000A'], ('gate', 'itPP'))
        self.assertEqual(golden['TST4000A'], ('gate', 'inpb'))

    def test_letters_name_every_erc_pin_type(self):
        self.assertEqual(len(set(kpt.OURS.values())), 12)
        self.assertEqual(kpt.KICAD, {'unconnected': 'no_connect'})
        self.assertEqual(kpt.kc.find_symbols.__module__, 'kicad_compare')


class MainTests(unittest.TestCase):
    def test_counts_pairs_examples_and_disagreements(self):
        out, err = run_main('--kicad', SHARED, '--golden', GOLDEN, '--pair', 'power_in:power_in', '--pair', 'free:free')
        self.assertEqual(err, 'reading KiCad symbols…\n')
        self.assertEqual(out, EXPECTED)

    def test_no_disagreement_says_none(self):
        out, _ = run_main('--kicad', SHARED, '--golden', os.path.join(HERE, 'erc_pins_agree.txt'))
        self.assertEqual(out, '\n'.join([
            'parts matched to a KiCad symbol: 1; compared (same pin count): 1; skipped (another package): 0; '
            'pins compared: 4', '', ROW % ('ours', 'KiCad', 'pins'), ROW % ('power_in', 'power_in', 2),
            ROW % ('input', 'input', 1), ROW % ('output', 'output', 1), '',
            'tri_state vs output disagreements, by kind (parts, first three):', '  none', '']))


class CommandLineTests(unittest.TestCase):
    def test_over_the_real_parts(self):
        # None of the library's parts is named TST…: nothing matches the recorded symbols.
        r = support.run('kicad-pin-types.py', '--kicad', SHARED, '--golden', GOLDEN)
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertEqual(r.stderr, 'reading KiCad symbols…\n')
        self.assertEqual(r.stdout, 'parts matched to a KiCad symbol: 0; compared (same pin count): 0; skipped '
                                   '(another package): 0; pins compared: 0\n\n' + ROW % ('ours', 'KiCad', 'pins') +
                         '\n\ntri_state vs output disagreements, by kind (parts, first three):\n  none\n')

    def test_a_missing_golden_file_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            r = support.run('kicad-pin-types.py', '--kicad', SHARED, '--golden', os.path.join(tmp, 'none.txt'))
        self.assertEqual(r.returncode, 1)
        self.assertIn('FileNotFoundError', r.stderr)
        self.assertEqual(r.stdout, '')


if __name__ == '__main__':
    unittest.main()

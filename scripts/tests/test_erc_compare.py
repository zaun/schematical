"""scripts/erc-compare.py (plan-68-U): the log reader, both summaries, the KiCad run
through the fake kicad-cli, and the command line."""

import base64
import json
import os
import re
import tempfile
import unittest

import support

ec = support.load('erc-compare.py', 'erc_compare')
FAKE_CLI = os.path.join(support.FAKES, 'kicad-cli')


def b64(data):
    return base64.b64encode(data).decode()


def result(*violations):
    return b64(json.dumps({'violations': [{'rule': r, 'reference': ref} for r, ref in violations]}).encode())


AFU_RESULT = result(('pin_to_pin', 'U1.3, U2.3'), ('pin_not_connected', 'U1.5'), ('wire_dangling', ''),
                    ('duplicate_reference', 'U4'))
D2_RESULT = result(('pin_not_driven', 'U3.1'), ('no_connect_connected', 'U3.8'))
AFU_PRO = json.dumps({'meta': {'version': 1}, 'erc': {'rule_severities': {'pin_to_pin': 'error'}}}).encode()


def write_log(folder, lines):
    path = os.path.join(folder, 'xcodebuild.log')
    with open(path, 'w') as fh:
        fh.write('\n'.join(lines) + '\n')
    return path


def afu_lines():
    return ['Test Case started',
            'KICAD_FILE erc/AFU/AFU.kicad_sch ' + b64(b'(kicad_sch AFU)'),
            'KICAD_FILE erc/AFU/AFU.kicad_pro ' + b64(AFU_PRO),
            'KICAD_FILE board/AFU/AFU.kicad_pcb ' + b64(b'not ERC'),
            'ERC_RESULT AFU ' + AFU_RESULT]


def d2_lines():
    return ['KICAD_FILE erc/D2/D2.kicad_sch ' + b64(b'(kicad_sch D2)'), 'ERC_RESULT D2 ' + D2_RESULT]


class ReadLogTests(unittest.TestCase):
    def test_reads_exports_and_results_and_skips_other_lines(self):
        with tempfile.TemporaryDirectory() as tmp:
            exports, results = ec.read_log(write_log(tmp, afu_lines() + ['ERC_RESULT short']))
        self.assertEqual(dict(exports), {'AFU': {'AFU.kicad_sch': b'(kicad_sch AFU)', 'AFU.kicad_pro': AFU_PRO}})
        self.assertEqual(list(results), ['AFU'])
        self.assertEqual(results['AFU']['violations'][0], {'rule': 'pin_to_pin', 'reference': 'U1.3, U2.3'})


class SummaryTests(unittest.TestCase):
    def test_kicad_summary_keeps_rules_drops_edge_pins_and_unknown_rules(self):
        with open(support.fixture('erc-compare', 'AFU.erc.json')) as fh:
            found = ec.kicad_summary(json.load(fh))
        self.assertEqual(dict(found), {'pin_to_pin': [('U1.3', 'U2.3')], 'pin_not_connected': [('U1.5',)],
                                       'wire_dangling': [()], 'multiple_net_names': [(), ()]})

    def test_ours_summary_sorts_pins_of_pin_rules_only(self):
        found = ec.ours_summary(json.loads(base64.b64decode(AFU_RESULT)))
        self.assertEqual(dict(found), {'pin_to_pin': [('U1.3', 'U2.3')], 'pin_not_connected': [('U1.5',)],
                                       'wire_dangling': [()], 'duplicate_reference': [()]})


class RunKicadTests(unittest.TestCase):
    def test_writes_the_export_switches_rules_on_and_reads_the_report(self):
        with tempfile.TemporaryDirectory() as tmp:
            report = ec.run_kicad(FAKE_CLI, tmp, 'AFU', {'AFU.kicad_sch': b'x', 'AFU.kicad_pro': AFU_PRO})
            with open(os.path.join(tmp, 'AFU.kicad_pro')) as fh:
                pro = json.load(fh)
            with open(os.path.join(tmp, 'AFU.kicad_sch'), 'rb') as fh:
                sch = fh.read()
        self.assertEqual(report['sheets'][0]['violations'][0]['type'], 'pin_to_pin')
        self.assertEqual(sch, b'x')
        self.assertEqual(pro['meta'], {'version': 1})
        self.assertEqual(pro['erc']['rule_severities'], {'pin_to_pin': 'error', 'single_global_label': 'warning',
                                                        'four_way_junction': 'warning'})

    def test_writes_a_project_file_when_the_export_has_none(self):
        with tempfile.TemporaryDirectory() as tmp:
            report = ec.run_kicad(FAKE_CLI, tmp, 'D2', {'D2.kicad_sch': b'y'})
            with open(os.path.join(tmp, 'D2.kicad_pro')) as fh:
                pro = json.load(fh)
        self.assertEqual(report['source'], 'D2.kicad_sch')
        self.assertEqual(pro, {'erc': {'rule_severities': {'single_global_label': 'warning',
                                                          'four_way_junction': 'warning'}}})


class CommandLineTests(unittest.TestCase):
    def run_with(self, lines, *flags):
        with tempfile.TemporaryDirectory() as tmp:
            return support.run('erc-compare.py', write_log(tmp, lines), '--kicad-cli', FAKE_CLI, *flags)

    def test_a_difference_is_printed_and_exits_1(self):
        r = self.run_with(afu_lines() + d2_lines())
        self.assertEqual(r.returncode, 1, r.stderr)
        out = r.stdout
        self.assertRegex(out, r'(?m)^design +rule +ours +KiCad$')
        self.assertIn('\nAFU' + ' ' * 20 + 'pin_to_pin' + ' ' * 17 + '     1      1\n', out)
        self.assertRegex(out, r'(?m)^AFU +multiple_net_names +0 +2   \*$')
        self.assertRegex(out, r'(?m)^D2 +pin_not_driven +1 +1   \*$')
        self.assertRegex(out, r'(?m)^D2 +no_connect_connected +1 +1$')
        self.assertNotRegex(out, r'(?m)^AFU +four_way_junction')
        self.assertIn('\ndesigns: 2; rows compared: 32\n', out)
        self.assertIn('deliberate: AFU / duplicate_reference: ours 1, KiCad 0: kicad-cli 10.0.8 does not report '
                      'duplicate references (plan-63-data/README.md)\n', out)
        self.assertIn('deliberate: AFU / multiple_net_names: ours 0, KiCad 2: the export adds labels', out)
        self.assertIn("DIFFERENCE D2 / pin_not_driven:\n    ours : [('U3.1',)]\n    KiCad: [('U3.2',)]\n", out)
        self.assertTrue(out.endswith('\n1 differences\n'))

    def test_only_deliberate_differences_exit_0_and_verbose_prints_every_row(self):
        r = self.run_with(afu_lines(), '--verbose')
        self.assertEqual(r.returncode, 0, r.stderr)
        self.assertRegex(r.stdout, r'(?m)^AFU +four_way_junction +0 +0$')
        self.assertEqual(len(re.findall(r'(?m)^AFU ', r.stdout)), 16)
        self.assertTrue(r.stdout.endswith('\n0 differences\n'))

    def test_a_log_without_results_exits_with_a_message(self):
        r = self.run_with(['nothing here'])
        self.assertEqual(r.returncode, 1)
        self.assertEqual(r.stdout, '')
        self.assertRegex(r.stderr, r'^no ERC_RESULT lines in .*xcodebuild\.log '
                                   r'\(run the test with TEST_RUNNER_ERC_OUT=1\)\n$')


if __name__ == '__main__':
    unittest.main()

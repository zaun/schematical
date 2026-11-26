"""scripts/kicad-compare.py (plan-68-U): KiCad's S-expressions, our footprints,
symbol matching, the pin and footprint comparisons, the report, and the command
line, on the recorded KiCad library in fixtures/kicad/."""

import contextlib
import io
import os
import sys
import tempfile
import unittest
from unittest import mock

import support

kc = support.load('kicad-compare.py', 'kicad_compare')

KICAD = support.fixture('kicad')
SHARED = os.path.join(KICAD, 'Contents', 'SharedSupport')
PARTS = os.path.join(KICAD, 'Parts')
DIP8 = 'Package_DIP:DIP-8_W7.62mm'
R0603 = 'Resistor_SMD:R_0603_1608Metric'


def pin(number, name, role='input', **more):
    return dict(number=number, name=name, role=role, **more)


class SexpAndLibraryTests(unittest.TestCase):
    def test_sexp_nests_lists_and_unescapes_strings(self):
        self.assertEqual(kc.sexp('(a "b \\"c\\" \\\\d" (e 1.5) ())'), [['a', 'b "c" \\d', ['e', '1.5'], []]])

    def test_symbol_info_reads_extends_properties_and_pins_of_every_unit(self):
        tree = kc.sexp('(symbol "X" (extends "P") (property "Value" "X") (property "Other" "o") "loose" () '
                       '(pin input line (at 0 0) (name "A") (number "1")) '
                       '(symbol "X_1" (pin output line (number "2") (name "Q") ())))')[0]
        self.assertEqual(kc.symbol_info(tree), ('X', {'ext': 'P', 'props': {'Value': 'X'},
                                                       'pins': [['1', 'A', 'input'], ['2', 'Q', 'output']]}))

    def test_load_symbols_resolves_derived_symbols_and_repeated_pins(self):
        symbols = kc.load_symbols(SHARED)
        self.assertEqual(sorted(symbols), ['74xx:74TST00', '74xx:74TST01'])
        base, derived = symbols['74xx:74TST00'], symbols['74xx:74TST01']
        self.assertEqual(base['props'], {'Value': '74TST00', 'Footprint': DIP8, 'Datasheet': '',
                                         'ki_fp_filters': 'DIP*W7.62mm*'})
        self.assertEqual(base['pins']['8'], ('VCC', 'power_in'))
        self.assertEqual(base['pins']['3'], ('Y', 'output'))
        self.assertEqual(len(base['pins']), 8)
        self.assertEqual(derived['props'], {'Value': '74TST01', 'Footprint': DIP8,
                                            'Datasheet': 'https://example.com/74tst01.pdf',
                                            'ki_fp_filters': 'DIP*W7.62mm*'})
        self.assertEqual(derived['pins'], base['pins'])

    def test_load_footprints_reads_pad_centres_and_sizes(self):
        fps = kc.load_footprints(SHARED)
        self.assertEqual(sorted(fps), [DIP8, R0603])
        self.assertEqual(fps[R0603], {'1': [(-0.85, 0.0, 0.9, 0.95)], '2': [(0.85, 0.0, 0.9, 0.95)]})
        self.assertEqual(fps[DIP8]['2'], [(0.0, 2.54, 1.6, 1.6)])
        self.assertEqual(fps[DIP8]['3'], [(0.0, 5.08, 1.6, 1.6)])
        self.assertEqual(fps[DIP8][''], [(3.81, 3.81, 1.0, 1.0)])
        self.assertEqual(len(fps[DIP8]), 9)

    def test_kicad_version_reads_the_bundle_plist(self):
        self.assertEqual(kc.kicad_version(SHARED), '10.0.8')
        self.assertEqual(kc.kicad_version(os.path.join(KICAD, 'nowhere', 'SharedSupport')), '(unknown version)')
        with tempfile.TemporaryDirectory() as tmp:
            os.makedirs(os.path.join(tmp, 'SharedSupport'))
            with open(os.path.join(tmp, 'Info.plist'), 'wb') as fh:
                fh.write(b'<plist version="1.0"><dict></dict></plist>')
            self.assertEqual(kc.kicad_version(os.path.join(tmp, 'SharedSupport')), '(unknown version)')

    def test_find_kicad_takes_the_argument_then_the_app_then_exits(self):
        self.assertEqual(kc.find_kicad('/some/dir'), '/some/dir')
        with tempfile.TemporaryDirectory() as tmp:
            os.makedirs(os.path.join(tmp, 'Contents', 'SharedSupport', 'symbols'))
            with mock.patch.object(kc.os.path, 'expanduser', return_value=tmp):
                self.assertEqual(kc.find_kicad(None), os.path.join(tmp, 'Contents', 'SharedSupport'))
            with mock.patch.object(kc.os.path, 'expanduser', return_value=os.path.join(tmp, 'none')), \
                    mock.patch.object(kc.os.path, 'isdir', return_value=False):
                with self.assertRaises(SystemExit) as e:
                    kc.find_kicad(None)
        self.assertEqual(e.exception.code, 'KiCad not found: pass --kicad <its SharedSupport directory>')


class ExpandTests(unittest.TestCase):
    PAD = {'width': 1, 'height': 0.5}

    def test_dual_with_slots_and_a_centre_pad(self):
        fp = {'style': 'dual', 'pins': 5, 'perSide': 2, 'pitch': 1, 'span': 4, 'pad': self.PAD,
              'slots': [1, 2, 3, 4, 0], 'centre': {'width': 2, 'height': 2}}
        self.assertEqual(kc.expand(fp), {'1': [(-2, -0.5, 1, 0.5)], '2': [(-2, 0.5, 1, 0.5)], '3': [(2, 0.5, 1, 0.5)],
                                         '4': [(2, -0.5, 1, 0.5)], '5': [(0, 0, 2, 2)]})

    def test_dual_without_slots_names_its_centre_pad_null(self):
        fp = {'style': 'dual', 'pins': 2, 'pitch': 1, 'span': 2, 'pad': self.PAD, 'centre': {'width': 1, 'height': 1}}
        self.assertEqual(kc.expand(fp), {'1': [(-1, 0, 1, 0.5)], '2': [(1, 0, 1, 0.5)], 'NULL': [(0, 0, 1, 1)]})

    def test_quad_turns_the_pads_of_the_top_and_bottom_rows(self):
        fp = {'style': 'quad', 'pins': 4, 'sides': [1, 1, 1, 1], 'pitch': 1, 'spanX': 4, 'spanY': 6, 'pad': self.PAD}
        self.assertEqual(kc.expand(fp), {'1': [(-2, 0, 1, 0.5)], '2': [(0, 3, 0.5, 1)], '3': [(2, 0, 1, 0.5)],
                                         '4': [(0, -3, 0.5, 1)]})

    def test_grid_header_explicit_and_unknown_styles(self):
        grid = {'style': 'grid', 'pins': 2, 'rows': 2, 'columns': 2, 'pitch': 1, 'balls': ['A1', 'B2'], 'pad': self.PAD}
        self.assertEqual(kc.expand(grid), {'A1': [(-0.5, -0.5, 1, 0.5)], 'B2': [(0.5, 0.5, 1, 0.5)]})
        header = {'style': 'header', 'pins': 4, 'rows': 2, 'pitch': 2, 'firstRowY': 1, 'rowPitch': 3, 'pad': self.PAD}
        self.assertEqual(kc.expand(header), {'1': [(-1, 1, 1, 0.5)], '2': [(-1, 4, 1, 0.5)], '3': [(1, 1, 1, 0.5)],
                                             '4': [(1, 4, 1, 0.5)]})
        explicit = {'style': 'explicit', 'pins': 3, 'pads': [
            {'pin': 1, 'x': 1, 'y': 2, 'width': 3, 'height': 4}, {'ball': 'A1', 'x': 0, 'y': 0, 'width': 1, 'height': 1},
            {'pin': 0, 'x': 5, 'y': 5, 'width': 2, 'height': 2}]}
        self.assertEqual(kc.expand(explicit), {'1': [(1, 2, 3, 4)], 'A1': [(0, 0, 1, 1)], 'NULL': [(5, 5, 2, 2)]})
        self.assertEqual(kc.expand({'style': 'outline', 'pins': 0}), {})

    def test_load_ours_reads_every_kind_but_the_non_part_files(self):
        with mock.patch.object(kc, 'PARTS_DIR', PARTS):
            footprints, parts = kc.load_ours()
        self.assertEqual(sorted(footprints), ['DIP-8', 'DIP-8-WIDE', 'QFN-4', 'RES-0603'])
        self.assertEqual([(p['kind'], p['name'], p['fp'], p['flags']) for p in parts], [
            ('gate', 'SN74TST00N', 'DIP-8', []), ('gate', 'SN74TST01N', None, []),
            ('gate', 'SN54TST00J', 'DIP-8-WIDE', ['military']), ('gate', 'XYZ123', 'QFN-4', []),
            ('gate', 'WIDE8', 'DIP-8-WIDE', []), ('resistor', 'RC0603', 'RES-0603', []), ('resistor', 'R-AXIAL', None, [])])
        self.assertEqual(parts[3]['pins'][1], {'number': 2, 'name': 'Y', 'role': 'output'})


class SymbolMatchTests(unittest.TestCase):
    def test_build_symbol_keys_by_name_and_value(self):
        symbols = {'L:74HC00': {'props': {'Value': '74HC00'}}, 'L:R': {'props': {'Value': 'R'}},
                   'L:4017': {'props': {}}, 'L:12345': {'props': {'Value': '12345'}},
                   'L:Chip_A': {'props': {'Value': 'Chip-B'}}}
        self.assertEqual(dict(kc.build_symbol_keys(symbols)),
                         {'74HC00': ['L:74HC00'], '4017': ['L:4017'], 'CHIPA': ['L:Chip_A'], 'CHIPB': ['L:Chip_A']})

    def test_find_symbols(self):
        keys = {'74HC00': ['L:74HC00'], '74HC': ['L:74HC']}
        self.assertEqual(kc.find_symbols(keys, 'SN74HC00N'), (['L:74HC00'], False))
        self.assertEqual(kc.find_symbols(keys, 'SNJ54HC00J', military=True), (['L:74HC00'], True))
        self.assertEqual(kc.find_symbols(keys, '74HC00', military=True), (['L:74HC00'], False))
        self.assertEqual(kc.find_symbols(keys, '74HC001'), ([], False))
        self.assertEqual(kc.find_symbols(keys, '1274HC00'), ([], False))
        self.assertEqual(kc.find_symbols(keys, 'NE555'), ([], False))
        # The part's own number matches whole; its 74 spelling (74HC00) is shorter than that match, so it is not used.
        self.assertEqual(kc.find_symbols({'SN54HC00': ['L:SN54HC00'], '74HC00': ['L:74HC00']}, 'SN54HC00',
                                         military=True), (['L:SN54HC00'], False))


class PinTests(unittest.TestCase):
    def test_canon(self):
        self.assertEqual([kc.canon(p) for p in ('VDD', 'V+', 'VCC2A', 'VDDB', 'GND3', 'VSS', 'VEE', 'NC', 'NC4',
                                                'DNC', 'CLK')],
                         ['VCC', 'VCC', 'VCC2A', 'VCCB', 'GND3', 'GND', 'VEE', 'NC', 'NC', 'NC', 'CLK'])

    def test_toks(self):
        self.assertIsNone(kc.toks(None))
        self.assertIsNone(kc.toks(''))
        self.assertIsNone(kc.toks('~'))
        self.assertEqual(kc.toks('~{OE}'), {'OE'})
        self.assertEqual(kc.toks('2Y_BAR'), {'2YBAR', '2Y', 'YBAR', 'Y'})
        self.assertEqual(kc.toks('A/B'), {'A', 'B', 'AB'})
        self.assertEqual(kc.toks('-'), {''})
        self.assertEqual(kc.toks('VDD'), {'VCC'})

    def test_pclass_and_role_ok(self):
        self.assertEqual([kc.pclass(t) for t in (None, {'GND2'}, {'VCC'}, {'A'})], [None, 'gnd', 'pwr', None])
        cases = [('input', 'passive', True), ('input', 'weird', True), ('nc', 'no_connect', True),
                 ('input', 'unconnected', False), ('ground', 'power_in', True), ('input', 'power_out', False),
                 ('power', 'input', False), ('input', 'input', True), ('output', 'input', False),
                 ('output', 'tri_state', True), ('input', 'output', False)]
        for orole, ktype, ok in cases:
            self.assertEqual(kc.role_ok(orole, ktype), ok, (orole, ktype))

    def test_pin_compare_names_directions_and_power(self):
        opins = [pin(1, 'A'), pin(2, 'DNU', 'nc'), pin(3, 'NC', 'absent'), pin(4, 'Q', 'output'),
                 pin(5, 'VCC', 'power'), pin(6, 'CLK'), pin(7, 'D'), pin(8, 'OE'), pin(9, 'NC', 'nc'),
                 pin(10, 'RST', 'ground')]
        kpins = {'1': ('A', 'input'), '2': ('X', 'input'), '5': ('GND', 'power_in'), '6': ('CP', 'output'),
                 '7': ('', 'input'), '8': ('G', 'input'), '9': ('Y', 'input'), '10': ('VCC', 'power_in')}
        self.assertEqual(kc.pin_compare(opins, kpins), dict(
            exact=2, conv=1, convl=[('8', 'OE', 'G')], extra=[], ep=[],
            bad=[('2', 'DNU', 'X', 'ours is NC'), ('4', 'Q', None, 'no such pin in KiCad'),
                 ('5', 'VCC', 'GND', 'power/ground differ'), ('6', 'CLK', 'CP', 'output vs input'),
                 ('10', 'RST', 'VCC', 'power/ground differ')]))

    def test_pin_compare_unnamed_kicad_pin_with_a_clashing_direction(self):
        r = kc.pin_compare([pin(1, 'A', 'power')], {'1': ('~', 'input')})
        self.assertEqual(r['bad'], [('1', 'A', '~', 'input vs power')])

    def test_pin_compare_bus_numbered_differently_or_names_swapped(self):
        bus = [pin(1, 'A1'), pin(2, 'A2'), pin(3, 'A3')]
        r = kc.pin_compare(bus, {'1': ('A0', 'input'), '2': ('A1', 'input'), '3': ('A2', 'input')})
        self.assertEqual((r['exact'], r['conv'], r['bad']), (0, 3, []))
        self.assertEqual(r['convl'], [('3', 'A3', 'A2'), ('1', 'A1', 'A0'), ('2', 'A2', 'A1')])
        swapped = [pin(1, 'RD'), pin(2, 'WR')]
        r = kc.pin_compare(swapped, {'1': ('WR', 'input'), '2': ('RD', 'input')})
        self.assertEqual(r['bad'], [('1', 'RD', 'WR', 'name found on pin 2'), ('2', 'WR', 'RD', 'name found on pin 1')])
        uneven = [pin(1, 'B1'), pin(2, 'B3'), pin(3, 'B9')]
        r = kc.pin_compare(uneven, {'1': ('B0', 'input'), '2': ('B1', 'input'), '3': ('B3', 'input')})
        self.assertEqual(r['conv'], 1)
        self.assertEqual(r['bad'], [('1', 'B1', 'B0', 'name found on pin 2'), ('2', 'B3', 'B1', 'name found on pin 3')])
        rotated = [pin(1, 'C1'), pin(2, 'C2'), pin(3, 'C3')]
        r = kc.pin_compare(rotated, {'1': ('C3', 'input'), '2': ('C1', 'input'), '3': ('C2', 'input')})
        self.assertEqual((r['conv'], r['bad']), (3, []))

    def test_pin_compare_extra_kicad_pins_and_an_exposed_pad(self):
        opins = [pin(1, 'A'), pin(2, 'B')]
        k = {'1': ('A', 'input'), '2': ('B', 'input'), '3': ('EP', 'passive'), '4': ('NC', 'passive'),
             '5': ('X', 'no_connect')}
        r = kc.pin_compare(opins, k)
        self.assertEqual((r['extra'], r['ep']), ([], [('3', 'EP')]))
        k['6'] = ('Q', 'output')
        r = kc.pin_compare(opins, k)
        self.assertEqual((r['extra'], r['ep']), (['3', '6'], []))
        r = kc.pin_compare(opins, {'1': ('A', 'input'), '2': ('B', 'input'), '3': ('~', 'input')})
        self.assertEqual((r['extra'], r['ep']), (['3'], []))

    def test_pin_compare_by_ball_name(self):
        opins = [pin(None, 'VCC', 'power', ball='A1'), pin(None, 'X', ball='A2'), pin(None, 'Y', ball='A3'),
                 pin(None, 'NC', 'nc', ball='B1')]
        for p in opins:
            del p['number']
        r = kc.pin_compare(opins, {'1': ('VDD', 'power_in'), '2': ('', 'input')})
        self.assertEqual(r, dict(ball=True, exact=2, total=3, miss=[('A3', 'Y')], conv=0, convl=[], bad=[], extra=[],
                                 ep=[]))

    def test_pin_status(self):
        def status(**pr):
            base = dict(exact=6, conv=0, bad=[], extra=[])
            base.update(pr)
            return kc.pin_status({'pr': base})
        self.assertEqual(status(ball=True), 'n/a')
        self.assertEqual(status(), 'Yes')
        self.assertEqual(status(conv=2), 'Names differ')
        self.assertEqual(status(bad=[1], extra=[2]), 'Partial')
        self.assertEqual(status(exact=1, bad=[1, 2]), 'No')
        self.assertEqual(status(exact=0, bad=[]), 'Yes')


class FootprintTests(unittest.TestCase):
    def setUp(self):
        dual = {'style': 'dual', 'pins': 8, 'pitch': 2.54, 'span': 7.62, 'pad': {'width': 1.6, 'height': 1.6}}
        self.ours = {'DIP-8': dual, 'QFN-4': {'style': 'quad', 'pins': 4, 'sides': [1, 1, 1, 1], 'pitch': 0.5,
                                              'spanX': 2, 'spanY': 2, 'pad': {'width': 0.3, 'height': 0.6}},
                     'RES-0603': {'style': 'explicit', 'pins': 2, 'pads': [
                         {'pin': 1, 'x': -0.8, 'y': 0, 'width': 1, 'height': 1},
                         {'pin': 2, 'x': 0.8, 'y': 0, 'width': 1, 'height': 1}]},
                     'BGA-2': {'style': 'grid', 'pins': 2, 'rows': 1, 'columns': 2, 'pitch': 1, 'balls': ['A1', 'A2'],
                               'pad': {'width': 0.5, 'height': 0.5}}}
        self.parts = [{'fp': 'DIP-8', 'kind': 'gate'}, {'fp': 'RES-0603', 'kind': 'resistor'},
                      {'fp': 'QFN-4', 'kind': 'gate'}, {'fp': 'BGA-2', 'kind': 'gate'}, {'fp': None, 'kind': 'gate'}]
        thermal = dict(kc.load_footprints(SHARED)[DIP8], **{'9': [(3.81, 3.81, 3, 3)]})
        self.kicad = dict(kc.load_footprints(SHARED), **{
            'Package_DIP:DIP-8_EP': thermal,
            'Resistor_SMD:R_0805_2012Metric': {'1': [(-1, 0, 1, 1)], '2': [(1, 0, 1, 1)]},
            'Package_SO:Two_Pads': {'1': [(-0.8, 0, 1, 1)], '2': [(0.8, 0, 1, 1)]},
            'Package_BGA:BGA-2': {'A1': [(0, 0, 1, 1)], 'A2': [(1, 0, 1, 1)], 'MP1': [(5, 5, 1, 1)]},
            'Connector:Shield': {'SH': [(0, 0, 1, 1)]}})
        self.symbols = kc.load_symbols(SHARED)
        self.finder = kc.Footprints(self.ours, self.parts, self.kicad, self.symbols)

    def test_deviation_tries_rotations_and_a_mirror(self):
        ours = {'1': [(0, 0, 1, 1)], '2': [(2, 0, 1, 1)]}
        self.assertEqual(kc.deviation(ours, {'1': [(0, 0, 1, 1)], '2': [(0, 2, 1, 1)]}, ['1', '2']), 0)
        self.assertAlmostEqual(kc.deviation(ours, {'1': [(0, 0, 1, 1)], '2': [(3, 0, 1, 1)]}, ['1', '2']), 0.5)
        self.assertEqual(kc.centre([(0, 0), (2, 4)]), (1, 2))
        self.assertEqual([kc.tier(d) for d in (0.1, 0.11, 0.5, 0.51)], [0, 1, 1, 2])

    def test_index_and_candidates_include_one_extra_thermal_pad(self):
        self.assertNotIn(frozenset(), self.finder.index)
        self.assertEqual(self.finder.index[frozenset({'A1', 'A2'})], ['Package_BGA:BGA-2'])
        self.assertEqual(self.finder.candidates('DIP-8'), [(0.0, 'Package_DIP:DIP-8_EP'), (0.0, DIP8)])
        self.assertIs(self.finder.candidates('DIP-8'), self.finder.cands['DIP-8'])
        self.assertEqual(self.finder.candidates('BGA-2'), [(0.0, 'Package_BGA:BGA-2')])

    def test_pool_is_the_symbol_footprint_and_its_filters(self):
        self.assertEqual(self.finder.pool('74xx:74TST00'), {DIP8})
        self.symbols['74xx:74TST00']['props']['Footprint'] = 'Gone:Footprint'
        self.symbols['74xx:74TST00']['props']['ki_fp_filters'] = 'R_0805*'
        self.assertEqual(self.finder.pool('74xx:74TST00'), {'Resistor_SMD:R_0805_2012Metric'})

    def test_allowed_libraries_by_kind(self):
        self.assertTrue(self.finder.allowed(DIP8, 'DIP-8'))
        self.assertFalse(self.finder.allowed('Package_SO:Two_Pads', 'RES-0603'))
        self.assertTrue(self.finder.allowed(R0603, 'RES-0603'))
        self.assertFalse(self.finder.allowed(R0603, 'DIP-8'))

    def test_result(self):
        self.assertEqual(self.finder.result('DIP-8', '74xx:74TST00'), ('Yes', DIP8, 0.0, True))
        self.assertEqual(self.finder.result('RES-0603', None), ('Yes', R0603, 0.05, False))
        self.assertEqual(self.finder.result('QFN-4', None), ('None', None, None, False))
        self.finder.kicad.pop(R0603)
        self.finder.cands.clear()
        self.finder.index[frozenset({'1', '2'})].remove(R0603)
        self.assertEqual(self.finder.result('RES-0603', None), ('Close', 'Resistor_SMD:R_0805_2012Metric', 0.2, False))


def row(**r):
    base = dict(name='P1', kind='gate', sym='L:S', mil=False, nsyms=1, fp='F', nk=8, np=8,
                pr=dict(exact=8, conv=0, convl=[], bad=[], extra=[], ep=[]), fr=('Yes', 'K:Z', 0.0, True))
    base.update(r)
    return base


class NotesTests(unittest.TestCase):
    def test_text_helpers(self):
        self.assertEqual(kc.esc('a|b'), 'a\\|b')
        self.assertEqual(kc.q('a|b'), '`a\\|b`')
        self.assertEqual(kc.short(None), '-')
        self.assertEqual(kc.short('x' * 18), 'x' * 18)
        self.assertEqual(kc.short('x' * 19), 'x' * 17 + '…')

    def test_pin_notes_by_ball(self):
        pr = dict(ball=True, exact=2, total=3, miss=[('A3', 'Y')])
        self.assertEqual(kc.pin_notes({'pr': pr}), ["ours names pins by ball (A1…), KiCad's symbol numbers them; "
                                                    "compared by name: 2 of 3 match (no KiCad name for A3 Y)"])
        pr['miss'] = []
        self.assertEqual(kc.pin_notes({'pr': pr}), ["ours names pins by ball (A1…), KiCad's symbol numbers them; "
                                                    "compared by name: 2 of 3 match"])

    def test_pin_notes_list_each_disagreement(self):
        pr = dict(exact=0, conv=3, convl=[('5', 'A0', 'A1'), ('6', 'LONGNAME_123456', 'Q')], ep=[],
                  bad=[('1', 'A', None, 'no such pin in KiCad'), ('2', 'B', 'X', 'ours is NC'),
                       ('3', 'C', 'Y', 'power/ground differ'), ('4', 'D', 'Z', 'x')],
                  extra=['10', '11', '12', '13', '14'])
        self.assertEqual(kc.pin_notes({'pr': pr, 'nk': 20, 'np': 14}), [
            '4 pins differ: pin 1 `A`: not in KiCad symbol; pin 2: ours `B`, KiCad `X` (ours is NC); '
            'pin 3: ours `C`, KiCad `Y` (power/ground differ) …+1',
            'KiCad has pins 10, 11, 12, 13… that ours lacks',
            '3 pin names differ in convention (5 `A0`/`A1`, 6 `LONGNAME_…`/`Q`…)',
            'KiCad symbol has 20 pins, ours 14'])
        pr = dict(exact=0, conv=1, convl=[('5', 'A0', 'A1')], ep=[('9', 'EP')], bad=[('2', 'B', 'X', 'ours is NC')],
                  extra=['8'])
        self.assertEqual(kc.pin_notes({'pr': pr, 'nk': 9, 'np': 8}), [
            '1 pin differ: pin 2: ours `B`, KiCad `X` (ours is NC)', 'KiCad has pin 8 that ours lacks',
            'KiCad adds exposed-pad pin 9 (`EP`)', '1 pin names differ in convention (5 `A0`/`A1`)'])

    def test_fp_notes(self):
        self.assertEqual(kc.fp_notes({'fp': 'F', 'fr': ('None', None, None, False)}),
                         ['no KiCad footprint with ours `F` pad set'])
        self.assertEqual(kc.fp_notes({'fp': 'F', 'fr': ('No', 'K:X', 1.234, False)}),
                         ['ours `F`: nothing in KiCad within 0.5 mm; nearest is `K:X`, 1.2 mm off'])
        self.assertEqual(kc.fp_notes({'fp': 'F', 'fr': ('Close', 'K:Y', 0.2, False)}),
                         ["ours `F` ≈ KiCad `K:Y`, max pad offset 0.20 mm (not in the symbol's footprint filter)"])
        self.assertEqual(kc.fp_notes({'fp': 'F', 'fr': ('Yes', 'K:Z', 0.0, True)}),
                         ['ours `F` = KiCad `K:Z`, max pad offset 0.00 mm'])

    def test_table_row(self):
        self.assertEqual(kc.table_row(row(name='N|X', mil=True, nsyms=2)),
                         '| N\\|X | Yes | Yes | Symbol `L:S` (54 series → 74 symbol) (best of 2). '
                         'ours `F` = KiCad `K:Z`, max pad offset 0.00 mm |')
        self.assertEqual(kc.table_row(row()), '| P1 | Yes | Yes | Symbol `L:S`. ours `F` = KiCad `K:Z`, max pad offset '
                                              '0.00 mm |')


class ReportTests(unittest.TestCase):
    class Finder:
        def result(self, fpid, symkey):
            return {'A': ('Close', 'K:A', 0.3, False), 'B': ('No', 'K:B', 2.0, False)}[fpid]

    def test_groups_pin_disagreements_and_counts_no_symbol_footprints(self):
        bad = [(str(n), c, None if n == 4 else c.lower(), 'x') for n, c in zip(range(1, 6), 'ABCDE')]
        no = dict(exact=1, conv=0, convl=[], bad=bad, extra=[], ep=[])
        partial = dict(exact=7, conv=0, convl=[], bad=bad[:1], extra=[], ep=[])
        rows = [row(name='P2', pr=no), row(name='P1', pr=no), row(name='P3', pr=partial),
                row(name='P4', pr=partial, nk=9), row(name='P5', pr=dict(partial, ep=[('9', 'EP')])),
                row(name='P6', pr=dict(ball=True, exact=1, total=1, miss=[], bad=[], extra=[], conv=0, convl=[]))]
        parts = [{'kind': 'gate', 'name': 'P%d' % n, 'fp': 'F'} for n in range(1, 7)] + [
            {'kind': 'gate', 'name': 'Q1', 'fp': 'A'}, {'kind': 'gate', 'name': 'Q2', 'fp': 'B'},
            {'kind': 'mystery', 'name': 'M1', 'fp': 'B'}, {'kind': 'mystery', 'name': 'M2', 'fp': None}]
        text = kc.report(parts, rows, self.Finder(), '9.0', 1234, 56789)
        self.assertIn("KiCad 9.0's stock symbol and footprint libraries (`symbols` and `footprints` in KiCad's "
                      "`SharedSupport`, 1,234 symbols, 56,789 footprints)", text)
        self.assertIn('**6 of 10 parts (60%) have a KiCad symbol of the same part number** and are compared below. '
                      'The other 4 have none', text)
        self.assertIn('| **Pins Match** | 0 | 0 | 3 | 2 | 1 |', text)
        self.assertIn('| **Footprint Match** | 6 | 0 | 0 |\n', text)
        self.assertIn('| P1 | 2 | No | `L:S` | pin 1: ours `A`, KiCad `a`; pin 2: ours `B`, KiCad `b`; pin 3: ours `C`, '
                      'KiCad `c`; pin 4: ours `D`, KiCad `—` …+1 |\n| P3 | 1 | Partial | `L:S` | pin 1: ours `A`, '
                      'KiCad `a` |\n\n## Summary by kind', text)
        self.assertIn('| [gate](#gate) | 8 | 6 | 0 | 0 | 3 | 2 | 1 | 6 | 0 | 0 |\n| mystery | 2 | 0 | | | | | | | | |\n',
                      text)
        self.assertIn('## Gates (`gate`): 6 of 8 parts\n', text)
        self.assertNotIn('(`mystery`)', text)
        self.assertIn('| gate | 2 | no symbol of that part number in KiCad |\n| mystery | 2 | no symbol of that part '
                      'number in KiCad |\n', text)
        self.assertIn('| A | gate 1 | Close | KiCad `K:A`, max pad offset 0.30 mm |\n'
                      '| B | gate 1, mystery 1 | No | nothing in KiCad within 0.5 mm; nearest is `K:B`, 2.0 mm off |\n',
                      text)
        self.assertTrue(text.endswith('\n_2 footprints: Yes 0, Close 1, No 1, none 0._\n'))


class MainTests(unittest.TestCase):
    def test_writes_the_report_for_the_recorded_library(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = os.path.join(tmp, 'report.md')
            err = io.StringIO()
            with mock.patch.object(kc, 'PARTS_DIR', PARTS), \
                    mock.patch.object(sys, 'argv', ['kicad-compare.py', '--kicad', SHARED, '--out', out]), \
                    contextlib.redirect_stderr(err):
                kc.main()
            with open(out) as fh:
                text = fh.read()
        self.assertEqual(err.getvalue(), 'reading KiCad symbols…\nreading KiCad footprints…\ncomparing 7 parts…\n'
                                         '3 of 7 parts have a KiCad symbol; wrote %s\n' % out)
        self.assertTrue(text.startswith('# KiCad comparison\n\nCompares the part library'))
        self.assertIn("KiCad 10.0.8's stock symbol", text)
        self.assertIn('2 symbols, 2 footprints', text)
        self.assertIn('**3 of 7 parts (43%) have a KiCad symbol', text)
        self.assertIn('| **Pins Match** | 2 | 1 | 0 | 0 | 0 |', text)
        self.assertIn('| **Footprint Match** | 1 | 0 | 1 |', text)
        self.assertIn('| SN54TST00J | Yes | No | Symbol `74xx:74TST00` (54 series → 74 symbol). ours `DIP-8-WIDE`: '
                      'nothing in KiCad within 0.5 mm; nearest is `Package_DIP:DIP-8_W7.62mm`, 3.8 mm off |', text)
        self.assertIn('| SN74TST00N | Yes | Yes | Symbol `74xx:74TST00`. ours `DIP-8` = KiCad '
                      '`Package_DIP:DIP-8_W7.62mm`, max pad offset 0.00 mm |', text)
        self.assertIn('| SN74TST01N | Names differ | None | Symbol `74xx:74TST01`. 1 pin names differ in convention '
                      '(7 `W`/`Z`). no KiCad footprint with ours `None` pad set |', text)
        self.assertIn('| [gate](#gate) | 5 | 3 | 2 | 1 | 0 | 0 | 0 | 1 | 0 | 1 |\n| resistor | 2 | 0 | | | | | | | | |',
                      text)
        self.assertIn('| resistor | 2 | KiCad has one generic `R` symbol |', text)
        self.assertIn('| DIP-8-WIDE | gate 1 | No | nothing in KiCad within 0.5 mm; nearest is '
                      '`Package_DIP:DIP-8_W7.62mm`, 3.8 mm off |\n'
                      '| QFN-4 | gate 1 | None | no KiCad footprint in a fitting library has the same pad set |\n'
                      '| RES-0603 | resistor 1 | Yes | KiCad `Resistor_SMD:R_0603_1608Metric`, max pad offset 0.05 mm |',
                      text)
        self.assertTrue(text.endswith('_3 footprints: Yes 1, Close 0, No 1, none 1._\n'))

    def test_command_line_over_the_real_parts(self):
        with tempfile.TemporaryDirectory() as tmp:
            out = os.path.join(tmp, 'report.md')
            r = support.run('kicad-compare.py', '--kicad', SHARED, '--out', out)
            self.assertEqual(r.returncode, 0, r.stderr)
            with open(out) as fh:
                text = fh.read()
        lines = r.stderr.splitlines()
        self.assertEqual(lines[:2], ['reading KiCad symbols…', 'reading KiCad footprints…'])
        self.assertRegex(lines[2], r'^comparing \d+ parts…$')
        self.assertRegex(lines[3], r'^\d+ of \d+ parts have a KiCad symbol; wrote ' + out.replace('.', r'\.') + '$')
        self.assertEqual(r.stdout, '')
        self.assertTrue(text.startswith('# KiCad comparison\n'))
        self.assertIn('## Parts with no KiCad symbol\n', text)


if __name__ == '__main__':
    unittest.main()

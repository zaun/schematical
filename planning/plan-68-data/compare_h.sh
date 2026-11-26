#!/bin/sh
# For each old program: its new programs' case lines, sorted, against the recorded output.
B=/tmp/p68-build
D=/Users/justinzaun/Development/logic/.claude/worktrees/P-68/planning/plan-68-data/test-output-before
cd /tmp || exit 1
check() {
  old=$1; shift
  for p in "$@"; do $B/$p; echo "exit=$? ($p)" >&2; done 2>/tmp/p68-exits.txt | grep -E '^(ok|PASS|pass|FAIL)' | grep -vE '^PASS$' | sort > /tmp/p68-new-$old.txt
  grep -E '^(ok|PASS|pass|FAIL)' $D/$old.txt | grep -vE '^PASS$' | sort > /tmp/p68-old-$old.txt
  if diff -q /tmp/p68-old-$old.txt /tmp/p68-new-$old.txt >/dev/null; then
    echo "$old: same $(wc -l < /tmp/p68-new-$old.txt) lines; $(grep -c 'exit=0' /tmp/p68-exits.txt)/$# exit 0"
  else
    echo "$old: DIFFERENT"; diff /tmp/p68-old-$old.txt /tmp/p68-new-$old.txt | head -5
  fi
}
check library_check library_check library_logic_check library_parts_check library_pins_check library_footprint_check library_voltage_check
check cpu_test cpu_test cpu_z80_test cpu_65816_test cpu_io_board_test
check counter_test counter_test counter_div_test
check charlcd_test charlcd_test charlcd_4row_test charlcd_board_test
check shift_test shift_test shift_special_test
check pio_test pio_test pio_via_test
check circuit_check circuit_check circuit_adc_check
check serialmem_test serialmem_test serialmem_flash_test
check latch_test latch_test

set -e
S=/private/tmp/claude-501/-Users-justinzaun-Development-logic/b09aff1a-5b91-4531-aa82-d3e9dc3331bb/scratchpad
B=$S/bcov
rm -rf $B; mkdir -p $B/prof
start=$(date +%s)
cmake -S /Users/justinzaun/Development/logic/backend -B $B -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fprofile-instr-generate -fcoverage-mapping" -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate" > $B/cmake.log
cmake --build $B -j 10 > $B/build.log
echo "build $(( $(date +%s)-start ))s"
LLVM_PROFILE_FILE=$B/prof/%p.profraw ctest --test-dir $B -j 10 > $B/ctest.log 2>&1 || true
echo "test $(( $(date +%s)-start ))s"
tail -5 $B/ctest.log
xcrun llvm-profdata merge -sparse $B/prof/*.profraw -o $B/all.profdata
objs=""; for t in $(ls $B | grep -E '_(test|check)$'); do [ -x $B/$t ] && objs="$objs -object $B/$t"; done
xcrun llvm-cov report $objs -instr-profile $B/all.profdata > $B/report.txt
tail -3 $B/report.txt

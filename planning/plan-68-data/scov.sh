S=/private/tmp/claude-501/-Users-justinzaun-Development-logic/b09aff1a-5b91-4531-aa82-d3e9dc3331bb/scratchpad
cd /Users/justinzaun/Development/logic/schematical
rm -rf $S/scov.xcresult
start=$(date +%s)
perl -e 'alarm 3000; exec @ARGV' xcodebuild test -project schematical.xcodeproj -scheme schematical -destination 'platform=macOS' -derivedDataPath $S/dd -enableCodeCoverage YES -resultBundlePath $S/scov.xcresult -skip-testing:schematicalTests/AutorouteModelTests/testTheSummaryOnAFU > $S/scov.log 2>&1
echo "exit $? after $(( $(date +%s)-start ))s"
pkill -f "$S/dd/Build/Products/Debug/schematical.app" || true
grep -E 'Executed [0-9]+ tests|TEST (SUCCEEDED|FAILED)' $S/scov.log | tail -3
xcrun xccov view --report --only-targets $S/scov.xcresult

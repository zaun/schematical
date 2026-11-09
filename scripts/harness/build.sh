#!/bin/sh
# Headless harness for the schematical app (first used in plan-03-D and
# plan-06-A; kept here so it is not lost with /tmp).
#
# It compiles the app's Swift sources (all but schematicalApp.swift) and the
# backend's C sources together with one extra main.swift into a CLI. The CLI
# is placed inside a copy of the Debug .app, so Bundle.main finds the part
# files. A main.swift drives SchematicDocument / SchematicFile directly and
# can render the app's real views offscreen with ImageRenderer. Every file in
# scripts/harness/support (shared helpers: Harness.render, Harness.open, …)
# is compiled in too.
#
# Usage:  scripts/harness/build.sh <main.swift> [out dir]
#   out dir defaults to /tmp/schematical-harness.
#   APP=<Debug schematical.app> picks the app bundle to copy; by default the
#   app is built with xcodebuild into <out dir>/dd.
# Then:   <out dir>/H.app/Contents/MacOS/harness <args>
set -e
MAIN=${1:?usage: build.sh <main.swift> [out dir]}
O=${2:-/tmp/schematical-harness}
R=$(cd "$(dirname "$0")/../.." && pwd)
mkdir -p "$O/obj" "$O/src"
# Top-level code is only allowed in a file named main.swift.
cp "$MAIN" "$O/src/main.swift"
MAIN=$O/src/main.swift

if [ -z "$APP" ]; then
  APP=$O/dd/Build/Products/Debug/schematical.app
  if [ ! -d "$APP" ]; then
    xcodebuild -project "$R/schematical/schematical.xcodeproj" -scheme schematical \
      -configuration Debug -derivedDataPath "$O/dd" CODE_SIGNING_ALLOWED=NO build >/dev/null
  fi
fi

B=$R/backend
INC="-I$B/include -I$B/src -I$B/src/engine -I$B/src/library -I$B/src/models -I$B/src/third_party/cjson"
rm -f "$O"/obj/*.o
for f in $(find "$B/src" -name '*.c'); do
  clang -c -O0 $INC "$f" -o "$O/obj/$(basename "$f" .c).o"
done

S=$R/schematical/schematical
swiftc -swift-version 5 -default-isolation MainActor -enable-bare-slash-regex \
  -enable-upcoming-feature MemberImportVisibility \
  -enable-upcoming-feature NonisolatedNonsendingByDefault \
  -enable-upcoming-feature InferIsolatedConformances \
  -import-objc-header "$S/schematical-Bridging-Header.h" $INC \
  $(find "$S" -name '*.swift' ! -name schematicalApp.swift) \
  $(find "$R/scripts/harness/support" -name '*.swift') "$MAIN" "$O"/obj/*.o \
  -o "$O/harness"

rm -rf "$O/H.app"
cp -R "$APP" "$O/H.app"
cp "$O/harness" "$O/H.app/Contents/MacOS/harness"
echo "$O/H.app/Contents/MacOS/harness"

#!/bin/zsh
# Loads a DigiKey page in a temporary tab of the Safari window that already has a
# DigiKey tab open (that window's session has passed Cloudflare's check) and prints
# the page text, or the result of a JavaScript expression.
#
#   scripts/safari-digikey.sh URL [wait_seconds] [javascript]
#
# Examples
#   scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/lcd-oled-character-and-numeric/99?keywords=7-segment%20lcd" 12 \
#     'Array.from(document.querySelectorAll("tr")).slice(1,30).map(r=>r.innerText.replace(/\s+/g," ").slice(0,200)).join("\n")'
#   scripts/safari-digikey.sh "https://www.digikey.com/en/products/filter/display-drivers/729" 10 'document.title'
#
# Needs: Safari > Develop > Developer Settings > Allow JavaScript from Apple Events, and
# a DigiKey page open in some Safari window (open one and tick Cloudflare's "Verify you
# are human" box once). Search-result URLs (/en/products/result?keywords=...) are
# challenged again; category filter URLs (/en/products/filter/<category>/<id>) are not.
# DigiKey-hosted PDFs (mm.digikey.com) are fetched with safari-digikey-pdf.sh.
URL="$1"; W="${2:-8}"; JS="${3:-document.body.innerText.slice(0,30000)}"
osascript - "$URL" "$W" "$JS" <<'OSA'
on run argv
  set theURL to item 1 of argv
  set w to (item 2 of argv) as integer
  set js to item 3 of argv
  tell application "Safari"
    set found to missing value
    repeat with win in windows
      repeat with t in tabs of win
        try
          if URL of t contains "digikey.com" and URL of t does not contain "oauth2" and name of t does not contain "Just a moment" then set found to win
        end try
      end repeat
      if found is not missing value then exit repeat
    end repeat
    if found is missing value then error "open a DigiKey page in Safari first"
    set cur to current tab of found
    tell found
      set t to make new tab with properties {URL:theURL}
      delay w
      try
        set r to do JavaScript js in t
      on error m
        close t
        set current tab of found to cur
        error m
      end try
      close t
      set current tab of found to cur
    end tell
    return r
  end tell
end run
OSA

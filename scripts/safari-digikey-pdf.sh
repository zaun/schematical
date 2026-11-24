#!/bin/zsh
# Downloads a PDF that DigiKey hosts (mm.digikey.com ...), which refuses curl, by
# fetching it inside a Safari tab on its own origin.
#
#   scripts/safari-digikey-pdf.sh PDF_URL OUTFILE
#
# Same Safari requirements as safari-digikey.sh.
URL="$1"; OUT="$2"
osascript - "$URL" <<'OSA' | base64 -D > "$OUT"
on run argv
  set theURL to item 1 of argv
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
      delay 8
      do JavaScript "window.__b=null; fetch(location.href).then(r=>r.arrayBuffer()).then(b=>{let s='';const u=new Uint8Array(b);for(let i=0;i<u.length;i+=8192)s+=String.fromCharCode.apply(null,u.subarray(i,i+8192));window.__b=btoa(s);});" in t
      delay 5
      set r to do JavaScript "window.__b" in t
      close t
      set current tab of found to cur
    end tell
    return r
  end tell
end run
OSA

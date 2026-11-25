#!/bin/zsh
# Sends probe commands (one per argument) to the running spike app and prints the result.
# SPIKE_ID picks the container (default jgz.schematical.spike59).
C=~/Library/Containers/${SPIKE_ID:-jgz.schematical.spike59}/Data
before=$(wc -c < $C/spike-out.txt 2>/dev/null || echo 0)
print -l -- "$@" > $C/spike-cmd.tmp
mv $C/spike-cmd.tmp $C/spike-cmd.txt
for i in {1..${SP_TRIES:-300}}; do
  sleep 0.2
  if [ -f $C/spike-out.txt ] && tail -c +$((before + 1)) $C/spike-out.txt | grep -q '^<done>$'; then
    tail -c +$((before + 1)) $C/spike-out.txt | grep -v '^<done>$'
    exit 0
  fi
done
echo "TIMEOUT waiting for the app"; tail -c +$((before + 1)) $C/spike-out.txt
exit 1

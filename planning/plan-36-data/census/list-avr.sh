#!/bin/zsh
# plan-36: the 8-bit AVR series of DigiKey category 685 (Microcontrollers), Microchip (150).
R=${1:-/tmp/p36/raw}
for pair in 22379:avr-90can 22381:avr-90pwm 23565:avr-90usb 123028:avr-ata66 22385:atmega 86846:atmega-fusa \
  22386:attiny 86834:attiny-fusa 81505:avr-da 85248:avr-db 103244:avr-dd 125257:avr-du 111670:avr-ea 120796:avr-eb \
  146654:avr-la 136982:avr-sd 49185:xmega-a1 49190:xmega-a1u 49186:xmega-a3 49188:xmega-a3b 49192:xmega-a3bu \
  49191:xmega-a3u 49189:xmega-a4 49193:xmega-a4u 49194:xmega-b1 49195:xmega-b3 49196:xmega-c3 49197:xmega-c4 \
  49198:xmega-d3 49199:xmega-d4 49200:xmega-e5 86858:megaavr-0 103243:tinyavr-2 89913:tinyavr-0 86833:tinyavr-1; do
  /tmp/p36/census/run.sh $R 685 150 ${pair%%:*} ${pair##*:} || break
done

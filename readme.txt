serial.c
--------

this sample is for an Atmel ATTiny85 (AVR uC)

to compile you need AVR-GCC toolchain for Linux.

install command for "debian like" GNU/Linux :
  sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc avrdude

to compile it                          : make
to read program size                   : make size
produce full disassembly file (.disasm): make disasm
to upload                              : make upload
  |-> i use AVRDUDE with an Arduino UNO (with ArduinoISP sketch)
to clean directory                     : make clean

info serial.c :
-> define USE_PRINTF for use stdio printf()
   code is bigger, make size return .text +1700 bytes
-> comment define USE_PRINTF for use custom serial_print() function
   produce tiny code, make size return .text +670 bytes

web site : http://source.perl.free.fr

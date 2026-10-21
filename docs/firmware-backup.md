# Preserve the installed satellite-rotor firmware

Flash readback was completed on 2026-10-21; see [the readback record](readback-2026-10-21.md).
The bootloader connection reset the Uno, with motor power disconnected. No
write/erase commands were issued. The procedure below remains general guidance;
EEPROM and restoration have not been verified.

An Uno normally stores compiled machine code, not the original .ino source.
Readback may preserve a restorable executable, but not the original comments,
names or project structure. First look for the original sketch on the old computer,
its backups, or the original satellite-rotor project/version. EEPROM may hold
calibration values separately from flash.

## Read-only procedure, after confirming the actual board and port

1. Disconnect the motor power supply and secure the rig; opening serial commonly
   toggles reset. Do not disconnect motor leads while the driver is energized.
2. Close serial monitors. Confirm ATmega328P Uno, its port and bootloader baud
   (standard Uno Optiboot normally uses 115200). Record shield wiring/jumpers.
3. Use AVRDUDE from Arduino's toolchain or its official distribution. The following
   commands are EXAMPLES requiring replacement of PORT, not an auto-run script:

```sh
avrdude -p atmega328p -c arduino -P PORT -b 115200 -n -U flash:r:rotor-flash-a.hex:i
avrdude -p atmega328p -c arduino -P PORT -b 115200 -n -U flash:r:rotor-flash-b.hex:i
cmp rotor-flash-a.hex rotor-flash-b.hex
```

The memory operations above are reads (`r`), and `-n` suppresses writes. Do not
add erase, upload/write, fuse, lock-bit, burn-bootloader, or signature-override
commands. A failed read is a reason to inspect compatibility, not erase the chip.

4. EEPROM access depends on the bootloader: some Optiboot builds do not support
   meaningful EEPROM reads. A returned file is not proof that calibration was
   captured. Check actual bootloader support; an ISP programmer may be required
   for verified EEPROM/fuse/lock reads. Do not erase to overcome read protection.
5. Verify image plausibility as well as two-read consistency; identical garbage
   is still garbage. Record tool version, board identity, read method, hashes,
   file size, application behavior/protocol, and calibration settings.
6. Treat bootloader coverage, fuse/lock settings, EEPROM and restoration as separate
   questions. A USB flash read alone is not a complete disaster-recovery image.
   If needed, demonstrate restoration on a spare compatible Uno rather than risk
   the sole working board.

Keep personal backups outside the public repository by default; flash/EEPROM may
contain station coordinates, callsigns or other configuration. Local `backups/`
is gitignored. Do not publish raw dumps without inspecting their contents.

References: [AVRDUDE memory operations](https://avrdudes.github.io/avrdude/7.3/avrdude_4.html)
and [Arduino build process](https://docs.arduino.cc/arduino-cli/sketch-build-process).

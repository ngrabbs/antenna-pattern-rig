# Sources and uncertainty

Reviewed 2026-10-21. The physical unit's own label, documentation and measurements
take priority over a matching model number from another supplier.

- [Zyltech manufacturer motor sheet, 17HD48002H-22B](https://cdck-file-uploads-europe1.s3.dualstack.eu-west-1.amazonaws.com/arduino/original/3X/6/a/6a780e4c048ae70a0d293b346b3e6114967d7532.pdf):
  supports 200 steps/rev; not a Longruner-specific certification.
- [Aideepen 600 P/R encoder sheet](https://m.media-amazon.com/images/I/91YvWuDZZ4L.pdf):
  quadrature 600 PPR, 5-24 V; similar family, exact unit wiring remains unverified.
- [Original encoder application paper](https://eprints.covenantuniversity.edu.ng/18650/1/Oguntosin_2019_J._Phys.__Conf._Ser._1299_012108.pdf):
  LPD3806 application, open-collector outputs and quadrature counting.
- [Analog Devices on microstepping](https://www.analog.com/en/resources/analog-dialogue/articles/mastering-precision-understanding-microstepping.html):
  resolution versus physical position accuracy and incremental torque.
- [Pololu DRV8825 carrier](https://www.pololu.com/product/2133/):
  mode table, timing, current limiting and A4988 differences. Do not transfer a
  carrier's VREF formula to an unidentified clone.
- [GRBL Uno CPU map](https://github.com/gnea/grbl/blob/master/grbl/cpu_map.h):
  conventional Uno STEP D2/D3/D4, DIR D5/D6/D7, ENABLE D8 assignments. Check actual
  shield wiring. This project does not use or modify GRBL.
- [SYSJOINT NanoVNA-F V2](https://www.sysjoint.com/NanoVNA-F_V2.html).
- [Rohde & Schwarz antenna testing discussion](https://scdn.rohde-schwarz.com/ur/pws/dl_downloads/premium_dl_miscellaneous/02_SatIndDays_3_Pfeifer_Guenter_Enabling_antenna_terminal_testing_for_New_Space_2.pdf).
- [Antenna project and simulated performance](https://codeberg.org/buildacubesat-project/bac-hardware/src/branch/main/rf/antenna/s-band-cross-patch).

No verified encoder absolute accuracy, maximum supported table load, driver
identity, shaft-to-table ratio, or assembled positioning accuracy is available yet.

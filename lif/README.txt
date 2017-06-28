lifutils.c 		LIF image functions, directory listing and file adding.extracting,renaming,deleting
lif             Stand alone version of the LIF utility part of the HP85disk project for linux

To get hep type: ./lif   or ./lif help
Stand alone version of LIF utilities for linux
HP85 Disk and Device Emulator
 (c) 2014-2017 by Mike Gore
 GNU version 3
-> https://github.com/magore/hp85disk
   GIT last pushed:   2017-06-28 14:32:26.251237991 -0400
   Last updated file: 2017-06-28 14:37:36.606681120 -0400
lif help
lif add lifimage lifname from_ascii_file
lif addbin lifimage lifname from_lif_file
lif create lifimage label directory_sectors sectors
lif del lifimage name
lif dir lifimage
lif extract lifimage lifname to_ascii_file
lif extractbin lifimage lifname to_lif_file
lif rename lifimage oldlifname newlifname

This is the final CS360 project 
It contains a.out, mydisk, diskimage
mydisk contains dirs, files in lowercase
diskimage contains dirs, files in uppercase

===========================================================
Run a.out    (defualt disk image = mydisk)
Run a.out -d (in debugging mode)
Enter menu to see supported commands
============================================================

To test mount-umount:

Run a.out on mydisk;
mount diskiamge /mnt
mount                    # display mounted file systems
cd /mnt; ls              # display /mnt contents; show cross mounting point
mkdir newdir; ls         # see newdir
cd newdir; pwd           # show CWD
cd ../../; pwd           # show cross mounting point                

umount diskimage         # umount
quit                     # exit project
===========================================================
If desired, check contents of diskimage under Linux
===========================================================

To test permissions:
Run a.out on mydisk
Enter cs to switch process to P1
try cd into dirs;
open file for WRITE
rmdir not belong to P1, et.


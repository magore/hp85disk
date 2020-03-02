Orginal Article found on Series80 Group
https://groups.io/g/hpseries80/topic/dondisks/655979

DonDisks
 Vassilis PREVELAKIS (series80.org)12/04/16   #1443  
On 4 Dec 2016, at 01:20, Dave Frederickson <dave9534@...> wrote:

I don't understand why LIF disc images are being distributed in this manner.  To have a LIF disc image with a file name of WS_FILE seems to be caused by misuse of LIFUTIL.  This HP-75C archive suffers from the same problem, http://www.forth.org/hp-75c/75Forth.zip.
Hi Dave,

I sooooo much agree with you. I believe the WS_FILE business was the fault of LIFUTIL which for some unfathomable reason assigned this name to the file (and maybe relied on this name to verify that its a single file rather than a LIF volume).

Having just got a PILBOX to use with my HP-IL stuff I went back to my LIF archives and promptly got hit by the WS_FILE business.

The idea of storing the original series 80 file in a structured file with a fake volume header, the directory metadata and finally the contents of the file, is a great idea, and in cases where you are looking for a single file, its much neater than having to deal with volumes.

Anyway, I thought of making a quick & dirty converter of LIF files into LIF volumes (that I could use with ilper (Linux version) and came up with this:

% cat swp2lif.sh 
#!/bin/sh -

FN=$1
FO=$2

FB=`echo $FN | sed -n 's/\..*$//p'`
printf "\200\0%-6.6s\0\0\0\2\10\0\0\0" $FB > $FO
dd if=$FN of=/dev/stdout bs=1 skip=16 count=240 >> $FO
dd if=/dev/zero of=/dev/stdout bs=256 count=1  >> $FO
printf "%-10.10s\340\211\0\0\0\4" $FB >> $FO
dd if=$FN of=/dev/stdout bs=1 skip=272 count=240 >> $FO
dd if=/dev/zero of=/dev/stdout bs=256 count=1  >> $FO
dd if=$FN of=/dev/stdout bs=256 skip=2 >> $FO
which has the bonus of changing the WS_FILE into the unix filename (without the .ext).

Now that few (if any) are using LIFUTUL, I propose that we start putting the real filename in the directory entry. If anybody wants to use LIFUTIL they can modify the above script to change the file name back to WS_FILE.

Best Regards

Vassilis

default :
	..\..\z_tools\make.exe img

ipl.bin : ipl.nas Makefile
	..\..\z_tools\nask.exe ipl.nas ipl.bin ipl.lst

helloos.img : ipl.bin Makefile
	..\..\z_tools\edimg.exe   imgin:../../z_tools/fdimg0at.tek   wbinimg src:ipl.bin len:512 from:0 to:0   imgout:helloos.img

asm :
	..\..\z_tools\make.exe -r ipl.bin
	
img :
	..\..\z_tools\make.exe -r helloos.img

clean :
	-del ipl.bin
	-del ipl.lst

src_only :
	../../z_tools/make.exe clean
	-del helloos.img
CC = icc12w
LIB = ilibw
CFLAGS =  -IC:\iccv712\include\ -e -D__ICC_VERSION=708 -D__BUILD=1191  -l -g -Wa-g 
ASFLAGS = $(CFLAGS) 
LFLAGS =  -LC:\iccv712\lib\ -g -nb:1191 -ucrt12initrm.o -btext:0x4000.0xFFFF -bdata:0x2000 -dinit_sp:0x4000 -fmots19 -dinitrm:0x21
FILES = Controller.o Flash.o Command.o EEProm.o mco.o user.o mcohw.o Interrupts.o Subroutines.o 

BUTTONBOX:	$(FILES)
	$(CC) -o BUTTONBOX $(LFLAGS) @BUTTONBOX.lk   -lfp12
	c:\iccv712\bin\SRecCvt -dt128a -o BUTTONBOXX.s19 BUTTONBOX.s19 
Controller.o: C:\iccv712\include\stdio.h C:\iccv712\include\stdarg.h C:\iccv712\include\_const.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\Interrupts.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\Flash.h .\..\REV3~1.05\EEProm.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h
Controller.o:	..\REV3~1.05\Controller.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\Controller.c
Flash.o: .\..\REV3~1.05\Flash.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\Subroutines.h
Flash.o:	..\REV3~1.05\Flash.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\Flash.c
Command.o: C:\iccv712\include\stdio.h C:\iccv712\include\stdarg.h C:\iccv712\include\_const.h C:\iccv712\include\string.h C:\iccv712\include\stdlib.h C:\iccv712\include\limits.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\Command.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\EEProm.h .\..\REV3~1.05\Interrupts.h
Command.o:	..\REV3~1.05\Command.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\Command.c
EEProm.o: .\..\REV3~1.05\EEProm.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\Subroutines.h
EEProm.o:	..\REV3~1.05\EEProm.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\EEProm.c
mco.o: C:\iccv712\include\string.h C:\iccv712\include\_const.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h .\..\REV3~1.05\mcohw.h .\..\REV3~1.05\Interrupts.h .\..\REV3~1.05\mc9s12a128.h
mco.o:	..\REV3~1.05\mco.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\mco.c
user.o: C:\iccv712\include\string.h C:\iccv712\include\_const.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h .\..\REV3~1.05\mcohw.h .\..\REV3~1.05\mc9s12a128.h
user.o:	..\REV3~1.05\user.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\user.c
mcohw.o: .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\mcohw.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h .\..\REV3~1.05\Interrupts.h .\..\..\..\..\iccv712\include\stdarg.h
mcohw.o:	..\REV3~1.05\mcohw.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\mcohw.c
Interrupts.o: .\..\REV3~1.05\Interrupts.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h .\..\REV3~1.05\mcohw.h C:\iccv712\include\math.h .\..\REV3~1.05\vectors.h
Interrupts.o:	..\REV3~1.05\Interrupts.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\Interrupts.c
Subroutines.o: C:\iccv712\include\stdio.h C:\iccv712\include\stdarg.h C:\iccv712\include\_const.h C:\iccv712\include\string.h C:\iccv712\include\stdlib.h C:\iccv712\include\limits.h .\..\REV3~1.05\Subroutines.h .\..\REV3~1.05\Controller.h .\..\REV3~1.05\mc9s12a128.h .\..\REV3~1.05\Interrupts.h .\..\REV3~1.05\command.h .\..\REV3~1.05\mcohw.h .\..\REV3~1.05\mco.h .\..\REV3~1.05\nodecfg.h .\..\REV3~1.05\procimg.h .\..\REV3~1.05\EEProm.h
Subroutines.o:	..\REV3~1.05\Subroutines.c
	$(CC) -c $(CFLAGS) ..\REV3~1.05\Subroutines.c

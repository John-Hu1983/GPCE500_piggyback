#####################################################################
#																	 
#	Created by u'nSP IDE V4.1.1		13:01:35	11/11/26
#
#####################################################################




COMPILEDATE = %date:~0,4%%date:~5,2%%date:~8,2%

COMPILEDATE_ = %date:~0,4%-%date:~5,2%-%date:~8,2%

COMPILETIME = %time:~0,2%%time:~3,2%%time:~6,2%

PRJDIR	= D:\JohnHu\Github_private\GPCE500_piggyback\code\project

APPDIR	= C:\PROGRA~2\GENERA~1\UNSPID~2.1

USERDIR	= C:\Users\hujia\AppData\Roaming\Generalplus\unSPIDE

OUTDIR	= .\Debug

MK	= $(APPDIR)\make

CC	= "$(APPDIR)\toolchain\udocc"

AS	= "$(APPDIR)\toolchain\xasm16"

LD	= "$(APPDIR)\toolchain\xlink16_GPCE3"

LDIRAM0	= "$(APPDIR)\toolchain\xlink16_GPCE3_IRAM0"

AR	= "$(APPDIR)\toolchain\xlib16"

RESC	= "$(APPDIR)\toolchain\resc"

RM	= del	/F	1>NUL	2>NUL

STRIP	= "$(APPDIR)\toolchain\stripper"

S37STRIP	= "$(APPDIR)\toolchain\BinaryFileStripper"

RD	= rd /S /Q

GETCHKSUM	= "$(APPDIR)\toolchain\GetChecksum"

HDBPACKER	= "$(APPDIR)\toolchain\HDBPacker"

BOOTPACKER	= "$(APPDIR)\toolchain\BootPacker" 

LIKMODIFIER	= "$(APPDIR)\toolchain\unSP_LIKModifier" 

INCLUDES	= -I"D:/JohnHu/Github_private/GPCE500_piggyback/code/project" -I"C:/Program Files (x86)/Generalplus/unSPIDE_4.1.1/library/include" -I"C:/Program Files (x86)/Generalplus/unSPIDE_4.1.1/library/include/sys" -I"../Include" -I"../Library"

BODY	= -body GPCE500A -nobdy -bfile "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\GP500_base.bdy"

BODYFILE	= "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\GP500_base.bdy" 

BINFILE	= "$(OUTDIR)\GP500_base.TSK"

PRJNAME = GP500_base

BODYNAME = GPCE500A

SWTOOLID = 0x0008

IDEBUILDVER = 0x04010100

BINFILENOEXT	= $(OUTDIR)\GP500_base

ARYFILE	= "$(OUTDIR)\GP500_base.ary"

SBMFILE	= "$(OUTDIR)\GP500_base.sbm"

PKINFOFILE	= "$(BINFILENOEXT).pki"

OPT	= -S -gstabs -Wall -mnonb_data -mglobal-var-ram -mISA=2.0

ASFLAGS	= -t4 -d -sr -Wno-A2013

CASFLAGS	= -t4 -sr -Wno-A2013 -wpop

CFLAGS	= $(OPT) -B$(APPDIR)\toolchain\ $(INCLUDES) 

BINTYPE	= -at

LDFLAGS	= -Wno-L0062 -blank 0x00 -initdata -infblk "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\GP500_base.inb" -conf "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\Body\GPCE5\Checksum\GPCE5.cks"

EXTRAFLAGS	= 


OBJFILES	= \
	"$(OUTDIR)\aintworried_12K_wav_se4.res" \
	"$(OUTDIR)\GIRL_SKY_wav_se4.res" \
	"$(OUTDIR)\FileMerger_ROM_bin.res" \
	"$(OUTDIR)\Resource.obj" \
	"$(OUTDIR)\BootCode_User.obj" \
	"$(OUTDIR)\SPI_Flash.obj" \
	"$(OUTDIR)\main.obj" \
	"$(OUTDIR)\isr.obj" \
	"$(OUTDIR)\system.obj" \
	"$(OUTDIR)\gpce.obj" \
	"$(OUTDIR)\audio_spu.obj" \
	"$(OUTDIR)\spu_patch.obj" \
	"$(OUTDIR)\hal_gpio.obj" \
	"$(OUTDIR)\hal_adc.obj" \
	"$(OUTDIR)\hal_iopwm.obj" \
	"$(OUTDIR)\hal_spi.obj" \
	"$(OUTDIR)\hal_uart.obj" \
	"$(OUTDIR)\hal_serial.obj" \
	"$(OUTDIR)\hal_rtc.obj" \
	"$(OUTDIR)\hal_exisr.obj" \
	"$(OUTDIR)\bsp_global.obj" \
	"$(OUTDIR)\dev_button.obj" \
	"$(OUTDIR)\soft_timer.obj" \
	"$(OUTDIR)\st7735p3.obj" \
	"$(OUTDIR)\message.obj" \
	"$(OUTDIR)\schedule.obj" \
	"$(OUTDIR)\task_led.obj" 

"$(OUTDIR)\aintworried_12K_wav_se4.res": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\se4_12k\aintworried_12K_wav.se4"
	$(RESC) "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\se4_12k\aintworried_12K_wav.se4" "$(OUTDIR)\aintworried_12K_wav_se4.res" RES_AINTWORRIED_12K_WAV_SE4 

"$(OUTDIR)\GIRL_SKY_wav_se4.res": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\se4_12k\GIRL_SKY_wav.se4"
	$(RESC) "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\se4_12k\GIRL_SKY_wav.se4" "$(OUTDIR)\GIRL_SKY_wav_se4.res" RES_GIRL_SKY_WAV_SE4 

"$(OUTDIR)\FileMerger_ROM_bin.res": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\FileMergerVersion1\FileMerger_ROM.bin"
	$(RESC) "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\voice\FileMergerVersion1\FileMerger_ROM.bin" "$(OUTDIR)\FileMerger_ROM_bin.res" RES_FILEMERGER_ROM_BIN 

"$(OUTDIR)\Resource.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\Resource.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\Resource.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\Resource.asm" 

"$(OUTDIR)\BootCode_User.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\BootCode_User.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\BootCode_User.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\BootCode_User.asm" 

"$(OUTDIR)\SPI_Flash.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\SPI_Flash.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\SPI_Flash.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\project\SPI_Flash.asm" 

"$(OUTDIR)\main.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\main.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\main.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/main/main.c" 

"$(OUTDIR)\main.obj": "$(OUTDIR)\main.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\main.obj" "$(OUTDIR)\main.asm" 

"$(OUTDIR)\isr.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\isr.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\isr.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\isr.asm" 

"$(OUTDIR)\system.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\system.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\system.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\system.asm" 

"$(OUTDIR)\gpce.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\main\gpce.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\gpce.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/main/gpce.c" 

"$(OUTDIR)\gpce.obj": "$(OUTDIR)\gpce.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\gpce.obj" "$(OUTDIR)\gpce.asm" 

"$(OUTDIR)\audio_spu.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\audio_spu.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/audio/audio_spu.c" 

"$(OUTDIR)\audio_spu.obj": "$(OUTDIR)\audio_spu.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\audio_spu.obj" "$(OUTDIR)\audio_spu.asm" 

"$(OUTDIR)\spu_patch.obj": "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\spu_patch.asm" 
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\spu_patch.obj" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\spu_patch.asm" 

"$(OUTDIR)\hal_gpio.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_gpio.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_gpio.c" 

"$(OUTDIR)\hal_gpio.obj": "$(OUTDIR)\hal_gpio.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_gpio.obj" "$(OUTDIR)\hal_gpio.asm" 

"$(OUTDIR)\hal_adc.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_adc.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_adc.c" 

"$(OUTDIR)\hal_adc.obj": "$(OUTDIR)\hal_adc.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_adc.obj" "$(OUTDIR)\hal_adc.asm" 

"$(OUTDIR)\hal_iopwm.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_iopwm.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_iopwm.c" 

"$(OUTDIR)\hal_iopwm.obj": "$(OUTDIR)\hal_iopwm.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_iopwm.obj" "$(OUTDIR)\hal_iopwm.asm" 

"$(OUTDIR)\hal_spi.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_spi.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_spi.c" 

"$(OUTDIR)\hal_spi.obj": "$(OUTDIR)\hal_spi.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_spi.obj" "$(OUTDIR)\hal_spi.asm" 

"$(OUTDIR)\hal_uart.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_uart.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_uart.c" 

"$(OUTDIR)\hal_uart.obj": "$(OUTDIR)\hal_uart.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_uart.obj" "$(OUTDIR)\hal_uart.asm" 

"$(OUTDIR)\hal_serial.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_serial.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_serial.c" 

"$(OUTDIR)\hal_serial.obj": "$(OUTDIR)\hal_serial.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_serial.obj" "$(OUTDIR)\hal_serial.asm" 

"$(OUTDIR)\hal_rtc.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_rtc.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_rtc.c" 

"$(OUTDIR)\hal_rtc.obj": "$(OUTDIR)\hal_rtc.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_rtc.obj" "$(OUTDIR)\hal_rtc.asm" 

"$(OUTDIR)\hal_exisr.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\hal_exisr.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/hal/hal_exisr.c" 

"$(OUTDIR)\hal_exisr.obj": "$(OUTDIR)\hal_exisr.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\hal_exisr.obj" "$(OUTDIR)\hal_exisr.asm" 

"$(OUTDIR)\bsp_global.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\bsp_global.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/bsp/bsp_global.c" 

"$(OUTDIR)\bsp_global.obj": "$(OUTDIR)\bsp_global.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\bsp_global.obj" "$(OUTDIR)\bsp_global.asm" 

"$(OUTDIR)\dev_button.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.c" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\dev_button.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/dev/button/dev_button.c" 

"$(OUTDIR)\dev_button.obj": "$(OUTDIR)\dev_button.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\dev_button.obj" "$(OUTDIR)\dev_button.asm" 

"$(OUTDIR)\soft_timer.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.c" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\soft_timer.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/dev/timer/soft_timer.c" 

"$(OUTDIR)\soft_timer.obj": "$(OUTDIR)\soft_timer.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\soft_timer.obj" "$(OUTDIR)\soft_timer.asm" 

"$(OUTDIR)\st7735p3.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.c" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\st7735p3.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/dev/lcd/st7735p3.c" 

"$(OUTDIR)\st7735p3.obj": "$(OUTDIR)\st7735p3.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\st7735p3.obj" "$(OUTDIR)\st7735p3.asm" 

"$(OUTDIR)\message.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\message.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/message/message.c" 

"$(OUTDIR)\message.obj": "$(OUTDIR)\message.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\message.obj" "$(OUTDIR)\message.asm" 

"$(OUTDIR)\schedule.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\schedule.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/schedule/schedule.c" 

"$(OUTDIR)\schedule.obj": "$(OUTDIR)\schedule.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\schedule.obj" "$(OUTDIR)\schedule.asm" 

"$(OUTDIR)\task_led.asm": "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.c" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\spinmaster.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\System.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\SPI_Flash.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_config.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\bsp\bsp_global.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_gpio.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\Include\user_define.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_adc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_iopwm.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_rtc.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_exisr.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_serial.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_spi.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\hal\hal_uart.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\audio\audio_spu.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\timer\soft_timer.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\message\message.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\button\dev_button.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\dev\lcd\st7735p3.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\schedule\schedule.h" "D:\JohnHu\Github_private\GPCE500_piggyback\code\task\task_led.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stddef.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\string.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdio.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\_ansi.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdlib.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\sys\reent.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine/_types.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\assert.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\limits.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\stdarg.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\math.h" "C:\Program Files (x86)\Generalplus\unSPIDE_4.1.1\library\include\machine\ieeefp.h" "GPCE36_CE5.h" "Resource.h" "voice/FileMergerVersion1/FileMerger.h" 
	set PATH="$(APPDIR)\toolchain\";%PATH% & \
	$(CC) $(CFLAGS) -o "$(OUTDIR)\task_led.asm" "D:/JohnHu/Github_private/GPCE500_piggyback/code/task/task_led.c" 

"$(OUTDIR)\task_led.obj": "$(OUTDIR)\task_led.asm"
	$(AS) $(CASFLAGS) $(INCLUDES) -o "$(OUTDIR)\task_led.obj" "$(OUTDIR)\task_led.asm" 


GEN_BOOTPACKER_INFO :
	@echo TagAlgorithm=0x0001 >> "$(OUTDIR)\BootpackerIni.ini"
	@echo BodyName="$(BODYNAME)" >> "$(OUTDIR)\BootpackerIni.ini"
	@echo SWToolID=$(SWTOOLID) >> "$(OUTDIR)\BootpackerIni.ini"
	@echo IDEVersion=$(IDEBUILDVER) >> "$(OUTDIR)\BootpackerIni.ini"
	@echo BuildDate="$(COMPILEDATE)" >> "$(OUTDIR)\BootpackerIni.ini"
	@echo BuildTime="$(COMPILETIME)" >> "$(OUTDIR)\BootpackerIni.ini"

GEN_HDB :
	@echo [Config] > $(PKINFOFILE)
	@echo IDEVersion=$(IDEBUILDVER) >> $(PKINFOFILE)
	@echo SWToolID=$(SWTOOLID) >> $(PKINFOFILE)
	@echo BodyName=$(BODYNAME) >> $(PKINFOFILE)
	@echo TSKFile=$(BINFILE) >> $(PKINFOFILE)
	@echo LODFile="$(BINFILENOEXT).lod" >> $(PKINFOFILE)
	@if exist "$(BINFILENOEXT)_SPI.bin" @echo SPIFile="$(BINFILENOEXT)_SPI.bin" >> $(PKINFOFILE)
	@echo HDBFile="$(BINFILENOEXT).hdb" >> $(PKINFOFILE)
	$(HDBPACKER) -p -conf $(PKINFOFILE)
	@if exist $(BINFILE) del $(BINFILE)

.SUFFIXES : .c .asm .obj .s37 .tsk .res

all :	 BEFOREBUILD "$(OUTDIR)" GEN_BOOTPACKER_INFO $(BINFILE) GEN_HDB

BEFOREBUILD :

"$(OUTDIR)" :
	@if not exist "$(OUTDIR)\$(NULL)" mkdir "$(OUTDIR)"

$(BINFILE) : $(OBJFILES) 
	$(LD) $(BINTYPE) $(ARYFILE) "$(BINFILENOEXT)_SPI.bin" -tskMaxUsed $(LDFLAGS) $(BODY) $(EXTRAFLAGS) -exportvalues "$(OUTDIR)\BootpackerIni.ini" -bootrominfo "BootCode_SEC;0100;BootCode_SEC_EX" -undefined-opt __TgP190708CM -undefined-opt __TgP190708CL -undefined-opt __TgP190708M
	@copy "$(BINFILENOEXT)_SPI.bin" $(BINFILE) /A /Y > NUL
	$(BOOTPACKER) "$(OUTDIR)\BootpackerIni.ini"
	@if exist "$(BINFILENOEXT)_SPI.bin" del "$(BINFILENOEXT)_SPI.bin"
	@copy "$(BINFILENOEXT)_SPI.bintmp" "$(BINFILENOEXT)_SPI.bin" /A /Y > NUL
	@if exist "$(BINFILENOEXT)_SPI.bintmp" del "$(BINFILENOEXT)_SPI.bintmp"
	$(STRIP) "$(BINFILENOEXT)_SPI.bin" "$(BINFILENOEXT)_SPI.bin" 0x10000 0 1 

compile :	 $(OBJFILES)

CLEANFILES = \
	"$(OUTDIR)\aintworried_12K_wav_se4.res" \
	"$(OUTDIR)\GIRL_SKY_wav_se4.res" \
	"$(OUTDIR)\FileMerger_ROM_bin.res" \
	"$(OUTDIR)\Resource.obj" \
	"$(OUTDIR)\Resource.lst" \
	"$(OUTDIR)\BootCode_User.obj" \
	"$(OUTDIR)\BootCode_User.lst" \
	"$(OUTDIR)\SPI_Flash.obj" \
	"$(OUTDIR)\SPI_Flash.lst" \
	"$(OUTDIR)\main.obj" \
	"$(OUTDIR)\main.lst" \
	"$(OUTDIR)\main.asm" \
	"$(OUTDIR)\isr.obj" \
	"$(OUTDIR)\isr.lst" \
	"$(OUTDIR)\system.obj" \
	"$(OUTDIR)\system.lst" \
	"$(OUTDIR)\gpce.obj" \
	"$(OUTDIR)\gpce.lst" \
	"$(OUTDIR)\gpce.asm" \
	"$(OUTDIR)\audio_spu.obj" \
	"$(OUTDIR)\audio_spu.lst" \
	"$(OUTDIR)\audio_spu.asm" \
	"$(OUTDIR)\spu_patch.obj" \
	"$(OUTDIR)\spu_patch.lst" \
	"$(OUTDIR)\hal_gpio.obj" \
	"$(OUTDIR)\hal_gpio.lst" \
	"$(OUTDIR)\hal_gpio.asm" \
	"$(OUTDIR)\hal_adc.obj" \
	"$(OUTDIR)\hal_adc.lst" \
	"$(OUTDIR)\hal_adc.asm" \
	"$(OUTDIR)\hal_iopwm.obj" \
	"$(OUTDIR)\hal_iopwm.lst" \
	"$(OUTDIR)\hal_iopwm.asm" \
	"$(OUTDIR)\hal_spi.obj" \
	"$(OUTDIR)\hal_spi.lst" \
	"$(OUTDIR)\hal_spi.asm" \
	"$(OUTDIR)\hal_uart.obj" \
	"$(OUTDIR)\hal_uart.lst" \
	"$(OUTDIR)\hal_uart.asm" \
	"$(OUTDIR)\hal_serial.obj" \
	"$(OUTDIR)\hal_serial.lst" \
	"$(OUTDIR)\hal_serial.asm" \
	"$(OUTDIR)\hal_rtc.obj" \
	"$(OUTDIR)\hal_rtc.lst" \
	"$(OUTDIR)\hal_rtc.asm" \
	"$(OUTDIR)\hal_exisr.obj" \
	"$(OUTDIR)\hal_exisr.lst" \
	"$(OUTDIR)\hal_exisr.asm" \
	"$(OUTDIR)\bsp_global.obj" \
	"$(OUTDIR)\bsp_global.lst" \
	"$(OUTDIR)\bsp_global.asm" \
	"$(OUTDIR)\dev_button.obj" \
	"$(OUTDIR)\dev_button.lst" \
	"$(OUTDIR)\dev_button.asm" \
	"$(OUTDIR)\soft_timer.obj" \
	"$(OUTDIR)\soft_timer.lst" \
	"$(OUTDIR)\soft_timer.asm" \
	"$(OUTDIR)\st7735p3.obj" \
	"$(OUTDIR)\st7735p3.lst" \
	"$(OUTDIR)\st7735p3.asm" \
	"$(OUTDIR)\message.obj" \
	"$(OUTDIR)\message.lst" \
	"$(OUTDIR)\message.asm" \
	"$(OUTDIR)\schedule.obj" \
	"$(OUTDIR)\schedule.lst" \
	"$(OUTDIR)\schedule.asm" \
	"$(OUTDIR)\task_led.obj" \
	"$(OUTDIR)\task_led.lst" \
	"$(OUTDIR)\task_led.asm" \
	"$(BINFILENOEXT).s37" \
	"$(BINFILENOEXT).tsk" \
	"$(BINFILENOEXT)_SPI.bin*" \
	"$(BINFILENOEXT).hdb" \
	"$(BINFILENOEXT).lod" \
	"$(BINFILENOEXT).map" \
	"$(BINFILENOEXT).sbm" \
	"$(BINFILENOEXT).sym" \
	"$(BINFILENOEXT).smy" \
	$(PKINFOFILE) \
	$(SBMFILE)

clean :
	$(RM) $(wordlist 1,81,$(CLEANFILES))

.c.asm:
	$(CC) $(CFLAGS) $(INCLUDES) -o "$(OUTDIR)\$@" $<

.asm.obj:
	$(AS) $(ASFLAGS) $(INCLUDES) -o "$(OUTDIR)\$@" $<


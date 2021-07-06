
#---------------------------------------------------
#
# ��������� make ���� ��� 
# mingw32-make.exe -C Objects -f..\makefile
# �.�. ������� ��������� ������ ���� Objects!
#---------------------------------------------------

source_files=delay.cpp main_puep.cpp adc_puep.cpp i2c.cpp LCD_i2c.cpp sprintf.cpp system_stm32f0xx.c startup_stm32f030x6.s

# ����� ��������� Keil
KEIL=D:\Keil_v5\ARM\ARMCC\bin
# ������� �������� �������� ������
OUTDIR:=Objects\
# C-compiler, link, as
CC:=$(KEIL)\armcc.exe
LNK:=$(KEIL)\armlink
AS:=$(KEIL)\armasm.exe

# ��� ������ �������� make �������� ������ � ������ ( �� �������!) ���������
# � ������ ������ - ��� ����� � ������������ (������������ "Objects") ��������
VPATH = ..  

# ��� ����� ����� � ����� armcc.cfg, ������� ������� ���� �� �������� ���� armcc.cfg (����)
CCFLAG= --Via armcc.cfg
TARGET=heatc
# ������ ���� �������� ������

# ����� ��� ��������� �� �������
object_files:=$(patsubst   %.cpp,%.o,$(source_files)) 
object_files:=$(patsubst   %.c,%.o,  $(object_files)) 
object_files:=$(patsubst   %.s,%.o,  $(object_files)) 

# %.o: %.cpp ����� ������ ������� ������ - ��� �������� � �� ���
# $(notdir $(basename $^)).o  �������� ���������� � <notdir> ��������� ��� ../.\ 
# ������ armcc.exe ������������, ����� ������� ������� Objects!
%.o: %.cpp
	$(CC) $(CCFLAG) $^ -o $(notdir $(basename $^)).o

# ������-�� ����� ������ ������� ������ �������� � ����� ������������ �� �����, ��������� ������
.c.o:
	$(CC) --Via armcc.cfg $^ -o $(notdir $(basename $^)).o

.s.o: 
	$(AS) --cpu Cortex-M0 --pd "__EVAL SETA 1" -g --apcs=interwork $^  -o $(notdir $(basename $^)).o

# �������� ������� - ��� �������� 

$(TARGET).axf: armcc.cfg f0.sct $(object_files) 
	@del $(TARGET).axf 2> nul
	$(LNK) --scatter f0.sct --feedback=unused --cpu Cortex-M0 --strict $(object_files) -o $(TARGET).axf

# ��� �������� f0.sct
f0.sct: 
	echo LR_IROM1 0x08000000 0x00004000{ER_IROM1 0x08000000 0x00004000{*.o (RESET, +First) .ANY(+RO) .ANY(+XO)}RW_IRAM1 0x20000000 0x00001000{.ANY (+RW +ZI)}} > f0.sct

# ��� �������� armcc.cfg
armcc.cfg:
	echo  --cpp -c --cpu Cortex-M0 -D__EVAL -g -O0 --apcs=interwork --split_sections --feedback=unused  --diag_style=gnu >armcc.cfg
	echo  -Id:\Keil_v5\packs\ARM\CMSIS\5.7.0\CMSIS\Core\Include>>armcc.cfg
	echo  -Id:\Keil_v5\packs\Keil\STM32F0xx_DFP\2.1.0\Drivers\CMSIS\Device\ST\STM32F0xx\Include>>armcc.cfg

# ���� - ������� (�����������) ���������
clean:
	@del /S /Q *.o,*.lnp,*.cfg,1,*.bak,*.axf,*.lst,*.map


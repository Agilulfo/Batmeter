

include ./compile.mk

# Directories
MPC30 = $(XC16DIR)
PK2 = $(PK2DIR)/pk2cmd

BIN = $(MPC30)/bin
LIB = $(MPC30)/lib
H = $(MPC30)/h


# Commands
AS = $(BIN)/../mpasm/MPASMWIN
CC = $(BIN)/xc16-gcc
LD = $(BIN)/xc16-ld
BIN2HEX = $(BIN)/xc16-bin2hex
AR = $(BIN)/mplib
DIS = $(BIN)/pic30-objdump
HEX = $(BIN)/mp2hex
PK2CMD = $(PK2)/pk2cmd
RM = rm

DIR = build

DEVICE=dsPIC33FJ128MC802
P_DEVICE=33FJ128MC802

# Project files
OBJECTFILES=$(DIR)/main.o  $(DIR)/serial_driver.o $(DIR)/clocking.o  \
      $(DIR)/queue.o \
#      $(DIR)/adc.o $(DIR)/i2c_communication.o 

#CAN_DIR=../ECANLib/build
#CAN_OBJECTFILES=$(CAN_DIR)/ecan_driver.o $(CAN_DIR)/ecan_config.o $(CAN_DIR)/ecan_lib.o

TARGET_FILE=microbrain

DEFINES=  #-DUSE_ECAN

COF_FILE=$(TARGET_FILE).cof
MAP_FILE=$(TARGET_FILE).map
HEX_FILE=$(TARGET_FILE).hex
LIST_FILE=$(TARGET_FILE).lst
OTHERSFILES=$(DIR)/$(MAP_FILE) $(DIR)/$(COF_FILE) $(DIR)/$(HEX_FILE)


all: $(DIR)/$(HEX_FILE)

# build
$(DIR)/$(HEX_FILE): $(OBJECTFILES)
	@# from mplab's Makefile
	@#echo "=> Running $@... Configuration=$(CONF)"
	@echo "======> Linking"
	@$(LD) --processor $(P_DEVICE) --script $(MPC30)/support/dsPIC33F/gld/p$(P_DEVICE).gld --report-mem -o$(DIR)/$(COF_FILE) ${OBJECTFILES} --heap=512 -L$(LIB) -L$(LIB)/dsPIC33F -Map=$(DIR)/$(MAP_FILE)  -l c -l pic30 -l p33FJ128MC802 -l dsp -l fastm -l q-dsp -l q -l sol -l m
	@$(BIN2HEX) $(DIR)/$(COF_FILE) $(DIR)/$(HEX_FILE)


#  --start-group -lpic30 --end-group
# -l lega-c -l lega-pic30

# from mplab's Makefile
build/%.o: %.c
	@echo "======> Compiling $*.c"
	@$(CC) -Wall -mcpu=$(P_DEVICE) $(DEFINES) -I ../ECANLib -c `basename $*.c` -o $@

#$(OBJECTFILES) : $(SOURCEFILES) $(INCLUDEFILES)
#	@echo "======> Compiling $*.c"
#	@$(CC) -Wall -mcpu=$(P_DEVICE) -c `basename $*.c` -o $@

program: all
	sudo $(PK2CMD) -B$(PK2) -P$(DEVICE) -F$(DIR)/$(HEX_FILE) -M -R

dis:
	@(cd $(DIR); $(DIS) -D $(COF_FILE))

check:
	sudo $(PK2CMD) -B$(PK2) -P -I

# clean
clean: #.clean-post
	@# from mplab's Makefile
	@echo "======> Cleaning ------> $(OTHERSFILES)"
	@-$(RM) $(OBJECTFILES) $(OTHERSFILES)

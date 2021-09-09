##########################################################################################################################
# File automatically-generated by tool: [projectgenerator] version: [3.14.1] date: [Sat Sep 04 11:51:44 CST 2021] 
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
BSTARGET = BootStub
TARGET = CanFilter


######################################
# building variables
######################################
# debug build?
#DEBUG = 1
DEBUG = 0
# optimization
ifeq ($(EON),)
OPT = -Og
else
OPT = -Os
endif


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
#Core/Src/stm32f1xx_it.c \

COMM_C_SOURCES =  \
Core/Src/stm32f1xx_hal_msp.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_tim.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.c \
Core/Src/system_stm32f1xx.c \
Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c

C_SOURCES = Core/Src/main.c

BS_C_SOURCES = Core/Src/bootstub.c \
Core/Src/rsa.c \
Core/Src/sha.c 

# ASM sources
ASM_SOURCES =  \
startup_stm32f105xc.s

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m3 -mlittle-endian 

# fpu
# NONE for Cortex-M0/M0+/M3

# float-abi


# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_FULL_LL_DRIVER \
-DUSE_HAL_DRIVER \
-DSTM32F105xC


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-ICore/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc \
-IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F1xx/Include \
-IDrivers/CMSIS/Include \
-ICore/Inc/crypto \
-I.

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -std=gnu11 -nostdlib -fno-builtin
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -Wextra -std=gnu11 -nostdlib -fno-builtin

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F105RCTx_FLASH.ld

# libraries
ifeq ($(EON),)
LIBS = -lc -lm -lnosys 
endif

LIBDIR = 

ifeq ($(EON),)
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections
BSLDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(BSTARGET).map,--cref -Wl,--gc-sections
else
LDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections  -std=gnu11 -nostdlib -fno-builtin
BSLDFLAGS = $(MCU) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(BSTARGET).map,--cref -Wl,--gc-sections  -std=gnu11 -nostdlib -fno-builtin
endif

# default action: build all
all: $(BUILD_DIR)/$(BSTARGET).elf $(BUILD_DIR)/$(BSTARGET).hex $(BUILD_DIR)/$(BSTARGET).bin \
	$(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin \
	$(BUILD_DIR)/$(TARGET).bin.signed


#######################################
# build the application
#######################################
# list of objects

COMMOBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(COMM_C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(COMM_C_SOURCES)))

## list of ASM program objects
COMMOBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

BSOBJECTS = $(BUILD_DIR)/bootstub.o \
$(BUILD_DIR)/rsa.o \
$(BUILD_DIR)/sha.o \
$(COMMOBJECTS) 

OBJECTS = $(BUILD_DIR)/main.o $(COMMOBJECTS) 

$(BUILD_DIR)/gitversion.h:
	./git-ver.py $(BUILD_DIR)/gitversion.h

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) $(BUILD_DIR)/gitversion.h
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
ifeq ($(EON),)
	$(AS) -c $(CFLAGS) $< -o $@
else
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.s=.lst)) $< -o $@
endif

$(BUILD_DIR)/$(BSTARGET).elf: $(BSOBJECTS) Makefile
	$(CC) $(BSOBJECTS) $(BSLDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -Wl,--section-start,.isr_vector=0x8004000 -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	

$(BUILD_DIR)/$(TARGET).bin.signed: $(BUILD_DIR)/$(TARGET).bin
	SETLEN=1 ./sign.py $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).bin.signed ./Core/Src/certs/debug

$(BUILD_DIR):
	mkdir $@		

flash:
	st-flash write $(BUILD_DIR)/CanFilter.bin.signed 0x8004000

flash-bootloader:
	st-flash write $(BUILD_DIR)/BootStub.bin 0x8000000

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***

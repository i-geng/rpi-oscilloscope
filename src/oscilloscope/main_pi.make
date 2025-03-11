BOOTLOADER = $(CS140E_FINAL_PROJ_PATH)/bin/pi-install

# PROGS := tests/0-oscilloscope.c
PROGS := tests/0-oscilloscope-main.c

# INCLUDE +=
COMMON_SRC += multi-display.c
COMMON_SRC += ascii-font.c
# COMMON_SRC += signal_processing.c

COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/adc/adc.c
COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/adc/interrupts-asm.S 
COMMON_SRC += nrf-hw-support.c nrf-public.c nrf-driver.c

STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/kmalloc.o
STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/staff-i2c.o
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/staff-hw-spi.o

# LIB_POST += $(CS140E_2025_PATH)/lib/libm.a
# LIB_POST += $(CS140E_2025_PATH)/lib/libc.a
LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a


# define this if you need to give the device for your pi
TTYUSB = /dev/ttyUSB0
# TTYUSB = /dev/ttyUSB1

# set RUN = 1 if you want the code to automatically run after building.
RUN = 0

include $(CS140E_FINAL_PROJ_PATH)/libpi/mk/Makefile.robust
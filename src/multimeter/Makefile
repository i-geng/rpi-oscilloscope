BOOTLOADER = $(CS140E_FINAL_PROJ_PATH)/bin/pi-install

# PROGS := 0-multimeter.c
PROGS := 1-multimeter-buffer.c


INCLUDE +=
# COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/display_ig/display.c
COMMON_SRC += multi-display.c
COMMON_SRC += ascii-font.c
COMMON_SRC += signal_processing.c

COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/adc/adc.c
COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/adc/interrupts-asm.S 

STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/kmalloc.o
STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/staff-i2c.o

LIB_POST += $(CS140E_2025_PATH)/lib/libm.a
LIB_POST += $(CS140E_2025_PATH)/lib/libc.a
LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a


# define this if you need to give the device for your pi
# TTYUSB = /dev/ttyUSB1

# set RUN = 1 if you want the code to automatically run after building.
RUN = 0

include $(CS140E_FINAL_PROJ_PATH)/libpi/mk/Makefile.robust
BOOTLOADER = $(CS140E_FINAL_PROJ_PATH)/bin/pi-install

# PROGS := 1-interrupt_adc_test.c 
PROGS := 2-interrupt_adc_test.c 

COMMON_SRC += adc.c
COMMON_SRC += interrupts-asm.S 

STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/kmalloc.o
# STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/staff-faster-i2c.o
STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/staff-i2c.o


# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 1

include $(CS140E_FINAL_PROJ_PATH)/libpi/mk/Makefile.robust

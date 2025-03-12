BOOTLOADER = $(CS140E_FINAL_PROJ_PATH)/bin/pi-install

# PROGS := test_freq.c
# PROGS := 2-interrupt_adc_test.c
# PROGS := 1-multimeter-buffer.c
PROGS := 3-interrupt_multimeter_integrated.c

COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/adc/adc.c
# COMMON_SRC += $(CS140E_FINAL_PROJ_PATH)/src/multimeter/multi-display.c

COMMON_SRC += interrupts-asm.S 

# define this if you need to give the device for your pi
TTYUSB = 

# set RUN = 1 if you want the code to automatically run after building.
RUN = 0

LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a

STAFF_OBJS += $(CS140E_FINAL_PROJ_PATH)/libpi/staff-objs/kmalloc.o


include $(CS140E_FINAL_PROJ_PATH)/libpi/mk/Makefile.robust
# User Test
#------------------------------------------
TEST_C          =trap_gap8-diego2.c

# For RTOS Jenkins test, it will never finished so add a jenkins test Flag to exit().
MBED_FLAGS     +=-DJENKINS_TEST_FLAG=1

PLPBRIDGE_FLAGS +=-v 1

include $(GAP_SDK_HOME)/tools/rules/mbed_rules.mk

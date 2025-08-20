#
# Makefile for the Mstar Titania pwm test code
#

CC = mips2_fp_le-gcc

.PHONY : all clean

all :
	$(CC) -o test_pwm madp_pwm.c test_pwm.c -I../../include -I../../../../include


clean:
	@rm -f test_pwm

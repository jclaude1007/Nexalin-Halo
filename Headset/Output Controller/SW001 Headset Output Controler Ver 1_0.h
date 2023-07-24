#include <24HJ64GP202.h>
#device ICSP=1
#use delay(internal=80MHz)

#FUSES NOWDT                     //No Watch Dog Timer
#FUSES CKSFSM                    //Clock Switching is enabled, fail Safe clock monitor is enabled

#pin_select U1TX=PIN_B14

#use rs232(UART1, baud=115200, stream=DEBUG)



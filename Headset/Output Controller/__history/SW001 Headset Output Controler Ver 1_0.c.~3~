/**
Title: SW001 Headset Output Controller SW Ver 1_0

Copyright: 2022 Nexalin Technology

Author: John Claude

Function: This application controls the Nexalin Headset output controller. 

Target: PIC24HJ64GP502 running at 80MHz 

Communication: 
    Serial 115.2KB factory debug port,

Release History

12/12/2022   Intial Release, checksum 1A0D, Build 1_0
20230320    Change sync logic, 30DF
7/25/2023    Released to China

**/



#include <SW001 Headset Output Controler Ver 1_0.h>



#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//DIO Map

#define SH         PIN_B10     //DO, active low to sample
#define SYNC       PIN_B11    //DO. active high when VI data can be sampled
#define SCLK       PIN_B5     //DO, SPI clock
#define CS         PIN_A4      //DO. active low chip select
#define MOSI       PIN_B7     //DO, MOSI
#define LED        PIN_A2      //DO, active high LED


const   uint8_t    DLAY = 50;
const   uint8_t    LONG_DLAY = 220;
const   uint8_t    HIGH = 1;
const   uint8_t    LOW = 0;
static  uint8_t    g_Phase = HIGH;

//test
//test2


void main()
{
    static  uint16_t    PHase_Count;
   // setup_timer1(TMR_INTERNAL | TMR_DIV_BY_256, 16);

   // setup_adc_ports(sAN0);
   // setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_0);

  //  enable_interrupts(INT_TIMER1);
   // enable_interrupts(INTR_GLOBAL);
    delay_ms(10);
    
    fprintf(DEBUG,"\n\r Nexalin Headset PIC24HJ64GP202 Controller");

   
    
    while(TRUE)
    {
        //generate in-line coding of DAC control to attain 100KHz clock at 25% duty cycle
       switch(g_Phase)
       {
            case HIGH:  //waveform is bewteen 255 and 0
                 output_high(SYNC);  //start window for Main Controller to sample VI 
                 
                //Set DAC out = 255               
                output_high(SYNC);  //start window for Main Controller to sample VI 
                output_low(CS);  //take DAC CS low
                
                output_high(MOSI);   //rest of bits wil be 0  //all bits will be high starting with MSB
                output_high(SCLK);  // clock 1
                output_low(SCLK);

                output_high(MOSI);
                output_high(SCLK);  // clock 2
                output_low(SCLK);               
             
                output_high(MOSI);
                output_high(SCLK);  // clock 3
                output_low(SCLK);

                output_high(MOSI);
                output_high(SCLK);  // clock 4
                output_low(SCLK);                            
                
                output_high(MOSI);
                output_high(SCLK);  // clock 5
                output_low(SCLK);

                output_high(MOSI);
                output_high(SCLK);  // clock 6
                output_low(SCLK);               
             
                output_high(MOSI);
                output_high(SCLK);  // clock 7
                output_low(SCLK);

                output_high(MOSI);
                output_high(SCLK);  // clock 8
                output_low(SCLK);                 
                
                output_high(CS);  //latch in DAC
                delay_cycles(DLAY);
 
                //now set DAC output to 0
                output_low(CS);  //take DAC CS low
                
                output_low(MOSI);  //all bits will be low starting with MSB
                output_high(SCLK);  // clock 1
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 2
                output_low(SCLK);               
             
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 3
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 4
                output_low(SCLK);                            
                
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 5
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 6
                output_low(SCLK);               
             
                output_high(SH);
               
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 7
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 8
                output_low(SCLK);                 
                 
                output_low(SH);
                output_high(CS);  //latch in DAC
                               
                Phase_Count++;
                if(Phase_Count >=322l)
                {
                    Phase_Count = 0;
                    g_Phase = LOW;
                    output_low(SYNC);  //close window for Main Controller to sample VI 
                }
                delay_cycles(LONG_DLAY);
                
                break;
                
            case LOW:  //waveform is bewteen  127 and 
                //DAC out = 127
                output_low(CS);  //take DAC CS low

                output_high(MOSI);  //Only MSB = 1
                output_high(SCLK);  // clock 1
                output_low(SCLK);
                
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 2
                output_low(SCLK);               

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 3
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 4
                output_low(SCLK);                            
                
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 5
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 6
                output_low(SCLK);               
             
                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 7
                output_low(SCLK);

                output_low(MOSI);   //rest of bits wil be 0
                output_high(SCLK);  // clock 8
                output_low(SCLK);                 
                
                output_high(CS);  //latch in DAC
                delay_cycles(DLAY);
 
                //now set DAC output to 43 (0010 1011)
                output_low(CS);  //take DAC CS low
                
                output_low(MOSI);  //SPI out
                output_high(SCLK);  // clock 1
                output_low(SCLK);

                output_low(MOSI);  //SPI out
                output_high(SCLK);  // clock 2
                output_low(SCLK);               
             
                output_high(MOSI);  //SPI out
                output_high(SCLK);  // clock 3
                output_low(SCLK);

                output_low(MOSI);  //SPI out
                output_high(SCLK);  // clock 4
                output_low(SCLK);                            

                output_high(MOSI);  //SPI out
                output_high(SCLK);  // clock 5
                output_low(SCLK);

                output_low(MOSI);  //SPI out
                output_high(SCLK);  // clock 6
                output_low(SCLK);               
             
                output_high(MOSI);  //SPI out              
                output_high(SCLK);  // clock 7
                output_low(SCLK);

                output_high(MOSI);  //SPI out
                output_high(SCLK);  // clock 8
                output_low(SCLK);                 
 
                output_high(CS);  //latch in DAC
                
                Phase_Count++;
                if(Phase_Count >=968ll)
                {
                    Phase_Count = 0;
                    g_Phase = HIGH;
                }
                delay_cycles(LONG_DLAY);
                break;
               
       }
                
    }
}

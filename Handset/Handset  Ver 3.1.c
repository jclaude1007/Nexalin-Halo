/////////////////////////////////////////////////////////////////////
//  Example program to use the custom RN487x driver 
//  Mark S. Ver 2.0 20230708
/////////////////////////////////////////////////////////////////////

#include <18f47K42.h>
#device ADC=10
#device pass_strings=in_ram

#use delay(internal=32mhz)


#pin_select U1TX=PIN_C6  // for RN487x
#pin_select U1RX=PIN_C7
#pin_select U2TX=PIN_B3  // for 18F47K42
#pin_select U2RX=PIN_B4
#use rs232(baud=115200,parity=N,xmit=PIN_B3,rcv=PIN_B4,bits=8,stream=DEBUG)
#pin_select CCP1OUT=PIN_C2      //battery charger PWM

//#pin_select U1TX=PIN_C6  // for 18f47K42
//#pin_select U1RX=PIN_C7
//#use rs232(ICD, baud=115200, stream=DEBUG, disable_ints)

#define BUTTON1 PIN_B0
#define BUTTON2 PIN_B1
#define CHRGR_ON    PIN_A2      //DI,SET WHEN CHARGER ATTACHED
#define RST_BLE PIN_B5
#define LED1    PIN_A6
#define ENB_VCC PIN_A5

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define BT_MODEL_NAME "BLE-Central"
#define BT_REMOTE_NAME "Halo"
#include "rn487x.c"


typedef enum    //ADC Channels
{
    BATT_I = 0,       //battery curent
    BATT_V,           //  battery voltage
    CHARGER,       //charger voltage
}  ADC_ChannelType;


typedef enum    //g_Flags foir internal use only
{
    TMR1 = 0,   //flag register bit set when timer1 interrupt
    SECOND,   //flag register bit set when 1 second has passed
    FACTORY,  //flag register bit set when factory programmer attached
    CHGR_ON ,    //set when charger is attached
    BUTTON_FLG,     //set when a button2 press detected
  //  INT_FLG,
}  SystemFlagType;


typedef enum    //battery charging states
{
    INIT_CHARGE = 0,  //initialize chargeer
    CC,     //constant current
    CV,     //constant voltage
    FULL,   //full charge
}   ChargeStageType;


typedef enum    //machine states while device is AWAKE
{
    POST = 0,
    CHARGING,
    TREAT,      //treatment
    POOR_CONTACT,   //poor electrode contact
    FAULT,  //fault condition
}  MachineStateType;

typedef enum    //electrode c ontact flags for g_Contact_Status
{
    LEFT_FLG = 0,   //left mastoid, set when good
    FORE_FLG,       // forehead, set when good
    RIGHT_FLG,      //right mastoid, set when good
}  ContactType;

// Battery charge constants C = 420 mAH
int16_t const PWM_MAX = 412;    //this disables charging
int16_t const PWM_MIN = 30; //this is maximum charging voltage
int16_t const FULL_BATT = 410;
int16_t const END_AMPS = 100;    //C/4 mA)
uint16_t const CC_AMPS = 220l;   // C/2 mA
uint32_t const MAX_TIME = 7200l;    //2 hours in seconds to timeout at at contant voltage mode
uint32_t const CC_TIME = 7200l;     //2 hours in seconds to timeout at at contant current mode

//uint8_t const TREATTIME = 40;   //treatment minutes
//uint16_t const PGRM_MEM = 0x7CFF;   //PIC18F45K22 program memory size 32K
uint8_t const  HEAD = 0;
uint8_t const  HAND = 1;
uint8_t const  ON = 1;
uint8_t const  OFF = 0;

/**Globals **/
//uint8_t     g_State = POST;
uint8_t     g_Flags;
uint8_t     g_Paired = FALSE;  //not paired 
uint8_t     g_BLE_Logo = 0;  //BLE logo blanked
uint8_t     g_Contact_Status = 0;   //all electrode poor contact
uint8_t     g_mode = OFF;
//uint8_t     g_Setpoint = 4; //initial headset output current in mA

uint16_t    g_TreatMinutes = 40;
uint16_t    g_TreatSeconds = 0;
uint16_t    g_Seconds = 2400;   //60 seconds x 40 minutes

uint16_t    g_Batt_Volts = 0;     //handset battery voltage
uint16_t    g_Batt_Amps = 0;     //handset battery charge current
uint16_t    g_Head_Batt_Volts = 0;     //headset  battery voltage
uint16_t    g_NoActivity;   //activity counter, turn off device if no activity
uint8_t     g_BLE_Activity = 0; //BLE connection counter
//char g_command;
uint8_t g_Head_Batt_Level;
char resp[32];

#include <Handset Main HaloLCD.c>

//Prototypes
//**********************************************************
void    TIMER1_isr(void);       //system heartbeat 10msec
void    Init_IO(void);
void    Self_Test(void);
void    Check_Buttons(void);     //check buttons
void    Clocks(void);   //update clocks
void    Charge_Battery(void);   //charge battery
void    Tasks(void);
void    Main(void);
void    UpdateDisplayTime(void);
void    Check_BLE(void); 
void    Update_Handset_Battery_Gauge(void);
void    Update_Headset_Battery_Gauge(void);
void    Update_Handset_Battery_VI(void);
void    Process_Command(void);

//*************************************Code **********************************


/******************************************************************************
*
* FUNCTION     : Update_Head_Battery(void)
* INPUTS       : Head battery voltage
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Convert battery voltage to levels and display 
/*****************************************************************************/

void    Update_Headset_Battery_Gauge(void)
{
           
   if( (g_Head_Batt_Volts > 380l) && (g_Head_Batt_Level !=100) )
   {
        g_Head_Batt_Level = 100;
        Draw_Full_Battery(HEAD);
   }
   else if( (g_Head_Batt_Volts<= 380l) && (g_Head_Batt_Volts > 370l) && (g_Head_Batt_Level !=80) )
   {
        g_Head_Batt_Level = 80;
        Draw_Three_Quart_Battery(HEAD);
   }
   else if( (g_Head_Batt_Volts <= 370l) && (g_Head_Batt_Volts > 360l) && (g_Head_Batt_Level !=60) )
   {
        g_Head_Batt_Level = 60;
        Draw_Half_Battery(HEAD);
    }      
    else if( (g_Head_Batt_Volts <= 360l) && (g_Head_Batt_Volts > 350l) && (g_Head_Batt_Level !=40) )
   {
       g_Head_Batt_Level = 40;
        Draw_Quart_Battery(HEAD);
   }       
    else if( (g_Head_Batt_Volts <= 350l) && (g_Head_Batt_Volts > 340l) && (g_Head_Batt_Level !=20) )
   {
       g_Head_Batt_Level = 20;
        Draw_Empty_Battery(HEAD);
   }             
   else if(g_Head_Batt_Volts < 340l)  //if dead battert 
    {
       Draw_Empty_Battery(HEAD);
    }
 // fprintf(DEBUG,"\n\r %lu,Level=%u", g_Head_Batt_Volts, g_Head_Batt_Level);

 }   



/******************************************************************************
*
* FUNCTION     : Update_Handset_Battery(void)
* INPUTS       : Handset battery voltage
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Convert battery voltage to levels and display 
/*****************************************************************************/

void    Update_Handset_Battery_Gauge(void)
{
  static    uint8_t   Batt_Level = 0;

   if( (g_Batt_Volts > 380) && (Batt_Level !=100) )   
   {
        Batt_Level = 100;
        Draw_Full_Battery(HAND);
   }
   else if( (g_Batt_Volts <= 380l) && (g_Batt_Volts > 370l) && (Batt_Level !=80) )
   {
        Batt_Level = 80;
        Draw_Three_Quart_Battery(HAND);
   }
   else if( (g_Batt_Volts <= 370l) && (g_Batt_Volts > 360l) && (Batt_Level !=60) )
   {
        Batt_Level = 60;
        Draw_Half_Battery(HAND);
   }      
    else if( (g_Batt_Volts <= 360l) && (g_Batt_Volts > 350l) && (Batt_Level !=40) )
   {
        Batt_Level = 40;
        Draw_Quart_Battery(HAND);
   }       
    else if( (g_Batt_Volts <= 350l) && (g_Batt_Volts > 340l) && (Batt_Level !=20) )
   {
        Batt_Level = 20;
        Draw_Empty_Battery(HAND);
   }             
   else if(g_Batt_Volts < 340l)  //if dead battert 
   {
        Draw_Empty_Battery(HAND);
    }
    
    //fprintf(DEBUG,"\n\r %lu, %u", g_Batt_Volts, Batt_Level);
  
 }   


/******************************************************************************
*
* FUNCTION     : Update_Handset_Battery(void)
* INPUTS       : Handset battery voltage
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Convert battery voltage to levels and display 
/*****************************************************************************/

/**
    Measure handset battery voltage and current. Update handset battery icon
    Battery voltage is scaled down by 2 across BT link
**/
void    Update_Handset_Battery_VI(void)
{
    float ftemp;
       
    set_adc_channel(BATT_V);  //set ADC channel
    delay_us(100);
    ftemp= (float)read_adc();
    g_Batt_Volts = (long)(ftemp * 0.59) ;   //100x battery voltage, 0.00293V/lsb * 2(scale) * 100 = 0.586

 
    set_adc_channel(BATT_I);  //set ADC channel
    delay_us(100);
    ftemp= (float)read_adc();
    g_Batt_Amps = (long)(ftemp * 2.93);   //0.00293/lsb x 1000mA/V= 2.93

   // fprintf(DEBUG,"\n\r g_BatI=%lu, batV = %lu", g_Batt_Amps, g_Batt_Volts);
 }


/**
This routine reads the state of the ON/OFF button and the Start and Stop button
The button signal is low when the button is pushed. Pressing the button while
the device is on will turn the device off after reamping down the pressure
**/
void  Check_Buttons(void)
{
    uint8_t  ButtonLevel2 ;   
    static uint8_t   LastButton2=1; 

    // Now check the start /stop button
    ButtonLevel2 =   input(BUTTON2);  //get current button level, low if pressed
 
    if((ButtonLevel2) && (!LastButton2))   //while button is pushed
     {
        bit_set(g_FLags,BUTTON_FLG);
     }
    LastButton2 =  ButtonLevel2 ;
}

/**

**/
void    Read_Button(void)
{
      bit_clear(g_Flags, BUTTON_FLG);
      
      if(g_Mode == ON)
       {
            g_Mode = OFF;
            fprintf(DEBUG,"\n\r OFF");
       }
       else
       {
            g_Mode = ON;
            fprintf(DEBUG,"\n\r ON");
       }
}





 /**
    Generate 10ms interupt for system heartbeatt
 **/
#INT_TIMER1
void  TIMER1_isr(void) 
{
    static int8 count;
    
   // output_high(Pin_A4);
  //  delay_us(5);
  //output_low(Pin_A4);
   
    set_timer1(55550);      //10msec interrupt
    bit_set(g_Flags,TMR1);
 }   

#INT_EXT
void  EXT_isr(void) 
{
    //clear_interrupt(int_EXT); 
    // bit_set(g_Flags,INT_FLG); 
     fprintf(DEBUG,"\n\r Sleep now");
     output_low(ENB_VCC); //disable power
     while(1) {}  //wait till dead
}



/**
void Check_BLE(void)

Called once a second from Tasks()
**/
void Check_BLE(void)
{
    if(g_Head_Batt_Volts == 0)    //if no battery voltage =  not paired
    {
         g_BLE_Logo = 0; //then turn off
    }
    else
    {
        g_BLE_Logo = 1;    
    }
    Draw_BLE(); //update BLE logo
}



/**
Battery charger function 
    Battery PWM range is 100 to 240. Reducing PWM value increase charge voltage
    Start with constant curent then constant voltage till current level reaches minimum
**/
void    Charge_Battery(void)
{
    static uint16_t Batt_PWM = PWM_MAX;
    static uint16_t    Amps_Offset;
    static uint16_t  Charge_Time;
    static uint8_t   Charge_State = INIT_CHARGE;
    static uint8_t   Bars;

    if(!Input(CHRGR_ON))  //if charger is not attached
    {
        Charge_State = INIT_CHARGE; //reset charge state
        set_pwm1_duty(Batt_PWM);   //turn off charger PWM
        Bars = 0;
    }
    else    //charger is attached
    {
       // fprintf(DEBUG,"\n\r %u, %lu , %lu, %lu, ", Charge_State, Batt_PWM,  g_Batt_Volts, g_Batt_Amps);
       // g_NoActivity = 0;   //clear counter to prevent no activity shutdown
                      
        switch (Charge_State)
        {
            case INIT_CHARGE:
                Batt_PWM = PWM_MAX; //initialize. 
                Charge_Time = 0;
                Charge_State = CC;
                Bars = 0;
                Amps_Offset = 60;   //g_Batt_Amps;  //get offset amps level
               // fprintf(DEBUG,"\n\r %lu,", Amps_Offset);
                break;
                
            case CC:    //constant current mode
                
                if(g_Batt_Amps >= Amps_Offset)  //correct current for offset
                    g_Batt_Amps =  g_Batt_Amps - Amps_Offset;
                else
                    g_Batt_Amps = 0;
  
                if(g_Batt_Amps < CC_AMPS -1L)  //&& (g_Batt_Volts <= FULL_BATT) )
                {
                    if(Batt_PWM > PWM_MIN)
                    {
                        Batt_PWM -=2;   //reduce PWM, inrease charge voltage
                    }    
                }
                else if(g_Batt_Amps > CC_AMPS +5L)
                {
                    Batt_PWM++;     //increase duty cycle, reduce charge voltage
                }
                
                //if at full voltage or time out or  at max charge voltage
                if( (g_Batt_Volts >=  FULL_BATT) || (Charge_Time > CC_TIME) || (Batt_PWM <= PWM_MIN) )  
                {
                    Charge_State = CV;    //change to CV state
                    Charge_Time = 0;      //reset safety timer
                }
                else
                {
                     Charge_Time++;    //increment timer 
                }
                break;
        
            case CV:    //constant voltage
             
                if(g_Batt_Volts >  FULL_BATT)
                {
                   Batt_PWM++;  //decrase charge voltage
                }
                
                //if at end current or at end of 2 hours
                if( (g_Batt_Amps <= END_AMPS) || (Charge_Time > MAX_TIME) || (g_Batt_Volts >=  FULL_BATT   ))  
                {
                    Charge_State = FULL;    //change state
                    fprintf(DEBUG,"\n\r Full");
                    Batt_PWM = PWM_MAX;
                } 
                Charge_Time++;    //increment timer
              
                break;
            
            case FULL:  //full tank, leave blue LED on
                Batt_PWM = PWM_MAX;
                 output_high(LED1); 
                 break;
                
            default:
                break;
        }

       set_pwm1_duty(Batt_PWM);   //update PWM output
       fprintf(DEBUG,"\n\r %u, %lu , %lu, %lu, %lu ", Charge_State, Batt_PWM,  g_Batt_Volts, g_Batt_Amps, Amps_Offset);
       
       if(Charge_State != FULL)
       {
           switch(Bars)
           {
      
            case 0: //quarter full
                Draw_Quart_Battery(HAND);
                Bars++;
                break;
    
            case 1: //half full
                Draw_Half_Battery(HAND);
                Bars++;
                break;
                
            case 2: //3/4 full    
                Draw_Three_Quart_Battery(HAND);
                Bars++;;
                break;
                
            case 3: //full
                Draw_Full_Battery(HAND);
                Bars = 0;
                break;
            
            default:
                break;
           }
       }
       else
       {
            Draw_Full_Battery(HAND);
            Bars = 0;
       }
    }
 }



/**
    Perform tasks that need to be performed once a second
**/
void    Tasks(void)
{
  if (!input(CHRGR_ON))  //if charger not attached check BLE
        Check_BLE();
    
    Update_Handset_Battery_VI();    //get handset battery VI data
    Update_Handset_Battery_Gauge();
    
    Charge_Battery();
    
   if(g_Head_Batt_Volts >0)
        Update_Headset_Battery_Gauge();
        
   if( (g_Mode == ON) && (g_Paired) ) {
       UpdateDisplayTime();
       Draw_Electrodes();
   }
   
   if(bit_test(g_Flags,BUTTON_FLG))
        Read_Button();  //check if a button2 has been detected
 }




/******************************************************************************
*
* FUNCTION     :  UpdateDisplayTime(void)
* INPUTS       : g_State, g_Seconds
* OUTPUTS      : g_TreatmentMinutes, g_TreatmentSeconds
* RETURNS      : none
* DESCRIPTION  : Calculate remaining tretment time by decrementing treatment
   seconds. Send updated times to display module
/*****************************************************************************/
       
void    UpdateDisplayTime(void)
{
    if(g_Seconds)
    {
         g_Seconds--;
         g_TreatMinutes = g_Seconds /60l;
         g_TreatSeconds = g_Seconds %60;
    }     
    
    Draw_Colon();
    DisplayTime();  //update displayed time
    
    If(!g_Seconds)  //it out of treatment time
       printf(ble_cmd_putc,"$sleep/#\r");     
    
   // fprintf(DEBUG,"\n\r %lu, %lu, %lu", g_Seconds, g_TreatMinutes, g_TreatSeconds);
 }


/**
void    Clocks(void)
   Executed every 10msec, this function updates all clocks

**/
void    Clocks(void)
{
    static uint16_t   tmr1_cntr = 0;   //counts TIMR1 interrupts
    tmr1_cntr++;    //inrement it
     
    if(!(tmr1_cntr % 100)) //if 1000msec seconds have passed
    {
        bit_set(g_Flags,SECOND);    //set new second flag
        tmr1_cntr = 0;  //reset counter
        Tasks();    //do tasks that must be done every second
    }
}


void init(void) {

   output_high(ENB_VCC);
   output_high(RST_BLE);
   output_high(LED1);
  enable_interrupts(INT_TIMER1);  //10 msec heartbeat for Check_buttons()
  enable_interrupts(INT_RDA);
 //  clear_interrupt(INT_EXT_H2L); //interrupt when power button is pressed
  // enable_interrupts(INT_EXT_H2L); //interrupt when power button is pressed
  enable_interrupts(GLOBAL);

}


void    Process_Command(void)
{
    char  term[6];
    char  *pntr;    //fixed 20170728 JPC
    uint16_t Time;

    strcpy(term,"/");
    pntr = strtok(resp, term);
    pntr = strtok(0, term);  //get next field
    g_Contact_Status = atoi(pntr); //which is electrode contact status
    pntr = strtok(0, term);  //get next field
    g_Head_Batt_Volts = atol(pntr); //which is headset battery voltage 
    pntr = strtok(0, term);  //get next field
    Time = atol(pntr); //which is headset battery voltage 
  //  fprintf(DEBUG,"\n\r %u, %lu, %lu", g_Contact_Status, g_Head_Batt_Volts,Time);
  
}



void main(void) {
 
   //char resp[32];
   int16 handle;
   int1 connected;
   connected=FALSE;

     output_high(ENB_VCC);  //allow user to remove finger
     
    setup_adc_ports(sAN0 | sAN1, NO_ANALOGS_P2, VSS_VDD);
    setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_0 | ADC_LEGACY_MODE | ADC_THRESHOLD_INT_DISABLED);
    setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);        //52.4 ms overflow
    setup_timer_2(T2_DIV_BY_1 | T2_CLK_INTERNAL,106,1);        //13.4 us overflow, 13.4 us interrupt
    setup_ccp1(CCP_PWM);
    set_pwm1_duty((int16)430);  //this value is maximum PWM duty cycle. It generates zero charge voltage
 
    init();
    Display_Setup();    //configure the display
    Display();      //run test pattern
   
    fprintf(DEBUG," Halo Handset Ver 3\r\n");

    if(input(CHRGR_ON)) 
        fprintf(DEBUG,"\n\r Charger On \n\r");
    else 
        fprintf(DEBUG,"\n\r Charger Off \n\r");
   
    while(input(CHRGR_ON)) {
       Update_Handset_Battery_VI();
       Charge_Battery();
       delay_ms(500);
    }
  
     
    if(!input(CHRGR_ON)) {

        Draw_Electrodes();
        Update_Handset_Battery_VI();
        Update_Handset_Battery_Gauge();
        Draw_Alert(ON);
        delay_ms(2000);
        Draw_Alert(OFF);
        DisplayTime();
        
    }

  // enable_interrupts(INT_TIMER1);  //10 msec heartbeat for Check_buttons()
 //  enable_interrupts(INT_RDA);
   clear_interrupt(INT_EXT_H2L); //interrupt when power button is pressed
   enable_interrupts(INT_EXT_H2L); //interrupt when power button is pressed
 //  enable_interrupts(GLOBAL);
 
 
   
   while(TRUE) {
        if(bit_test(g_Flags,TMR1))  //10 msec interrupt
        {
            bit_clear(g_Flags,TMR1);
            Check_Buttons();
            Clocks();
        }    
              
        if(bit_test(g_Flags,SECOND))  {     //do things once a econd
            bit_clear(g_Flags,SECOND);
   
            if(!connected) {    //look for BLE connection
              handle=ble_start();
              if(handle)
                 connected=TRUE;
            }
            
            fprintf(DEBUG,"\n\r j=%u \n\r", connected);
            
            if (connected) {
                g_BLE_Activity = 0; //reset counter if connected
               g_Paired = TRUE;
              
               if(g_Mode == ON)
                    printf(ble_cmd_putc,"$on/#\r");
               if(g_Mode == OFF)
                    printf(ble_cmd_putc,"$off/#\r");
  
                   
              ble_gets(resp,sizeof(resp));
              if(resp[0]!=0)
                  fprintf(DEBUG,"Remote Data=%s\r\n",resp); 
              
              Process_Command();    //process command from handset
              Tasks();  //do tasks that need to be run once a second.      
                
            }
            if(!ble_get_status_resp(resp, 0)) {
               //fprintf(DEBUG,"status=%s\r\n",resp); 
               if((resp[0]=='D')&&(resp[1]=='I'))
                  connected=FALSE;
           }  
           
        }    
    }
}


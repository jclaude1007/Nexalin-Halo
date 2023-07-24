/**
Copyright: 2022 Nexalin Technology

Author: John Claude

Function: This application controls the Nexalin Headset Main controller. 
The Main controller controls
    treatment stimuation output
    BLE peripheral function
    battery maintenance
    the output controller (waveform generation timing)


Target: 18F26K83 running at 32MHz located on the Halo headset flex circuit

Communication: 
    Serial 115.2KB factory debug port and BLE Atmosic module,
    Serial 115.2KB serial link with River Headset Output Controller.

Release History
    20221205    Initial release
    20230429    Modify to work with handset v 1.8.2
    20230503    Disable BLE function while charging
    20230506    Stand alon Headset mode ver 2.0.1
    20230510    Modfify haptic pattern, ver 2.0.2  CBE4  Sent to China
                3 beeps for mastoid,
                2 long beeps for mastoid
                10 second at end of treatment.
   20230531     Change relay timing for shock, fix haptic bug ver 2.0.5 32A4
   20230604     Change power control, change button logic, ver 2.1.0
   20230609     Change BLE controller to RN4678,  wer 3.0.1
   20230619     Add BT function for RN4678 ver 3.0.3
   20230630     Add BLE LED status, adjustable max stim current 
   
   **/

#include <18F26K83.h>
#device ADC=10
//#fuses WDT
#fuses NOWDT
#fuses NOBROWNOUT

#use delay(internal=32MHz)

#pin_select U1TX=PIN_C6
#pin_select U1RX=PIN_C7

#pin_select U2TX = PIN_B4
#pin_select U2RX = PIN_B3

#pin_select CCP1OUT=PIN_C2  // pin 10
#pin_select CCP2OUT=PIN_C1  //pin 9


#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=DEBUG)
#use rs232(baud=115200,parity=N,xmit=PIN_B4,rcv=PIN_B3,bits=8,stream=UART)

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PGD         PIN_B7     //DI, clear if programmer attached
#define ENB_VCC     PIN_C0      //DO, set to enable system VCC
//#define BUCK_PWM    PIN_C1      //DO, control battery charge voltage
//#define DAC_PWM     PIN_C2      //D0, control stimulation voltage, 
#define RESET_BT    PIN_C3      //DO, clear to reset BLE controller
#define ENB_SW_BAT  PIN_C4      //D0, clear to enable analog rails
#define STATUS      PIN_C5      //DI, cleared when BLE paired
#define BUTTON      PIN_B0      //DI, cleared when button is pushed
#define HAPTIC      PIN_B1      //DO, set to turn on haptic
#define SYNC        PIN_A6      //DI, set to trigger interrupt
#define RELAY       PIN_A4      //DO, set set to close output relay 
#define CHRGR_ON    PIN_B2      //DI, set when charger is attached
#define BLUE_LED    PIN_A7      //DO, set to turn on LED
#byte RCSTA1 =0xFAB      //UART serial port  receive control register
#byte RCSTA2 =0xF71      //DEBUG serial port receive control register

//constants
uint8_t const OERR = 1; //bit 1 set during serial port overrun
uint8_t const CREN = 4; //bit 4 , set to clear overrun
uint8_t const SPEN = 7; //bit 7 , set to clear overrun
uint8_t  const  CLEAR = 0;        //clear PID state
uint8_t  const  RUN = 1;        //run PID
float    const   FAST_RAMP = 0.02;
float    const   SLOW_RAMP = 0.002;
uint8_t const   DEFAULT_MA = 10;    //default mA setpoint
uint8_t const   SET_ADDR = 1;   //eeprom address in controller eeprom

// Serial port communication flag constants
//const unsigned int8 COMM_IDLE =      0x00;   //!< Idle state
const unsigned int8 COMM_CMD  =      0x01;   //!< Looking for command
const unsigned int8 COMM_FND_CR =    0x02;   //!< Looking for carriage return
const unsigned int8 COMM_FND_LF  =   0x04;   //!< Looking for line feed
const unsigned int8 COMM_FND_CMD =   0x80;   //!< Found command
const unsigned int8  BUFFER_LEN =    0xFF;     //!< length of the serial portcommand buffer   

// Atmosic NLE Communications buffers 
typedef struct    //!< Serial communication struct
    {
    char  buff[BUFFER_LEN];  //!< Buffer for received characters
    char  utf8[BUFFER_LEN];  //!< Buffer for received characters
    uint8_t  len;               //!< Length of buffer
    uint8_t  Flags; 
    }     // serial communication structure
    
structBuff; 
structBuff        g_UART;              //!< UART Buffer for Atmosic


typedef enum    //g_Fault_Num
{
    SELF_TEST_FLG,
    DEAD_BATT_FLG,
    STIM_FLG,   //set when stimulation output bad
 }  FaultFlagsType;


typedef enum    //machine states and substates
{
    POST = 0,
    WAIT,    // wait
    TREAT,      //treatment
    POOR_CONTACT,   //poor electrode contact
    GOOD_CONTACT,
    FAULT,  //fault condition
}  MachineStateType;


typedef enum    //electrode c ontact flags for g_Contact_Status
{
    LEFT_FLG = 0,   //left mastoid, set when good
    FORE_FLG,       // forehead, set when good
    RIGHT_FLG,      //right mastoid, set when good
}  ContactType;


typedef enum    //g_Flags for internal use only
{
    TMR1_FLG = 0,   //flag register bit set when timer1 interrupt
    BTPAIRED_FLG,   //flag register bit set when parried
    INT_FLG,

}  SystemFlagType;


typedef enum    //ADC Channels
{
    VSENSE = 0,       //stim voltage
    LSENSE = 1,       //stim left current
    RSENSE = 2,       //stim right current
    BATT_V = 3,
    BATT_I = 5,
      
}  ADC_ChannelType;


typedef enum    //battery charging states
{
    INIT_CHARGE = 0,  //initialize chargeer
    CC,     //constant current
    CV,     //constant voltage
    FULL,   //full charge
}   ChargeStageType;


typedef enum    //Bluetooth states
{
    UNPAIRED = 0,
    PAIRED   
}   BTStateType;


/**Globals **/
uint8_t     g_State;
uint8_t     g_Flags;
uint16_t    g_Seconds;
uint16_t    g_Batt_Volts;
uint16_t    g_Batt_Amps;
uint16_t    g_Vsense;
uint16_t    g_Lsense;
uint16_t    g_Rsense;
float       g_Total_mA;
float       g_Gain_Err;
float       g_Setpoint_mA;
float       g_MaxSetpoint;
uint16_t    g_DACVal;
uint8_t     g_Contact = 0;   //all electrode poor contact
uint8_t     g_Charge_State;
uint16_t    g_Haptic_Count;
uint8_t     g_BTState;
//uint16_t    g_Batt_Level;

// Prototypes
void    Init_Globals(void);
void    Init_IO(void);
void    Self_Test(void);
void    Clocks(void);
void    Tasks(void);
void    Charge_Battery(void);
void    Update_BatteryVI(void);
void    PID(uint8_t);
void    Go_To_Sleep(void);
void    Update_Stim_VI(void);  
void    Send_Serial(void);
void    Check_BLE(void);
void    Process_Command(void);
void    Int_2_Str(int16_t data);
void    SendString_2_BT(char* string, uint16_t  Delay);
void    Configure_BT(void);    
void    ThreeBeeps(void);
void    TwoBeeps(void);

// ************************* Start Code ****************************************

/**
void    UpdateLEDs(void)
  
**/
void    Update_LEDs(void)
{
    static uint8_t  ToggleBT;
 
    fprintf(DEBUG,"\n\r state=%u", g_BTState );
    
    if(g_BTState == UNPAIRED)   //if not paired 
    {
       if(ToggleBT)    //then toggle blue LED on and off
        {
            ToggleBT = 0;
            output_low(BLUE_LED); //Blue LED active indicating not paired
        }
        else
        {
            ToggleBT = 1;
            output_high(BLUE_LED); //Blue LED active indicating not paired  
        }
    }
    else if(g_BTState == PAIRED)    //we are paired
    {
        output_high(BLUE_LED); //Blue LED active indicating not paired      
    }
}


/**
    Hard Reboot the RN4678 BT module

**/
void    Reboot_BT(void)
{
    disable_interrupts(INT_TIMER1); //then enable 10msec interrupt
    output_low(RESET_BT);
    delay_ms(1);
    output_high(RESET_BT);
    fprintf(DEBUG,"\n\r BT Reset");
    delay_ms(100);
    enable_interrupts(INT_TIMER1); //then enable 10msec interrupt
}



/**
    Get the state of the bluetooth connection
**/
void     Get_BTState(VOID)
{

    static uint8_t    LastBTState = PAIRED; 
    static uint8_t    LastState;
  
    if(input(STATUS))   //if not paired now
    {
        if(LastBTState == PAIRED) //and last BT state was paired
        {
            g_BTState = UNPAIRED;   //then updare pairing status
        
            if(g_State == TREAT)    //if in treat state
            {
                LastState = TREAT;
                g_State = WAIT;
                Reboot_BT();    //reset BT radio
            }
        }
    }
    else     //device is paired now
    {
        if(LastBTState == UNPAIRED)       
        {
            g_BTState = PAIRED;
            
            if(LastState == TREAT)
            {
                 g_State = WAIT;
            }
       }
    } 
    LastBTState = g_BTState;
}


/**
*  Send contents of cmd[] to RN4678 BT controller over serial stream UART.
*  Requires UART Serial Interrupt Service Routine to be enabled
*
**/
void SendString_2_BT(char* string)
{
    char CR[] = "\r" ;
    char cmd[30] ;

    strcpy(cmd,string) ;
    strcat(cmd,CR) ;

    fprintf(UART,"%s",cmd) ;
    delay_ms(250);
    restart_wdt();  //reset watchdog
}


/**
    Command RN4678 to enter command mode. Send "$$$"
**/
void    CMD_Mode(void)
{
    
    uint8_t CONST DELAY = 20;
   
    fprintf(DEBUG,"\n\r Configuring BT");
    fprintf(DEBUG,"\n\r ");
    delay_ms(200);
    
    fputc(0x24,UART);   // send $ to RN4677
    delay_ms(DELAY);
    fputc(0x24,UART);
    delay_ms(DELAY);
    fputc(0x24,UART);
    delay_ms(500);     //allow for response
 }
 


 /**
    Configure the RN4678 BT module

**/
void    Configure_BT(void)
{
    char cmd[20];
       
    cmd = "SA,2";    // authenitication mode = 2
    SendString_2_BT(cmd);
    
    cmd = "SG,0";    // dual mode (BLE and SPP)
    SendString_2_BT(cmd);

    cmd = "SM,0";    // slave mode
    SendString_2_BT(cmd);

    cmd = "SN,Halo";    // device advertised name
    SendString_2_BT(cmd);

    cmd = "SY,4";    // RF power setting, high
    SendString_2_BT(cmd);

    cmd = "SS,Serial Port";    // service name
    SendString_2_BT(cmd);

    cmd = "R,1";    // save in flash and reboot
    SendString_2_BT(cmd);
    fprintf(DEBUG,"\n\r Done BT Config");

}





/**
void State_Machine(void)
**/
void State_Machine(void)
 {
   switch(g_State)
    {
        case WAIT:
             if( (input(SYNC)) && (!input(CHRGR_ON) ) ) //if new VI data available and charger not attached
                 Update_Stim_VI();   //get it
             
             g_Setpoint_mA = 0;
             break;

        case TREAT:
            if( (input(SYNC)) && (!input(CHRGR_ON) ) ) //if new VI data available and charger not attached
                Update_Stim_VI();   //get it
            
            g_Setpoint_mA = g_MaxSetPoint;
            break;
              
        case FAULT:
            Go_To_Sleep();
            break;
            
        default:
            g_State = FAULT:
            break;
    }
 }


/**
void    Go_To_Sleep(void)

**/
void    Go_To_Sleep(void)
{
    disable_interrupts(INT_TIMER1); //disable 10msec interrupt
    output_low(BLUE_LED);   //turn off b;ue LED
    output_low(HAPTIC);
        
    while(g_DACVal >1)
    {
        g_DACVal-=1; //reduce stimulation amplitude
        set_pwm1_duty(g_DACVal);  //update PWM 
        delay_ms(10);
        fprintf(DEBUG,"%lu,", g_DACVal);
    } 
    
    output_low(RELAY);  //turn relay off
    set_pwm1_duty(0);   //turn off stimulation boost 
    set_pwm2_duty(0);   //turn off charger voltage;  
    output_high(ENB_SW_BAT);  //turn off power to stimulation circuit
    fprintf(DEBUG,"\n\r Sleep");
    output_low(BLUE_LED);   //turn off b;ue LED
    delay_ms(50);
    output_low(ENB_VCC);
    while(1) { } //wait till dead

}


void    Init_Globals(void)
{
    g_Flags = 0;
    g_DACVal = 0;
    g_Setpoint_mA = 0;
    g_Seconds = 2400;   // = 40 minutes * 60 seconds
    g_State = 0;
    g_Charge_State = INIT_CHARGE; //reset charge state
    g_Haptic_Count = 0;
}


void Init_IO(void)
{
    uint8_t junk;
   // output_high(ENB_SW_BAT);  //turn off power to stimulation circuit
    output_low(ENB_SW_BAT);  //turn on power to stimulation circuit
    output_low(RELAY); //disconnect stimulation output to patient
    output_high(RESET_BT);   //take BLE ot of reset
    output_high(BLUE_LED);
    output_low(HAPTIC);
    junk = input(PIN_B2); //make an input
    g_Gain_Err = SLOW_RAMP;    //slow gain for ramp up

 }


void Self_Test(void)
{
  //  delay_ms(10);   //allow serial port to settle down
    fprintf(DEBUG,"\n\r");
    fprintf(DEBUG,"\n\r Self Test");
    
    if(!input(PGD)) { //if programmer is attached
        fprintf(DEBUG,"\n\r Programmer attached");
        write_eeprom(SET_ADDR, DEFAULT_MA); //load value from code into EEPROM
        g_MaxSetpoint = (float)DEFAULT_MA;  //convert to float
    }
    else {
        fprintf(DEBUG,"\n\r Programmer not attached");
        g_MaxSetpoint = (float)read_eeprom(SET_ADDR);  //read max setpoint from eeprom
    }
  // jpc  g_MaxSetPoint = g_MaxSetPoint * 40.0;
     g_MaxSetPoint = 400;
    
    fprintf(DEBUG,"\n\r MaxSetpoint = %f", g_MaxSetPoint);
    
    if(input(CHRGR_ON))
        fprintf(DEBUG,"\n\r Charger attached");    
    else
        fprintf(DEBUG,"\n\r Charger not attached");
    
    Update_BatteryVI();
    fprintf(DEBUG,"\n\r Battery=  %lu",g_Batt_Volts);

    output_high(ENB_VCC);   //turn on power
    output_high(HAPTIC);    //test haptic
    delay_ms(200);
    output_low(HAPTIC);
    output_high(RELAY); //connect output to patient
    
    g_State = WAIT;

}


#INT_TIMER1
void  TIMER1_isr(void) 
{
    set_timer1(45500);      //10msec interrupt
    bit_set(g_Flags,TMR1_FLG);
}   


/**
    Service Serial Port Interrupt
    Service the interupt triggeed by a serial byte from the BT UART 
    @returns none
**/   
#int_RDA2
void RDA2_isr()
{
    char c;

    c = fgetc(UART);    // Get byte from Bluetooth UART
  //  fputc(c,DEBUG);     //echo outr debug port
   
    switch(c)
        {     // Do according to the incoming byte count
        
        case '$':        // '$' - beginning of the string
            g_UART.flags = COMM_CMD;    // Set start of command flag, reset others
            g_UART.len = 0;  // Zero new command buffer length
            break;
   
        case '#':                            //possible end of the packet
            g_UART.flags |= COMM_FND_LF;    // Set the <LF> flag
            g_UART.flags |= COMM_FND_CR;    // Set the <CR> flag
            break;

        default:
            if (g_UART.flags & COMM_CMD)                // If we have already received the '$'
                {
                g_UART.buff[g_UART.len] = c;   // Store new character in the buffer
                ++g_UART.len;          // Increment the buffer length
                }
            break;
        }   // End switch
   
    // Test to see if we have a complete command in the buffer
    if ((g_UART.flags & (COMM_CMD | COMM_FND_CR | COMM_FND_LF)) ==
        (COMM_CMD | COMM_FND_CR | COMM_FND_LF))
    {
        g_UART.flags = COMM_FND_CMD;   // Set command found flag, reset others
    }
}    


/**
Indicate tteatment has stopped  by beeping the haptic

**/
void    ShutDown(void)
{
    uint8_t n;
     
    for(n=0; n<10; n++)
    {
        output_high(HAPTIC);
        delay_ms(100);
        output_low(HAPTIC);
        delay_ms(250);
    }
    g_State = WAIT;
}



/**
void    Clocks(void)
   Executed every 10 msec, this function updates all clocks

**/
void Clocks(void)
{
    static uint16_t   tmr1_cntr = 0;   //counts TIMR1 interrupts
    static uint8_t  g_Beep;;
    
    tmr1_cntr++;    //inc 10msec interrupt count
    
    //do the following tasks once a second
    if(!(tmr1_cntr % 100)) //if 1000ms have passed
    {
        Update_BatteryVI();    //get battery params
        Charge_Battery();   //charge battery
       
        if(!Input(CHRGR_ON))   { //if charger is not attached 
            Get_BTState();    //get BT state
            Update_LEDs();  //update LEDs
        }
        
        if((g_Contact <7) && (g_DACVal > 10)) {  //if an electrode contact is poor
            g_Beep =20; //beep period
            output_high(HAPTIC);    //start now
        }
     
    }
    if(g_Beep) {
        g_Beep--;
        if(!g_Beep)
            output_low(HAPTIC); //turn beep off
    }
        
 }  
    
 

Void Update_Stim_VI(void)
{
   float ftemp;
    uint16_t  ohms;
    
    set_adc_channel(VSENSE);  //set ADC channel
    delay_us(50);
    ftemp = (float)read_adc();  //get peak stimulation voltage
    g_Vsense = (uint16_t)(ftemp *0.32);

    set_adc_channel(LSENSE);  //set ADC channel
    delay_us(50);
    g_Lsense = read_adc();       // get left mA
      
    set_adc_channel(RSENSE);  //set ADC channel
    delay_us(50);
    g_Rsense= read_adc();   //add 
 
   
   g_Total_mA = (float)(g_Lsense + g_Rsense);  //calculate total current in both mastoids 
   g_Contact = 7;    //start with all three electrodes good
   
   
    if((g_Lsense) && (g_Rsense))  //if currents are non-zero 
    {
        if( (g_Rsense / g_Lsense) >= 3)  //left side cuurent low
        {
            bit_clear(g_Contact, LEFT_Flg);  //clear flag
            bit_set(g_Contact, RIGHT_Flg);  //set flag
        }
        else if( (g_Lsense / g_Rsense) >= 3) 
        {
            bit_clear(g_Contact, RIGHT_Flg);
            bit_set(g_Contact, LEFT_Flg);  //set flag
        }
    }
    
   if(g_State == TREAT) {
       ohms = (g_Vsense * 100)/g_Total_mA;  //calulate forhead resistance, R=E/I
          
       if(Ohms > 100)
            bit_clear(g_Contact,FORE_Flg);
       else
           bit_set(g_Contact,FORE_Flg);
            
        if(g_Contact < 7)
        {
           g_Setpoint_mA = 100;
           g_Gain_Err = FAST_RAMP;
        }
        else
       {
           g_Setpoint_mA = g_MaxSetPoint;
           g_Gain_Err =SLOW_RAMP;
       }
    }
   
    if(g_State == WAIT) {
        if(g_Setpoint_mA >5) {
            g_Setpoint_mA-=5;
          //  fprintf(DEBUG,"\n\r %f", g_SetPoint_mA);
        }
       g_Contact = 0;       //all electrodes are poor contact     
       g_Gain_Err = FAST_RAMP;
   
   }
    PID(RUN);   //update PID
}


/**
    Get headset battery voltage and current and scale battery voltage to a %level
**/
void    Update_BatteryVI(void)
{
    float ftemp;
 //   int16_t const DEAD_BATT = 340;
    
    set_adc_channel(BATT_V);  //set ADC channel
    delay_us(100);
    ftemp= (float)read_adc();
    g_Batt_Volts = (long)(ftemp * 0.64) ;   //100x battery voltage, 0.00322 V/lsb * 2(scale) * 100 = 0.64
      
    set_adc_channel(BATT_I);  //set ADC channel
    delay_us(100);
    ftemp= (float)read_adc();
    g_Batt_Amps  = (long)(ftemp *3.22);   //0.00322/lsb x 1000mA/V = 3.22
 
  //  fprintf(DEBUG,"\n\r %lu, %lu,",  g_Batt_Volts , g_Batt_Amps );
    
 //   if(g_Batt_Volts < DEAD_BATT)    //at dead battery
 //    {
 //        fprintf(DEBUG,"\n\r Dead Battery");
  //       Go_To_Sleep();   //shutdown
   //  }  
}



/******************************************************************************
*
* FUNCTION     : PID(uint8_t Cmd). Called when there is fresh VI data 
* INPUTS       : command byte - RUN or CLEAR
* OUTPUTS      : g_DACVal byte to DAC
* RETURNS      : none
* DESCRIPTION  : Usa a PI loop to control the stimulation voltage to 
   track to a treatment current setpoint
/*****************************************************************************/

void   PID(uint8_t Cmd)
{
    static float    Err_Integral = 0;
    float   Error;
    float temp1;
  
    disable_interrupts(INT_TIMER1);

    switch(Cmd)
     {
         case CLEAR: //reset things
             Err_Integral = 0;
             g_DACVal = 0;
             temp1 = 0;
             g_Total_mA = 0;
             g_Lsense = 0;
             g_Rsense = 0;
             
             break;
               
        case RUN:   //run the control loop using mA  setpoint
            Error = g_Gain_Err * (g_Setpoint_mA - g_Total_mA);
            Err_Integral += Error;  //update integral
              
            if (Err_Integral > 150)  //g_MaxSetpoint)    //limit integral to prevent rollover
                Err_Integral = 150; //g_MaxSetPoint;
          
            if (Err_Integral < 0)   //limit to positive value
                Err_Integral = 0;
                  
            g_DACVal = (long) Err_Integral;    //convert integral to DAC value
       //    fprintf(DEBUG,"\n\r %f, %f, %lu, %lu, %lu, %lu, %lu ", g_Setpoint_mA, Err_Integral, g_DACVal, g_Lsense,g_Rsense, g_Vsense, g_Seconds);
            break;
           
       Default:
          break;
     }
 
     set_pwm1_duty(g_DACVal);  //update PWM count
     enable_interrupts(INT_TIMER1);
}



/**
Battery charger function 
    Battery PWM range is 100 to 240. Reducing PWM value increase charge voltage
    Start with constant curent then constant voltage till current level reaches minimum
**/
void    Charge_Battery(void)
{
    int16_t const PWM_MAX = 250; //this PWM value creates the maximum charging voltage
    int16_t const FULL_BATT = 420;  //volts *100
    int16_t const END_AMPS = 100;  //end current (10% of rated AH *2
    uint16_t const CC_AMPS = 500l;    //charge current mA
    uint16_t const MAX_TIME = 3600l;    //2 hours in seconds to timeout at at contant voltage mode
    uint16_t const CC_TIME = 7200l;     //2 hours in seconds to timeout at at contant current mode
    static uint16_t Batt_PWM;
    static uint16_t  Charge_Time;
    static uint8_t   Led, Bars;
    static uint16_t   Offset;
    
    if(!Input(CHRGR_ON))  //if charger is not attached
    {
        g_Charge_State = INIT_CHARGE; //reset charge state
    }
    else    //charger is attached
    {
        output_high(ENB_SW_BAT);  //turn off power to rest of circuit
                 
        switch (g_Charge_State)
        {
            case INIT_CHARGE:
            
                Charge_Time = 0;
                Bars = 0;
                g_Charge_State = CC;
                Batt_PWM = 0;
                set_pwm2_duty(Batt_PWM);   //turn off charger PWM
                Offset = g_Batt_Amps;
                break;
                
            case CC:    //constant current mode
            
                if(g_Batt_Amps > Offset)    //adjust charging current for offset
                   g_Batt_Amps -= Offset;
                else
                   g_Batt_Amps = 0;
               
                if( (g_Batt_Amps < CC_AMPS -1L) && (g_Batt_Volts <= FULL_BATT) )
                {
                    if(Batt_PWM < PWM_MAX)
                    {
                        Batt_PWM +=2;   //increase PWM, inrease charge voltage
                    }    
                }
                else if(g_Batt_Amps > CC_AMPS +5L) 
                {
                    if(Batt_PWM)    //if non-zero
                        Batt_PWM--;     //reduce duty cycle, reduce charge voltage
                }
                
                //if at full voltage or time out or at max charge voltage
                if( (g_Batt_Volts >=  FULL_BATT) || (Charge_Time > CC_TIME) )  
                {
                    g_Charge_State = CV;    //change to CV state
                    Charge_Time = 0;      //reset safety timer
                }
                else
                {
                     Charge_Time++;    //increment timer 
                }
                break;
        
            case CV:    //constant voltage
                  if(g_Batt_Amps > Offset)    //adjust charging current for offset
                   g_Batt_Amps -= Offset;
                else
                   g_Batt_Amps = 0;
                    
                if(g_Batt_Volts >  FULL_BATT)
                {
                   Batt_PWM--;  //decrase charge voltage
                }
                
                 if( (g_Batt_Volts <  FULL_BATT) && (Batt_PWM < PWM_MAX))
                {
                   Batt_PWM++;  //increase charge voltage
                }
                
                //if at end current or at end of 2 hours
                if( (g_Batt_Amps <= END_AMPS) || (Charge_Time > MAX_TIME))  
                {
                    g_Charge_State = FULL;    //change state
                    Batt_PWM = 0;   //turn off charger voltage
                } 
                Charge_Time++;    //increment timer
              
                break;
            
            case FULL:
                Batt_PWM = 0; //disable charge voltage
                fprintf(DEBUG,"Battery Full");
                Go_To_Sleep();
                break;
                
            default:
                break;
        }
        
        switch(Led)
        {
            case 0:
                output_high(BLUE_LED);
                Led++;
                break;
            case 1:
               output_low(BLUE_LED);
                Led++;
                break; 
            case 2:
               output_low(BLUE_LED);
                Led++;
                break;    
            case 4:
               output_low(BLUE_LED);
                Led=0;
                break;
            default:
                 Led=0;
                 output_low(BLUE_LED);
                 break;    
                
                
        }
        set_pwm2_duty(Batt_PWM);   //update PWM output
        fprintf(DEBUG,"\n\r %u, %lu , %lu, %lu, %lu, %lu ", g_Charge_State, Batt_PWM,  g_Batt_Volts, g_Batt_Amps, (g_Batt_Amps + Offset), Charge_Time);
    }
 }



  

/**
*  Process the data packet received From the app. Packet starts with a '$', fields
* are deliniated by a '/', packet ended with a '#'. The packet start and stop 
* characters have been renoved by the serial port ISR
**/   
void Process_Command(void)   
{ 
    char  term[6];
    char  *pntr;    //fixed 20170728 JPC
    uint8_t n, Id; //TPP ID  0-9
    

    strcpy(term,"/");
    pntr = strtok(g_UART.buff, term);
       
    switch(pntr)
    {
           
        case "set": //set the treatment setpoint
            fprintf(UART,"\n\r ");      //echo to app
            fprintf(UART,"\n\r $%s", pntr);     //echo command to app
            fprintf(DEBUG,"\n\r $%s", pntr);     //echo command to app
            pntr = strtok(0, term);  //get next field
            Id = atoi(pntr); //which is sequence number
            
            if(Id > 15) //limit the current to 15 mA
                Id = 15;
                
            write_eeprom(SET_ADDR,Id); //store new max setpoint in EEPROM
            g_MaxSetpoint = 40.0 * ((float)Id);  //update run time maximum current setpoint
            
                       
            fprintf(UART,"/%s", pntr);
            fprintf(UART,"/#");  //send string end
            fprintf(DEBUG,"/%s", pntr);
            fprintf(DEBUG,"/#");  //send string end
             break;
      
       case "on":   //set stimulation level and respond with data
            fprintf(UART,"\n\r ");      //echo to app
            fprintf(UART,"\n\r $%s", pntr);     //echo command to app
            fprintf(UART,"/%u ", g_Contact);    //electrode contact
            fprintf(UART,"/%lu ", g_Batt_Volts);    //battery level
            fprintf(UART,"/%lu ", g_Seconds);    //time
            fprintf(UART,"/#");  //send string end
            fprintf(DEBUG,"\n\r $%s", pntr);     //echo command to app
            fprintf(DEBUG,"/%u ", g_Contact);    //electrode contact state
            fprintf(DEBUG,"/%lu ", g_Batt_Volts);   //battery level
            fprintf(DEBUG,"/%lu ", g_Seconds);    //time
            fprintf(DEBUG,"/#");  //send string end
            g_State = TREAT;
         //   fprintf(DEBUG,"\n\r set= %f, %f ", g_Setpoint_mA, g_MaxSetPoint);
            break;

      case "off":   //set stimulation level to zero and respond with data
            fprintf(UART,"\n\r ");      //echo to app
            fprintf(UART,"\n\r $%s", pntr);     //echo command to app
            fprintf(UART,"/%u ", g_Contact);    //electrode contact
            fprintf(UART,"/%lu ", g_Batt_Volts);    //battery level
            fprintf(UART,"/%lu ", g_Seconds);    //time
            fprintf(UART,"/#");  //send string end
            fprintf(DEBUG,"\n\r $%s", pntr);     //echo command to app
            fprintf(DEBUG,"/%u ", g_Contact);    //electrode contact state
            fprintf(DEBUG,"/%lu ", g_Batt_Volts);   //battery level
            fprintf(DEBUG,"/%lu ", g_Seconds);    //time
            fprintf(DEBUG,"/#");  //send string end
            g_State = WAIT;
         // fprintf(DEBUG,"\n\r set= %f, %f ", g_Setpoint_mA, g_MaxSetPoint);
            break;

       case "data":   //set stimulation level and respond with data
            fprintf(UART,"\n\r ");      //echo to app
            fprintf(UART,"\n\r $%s", pntr);     //echo command to app
            fprintf(UART,"/%u ", g_Contact);    //electrode contact
            fprintf(UART,"/%lu ", g_Batt_Volts);    //battery level
            fprintf(UART,"/%lu ", g_Seconds);    //time
            fprintf(UART,"/#");  //send string end
            fprintf(DEBUG,"\n\r $%s", pntr);     //echo command to app
            fprintf(DEBUG,"/%u ", g_Contact);    //electrode contact state
            fprintf(DEBUG,"/%lu ", g_Batt_Volts);   //battery level
            fprintf(DEBUG,"/%lu ", g_Seconds);    //time
            fprintf(DEBUG,"/#");  //send string end
            g_State = TREAT;
         //  fprintf(DEBUG,"\n\r set= %f, %f ", g_Setpoint_mA, g_MaxSetPoint);
            break;
            
        case "sleep": //this command will sound 5 peeps onthe haptic then power down
            enable_interrupts(INT_TIMER1);
            for(n=0; n<=5; n++)  {  //beep 5 times
                output_high(HAPTIC);
                delay_ms(200);
                output_low(HAPTIC);
                delay_ms(500); 
            }
            Go_To_Sleep();
            break;
            
        default:
            break;
    }
}



// Look for charger being unplugged
void Check_Charger(void)
{
    static uint8_t Charger_State;
    static uint8_t Last_State = 0;
    
    Charger_State = input(CHRGR_ON);    //update state
 
    if( (Last_State) && (!Charger_State))   //if charging has stopped
    {
       fprintf(DEBUG,"\n\r Sleep");
       delay_ms(50);
       output_low(ENB_VCC); //disable CPU power     //go to sleep
       while(1) {}  // hang forever
 
    }
    Last_State = Charger_State;
}


void    Check_Button(void)
{
    uint8_t ButtonLevel;
    static uint8_t  LastButton = 1;
    
    ButtonLevel = input(BUTTON);    //active low when pressed

    if( (ButtonLevel) && (!LastButton) )    //if button released
    {
        fprintf(DEBUG,"\n\r Sleep");
        output_low(ENB_VCC); //disable CPU power
        while(1) {}  // hang forever
        
    }
    LastButton = ButtonLevel;   //refresh button status
}



/**
    Clear serial port error flags
**/
void    ClearPorts(void)
{
    if(bit_test(RCSTA1,OERR))   //if error bit set
    {
        bit_clear(RCSTA1,SPEN);     //toggle these bits
        bit_clear(RCSTA1,CREN);
        delay_us(10);
        bit_set(RCSTA1,SPEN);
        bit_set(RCSTA1,CREN);
    }
    
      if(bit_test(RCSTA2,OERR))   //if error bit set
    {
        bit_clear(RCSTA2,SPEN);     //toggle these bits
        bit_clear(RCSTA2,CREN);
        delay_us(10);
        bit_set(RCSTA2,SPEN);
        bit_set(RCSTA2,CREN);
    }
}



/**
void main(void)

**/
void main(void)
{
    output_high(ENB_VCC);
    delay_ms(1000); //allow user to get finger off button
    
    setup_adc_ports(sAN0 | sAN1 | sAN2 | sAN3 | sAN5 | VSS_VDD);
    setup_adc(ADC_CLOCK_INTERNAL | ADC_TAD_MUL_0);
    
    setup_timer_1(T1_INTERNAL|T1_DIV_BY_4);        //32.7 ms overflow
    setup_timer_2(T2_DIV_BY_1 | T2_CLK_INTERNAL,63,1);    //8.0 us overflow, 8.0 us interrupt, 
  
    setup_ccp1(CCP_PWM);        //DAC_PWM, 125KHz, 256 max
    setup_ccp2(CCP_PWM);        //BATTERY_PWM, 125KHz, 256 max
    set_pwm1_duty((int16)0);    //increase PWM to increase stimulation
    set_pwm2_duty((int16)0);   //increse PWM to increase charge voltage

    port_b_pullups(128);
    Init_Globals();
    PID(CLEAR);
    Init_IO();
    Self_Test();
    fprintf(DEBUG,"\n\r Initializing Main Controller Ver 3.0.5");
    
 
   clear_interrupt(INT_RDA2);
    enable_interrupts(INT_RDA2);  
   enable_interrupts(INT_TIMER1);
   enable_interrupts(GLOBAL);
   
 //  if(!input(PGD)) { //if programmer is attached
        ClearPorts();   //check for Bluetooth UART serial port error
        CMD_Mode(); //place BT device into command mode
        Configure_BT(); //and configure it
  //  }
    enable_interrupts(INT_TIMER1);
  
    while(TRUE)
    {

        If(bit_test(g_Flags,TMR1_FLG))  //do things on interrupt
        {
            bit_clear(g_Flags,TMR1_FLG);    //clear 1 msec interrupt flag
            Clocks();   //update clocks
            State_Machine();    //state machine
            
            if(!input(CHRGR_ON))    //charger not attached
                Check_Button(); //look for power button closure
         
            Check_Charger();   //check charger
        }


        //perform the following tasks as quickly as possible
         ClearPorts();   //check for Bluetooth UART serial port error
                 
        // Check for packet from GUI 
        if (g_UART.flags == COMM_FND_CMD)   // Check for the communications buffer
        {  
          // disable_interrupts(INT_TIMER1); //then enable 10msec interrupt
           Process_Command();    // Process command
         //  enable_interrupts(INT_TIMER1); //then enable 10msec interrupt  
            g_UART.flags &= ~COMM_FND_CMD;          // Reset command found flag
        }  // End check comm buffer  
    }
}



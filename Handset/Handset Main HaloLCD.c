
//IO Map
#define RD          PIN_E0      //DO, DISPLAY READ ACTIVE LOW  p25
#define WR          PIN_E1      //DO, DISPLAY WRITE ACTIVE LOW  p26
#define CS          PIN_E2      //DO, DISPLAY SELECT ACTIVE LOW  p27

#define CHRGR_ON    PIN_A2      //DI,SET WHEN CHARGER ATTACHED
#define P2_0        PIN_A3      //D0, TEST = 0, APP = 1
//#define ENB_VCC     PIN_A5      //DO, SETCTO ENABLE VCC
//#define LED1        PIN_A6      //DO, SET TO TURN ON PCB LED

#define BUTTON       PIN_B0      //DI, ACTIVE LOW BUTTON1 SENSE (On/Off)
//#define BUTTON2      PIN_B1      //DI, ACTIVE LOW BUTTON2 SENSE  (Start/Stop
#define DC           PIN_B2      //DO, 1=DATA, 0=COMMAND  p10
#define TXD         PIN_B3      //DO, DEBUG SERIAL OUT
#define RST         PIN_B4      //DO, 1= RESET DISPLAY
//#define RST_BLE     PIN_B5      //DO, BLE RESET ACTIVE LOW

uint8_t const   MIN_TENS =0;
uint8_t const   MIN_ONES =1;
uint8_t const   SEC_TENS =2;
uint8_t const   SEC_ONES =3;

uint8_t const   NUM_ZERO = 0;
uint8_t const   NUM_ONE = 1;
uint8_t const   NUM_TWO = 2;
uint8_t const   NUM_THREE = 3;
uint8_t const   NUM_FOUR = 4;
uint8_t const   NUM_FIVE = 5;
uint8_t const   NUM_SIX = 6;
uint8_t const   NUM_SEVEN = 7;
uint8_t const   NUM_EIGHT = 8;
uint8_t const   NUM_NINE = 9;
uint8_t const       POOR = 0;
uint8_t const       GOOD = 1;
uint8_t const       LEFT = 1;
uint8_t const       RIGHT = 0;
uint8_t const DELAY1 = 100; //100ms delay  

// Protypes
void    Display_Setup(void);

void Draw_Zero(void);
void Draw_One(void);
void Draw_Two(void);
void Draw_Three(void);
void Draw_Four(void);
void Draw_Five(void);
void Draw_Six(void);
void Draw_Seven(void);
void Draw_Eight(void);
void Draw_Nine(void);
void Draw_BLE(void);
//void Draw_Full_Battery(uint8_t BattID);
  
/**
   Comm_Out(uint8 c)
   Write a command byte to display.
**/
void    Comm_Out(uint8_t c)
{
    output_high(RD);    //Set at start
    output_high(WR);    //Set at start 
    output_low(DC);     //clear to send a command
    output_d(c);        //load value
    delay_cycles(2);    //short delay
    output_low(WR);     //start write latch
    delay_cycles(2);    //short delay
    output_high(WR);    //stop write latch 
 }


/**
   Data_Out(uint8_t c
   Write a data byte to display
**/
void    Data_Out(uint8_t c)
{
    output_high(RD);    //set at start 
    output_high(WR);    //Set at start 
    output_high(DC);     //set for data 
    output_d(c);        //load value
    delay_cycles(2);    //short delay
    output_low(WR);     //start write latch
    delay_cycles(2);    //short delay
    output_high(WR);    //stop write latch 
 }

/**
   Data_In(uint8 c)
   Read a data byte from display
**/
uint8_t    Data_In(void)
{
    uint8_t    c;
    
    output_high(RD);    //set at start 
    output_high(WR);    //Set at start  
    output_high(DC);     //set for data 
    output_low(RD);     //start read latch
    delay_cycles(2);    //short delay   
    c=input_d();        //load value
    output_high(RD);    //stop read latch 
    return c;   //return result
}
  
  
  
/**
    Configure the NCD-1.8-128160 TFT display

**/
 
void    Display_Setup(void)
{
  //  uint8_t const DELAY1 = 100; //100ms delay  
    
    fprintf(DEBUG,"\n\r Start Display Config");
    output_low(CS);         //put display in continuous 8 bit parallel mode 
    output_high(RD);        //read mode not selected
    output_high(WR);        //write mode not selected
    output_high(RST);       //release hard reset
    delay_ms(DELAY1);
 
    Comm_Out(0x11);       //exit sleep mode
    delay_ms(DELAY1);
    
    Comm_Out(0x28);       //set display off
    delay_ms(DELAY1);

    Comm_Out(0x26);      //select gamma curve
    Data_Out(0x04);
    
    Comm_Out(0xF2);      //select F2
    Data_Out(0x00);   
    
    Comm_Out(0xB1);      //frame rate control
    Data_Out(0x0A);
    Data_Out(0x14);
  
    Comm_Out(0xC0);     //power control 1
    Data_Out(0x0A); 
    Data_Out(0x00);
  
    Comm_Out(0xC1);     //power control 2
    Data_Out(0x02);
  
    Comm_Out(0xC5);     //VCOM control 1
    Data_Out(0x2F); 
    Data_Out(0x3E);
  
    Comm_Out(0xC7);     //VCOM offset control
    Data_Out(0x40);
  
    Comm_Out(0x2A);    //column address set
    Data_Out(0x00);                  
    Data_Out(0x00);     //start 0x0000
    Data_Out(0x00);
    Data_Out(0x7F);     //end 0x007F (0-127)
  
    Comm_Out(0x2B);     //page address set
    Data_Out(0x00);                  
    Data_Out(0x00);     //start 0x0000
    Data_Out(0x00);
    Data_Out(0x9F);     //end 0x009F  (0-159)
  
    Comm_Out(0x36);     //memory access control
    Data_Out(0xC8);
  
    Comm_Out(0x3A);    //pixel format = 18 bit per pixel
    Data_Out(0x06);                  
    
    Comm_Out(0x29);      //display ON
    delay_ms(DELAY1);
    
    Comm_Out(0x2C);      //write memory start
    delay_ms(DELAY1);    
  
    fprintf(DEBUG,"\n\r End Display Config"); 
 }
 
 
 void Set_Column_Address(int8_t a, int8_t b, int8_t c, int8_t d)
{
    Comm_Out(0x2A);
    Data_Out(a);Data_Out(b);
    Data_Out(c);Data_Out(d);
}


void Set_Page_Address(int8_t a, int8_t b, int8_t c, int8_t d)
{
    Comm_Out(0x2B);
    Data_Out(a);Data_Out(b);
    Data_Out(c);Data_Out(d);
}

 
/**
    Turn on all pixesl in red, green and blue then black
 
 **/
 void Display(void)
{
    uint16_t i;

    Comm_Out(0x2C);     //comamnd to write to frame memory
    
    for(i=0; i<20480; i++)   //for each pixel
    {
        Data_Out(0xFF);     //all red pixels    
        Data_Out(0x00);     //    
        Data_Out(0x00);     //   
    }
    
     for(i=0; i<20480; i++)   //for each pixel
    {
        Data_Out(0x00);     //   
        Data_Out(0xFF);     // all  
        Data_Out(0x00);     // 
          
    }
    
     for(i=0; i<20480; i++)   //for each pixel
    {
        Data_Out(0x00);     //   
        Data_Out(0x00);     //   
        Data_Out(0xFF);     // all blue   
    
    }
    
       for(i=0; i<20480; i++)   //for each pixel
    {
        Data_Out(0x00);     //   
        Data_Out(0x00);     //   
        Data_Out(0x00);     // all black
    }
}

/**
    Start location in display memory
**/
void Write_Memory_Start(void)
{
    Comm_Out(0x2C);
}


/**
    Draw a white circle using 64 5x5 blocks
**/
void    Draw_Colon(void)
{
    uint8_t n;
    uint8_t  X_CENTER = 64;    //column(X) value for center
    uint8_t  Y_CENTER = 60;    //row (Y) value for center
    
/**    Set_Column_Address(0,X_CENTER,0,X_CENTER); //X coordinate
    Set_Page_Address(0,Y_CENTER,0,Y_CENTER);    //y coordinate
    Write_Memory_Start();
  
    Data_Out(0xFF);
    Data_Out(0xFF);
    Data_Out(0xFF);   
**/
    Y_CENTER = 55;    //upper colon block
    Set_Column_Address(0,X_CENTER-1,0,X_CENTER+1); //X coordinate
    Set_Page_Address(0,Y_CENTER-1,0,Y_CENTER+1);    //y coordinate
    Write_Memory_Start();
    for(n=0; n<9; n++) //draw a block
    {
        Data_Out(0xFF);
        Data_Out(0xFF);
        Data_Out(0xFF);
    }   


    Y_CENTER = 65;    //lower colon block 
    Set_Column_Address(0,X_CENTER-1,0,X_CENTER+1); //X coordinate
    Set_Page_Address(0,Y_CENTER-1,0,Y_CENTER+1);    //y coordinate
    Write_Memory_Start();
    for(n=0; n<9; n++) //draw a block
    {
        Data_Out(0xFF);
        Data_Out(0xFF);
        Data_Out(0xFF);
    }   
}

/**
    Draw a digit at the location
**/
void    Draw_Digit(uint8_t Location, uint8_t Value)
{
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    switch(Location)        //identify center of the time digit
    {
        case MIN_TENS:  //tens digit of minutes
            x=  X_CENTER - 30;
            y = Y_CENTER;
            break;
            
       case MIN_ONES:   //ones digit of minutes
            x=  X_CENTER - 12;
            y = Y_CENTER;       
            break;
            
       case SEC_TENS:   //tens digit of seconds
            x=  X_CENTER + 12;
            y = Y_CENTER;       
            break;
    
       case SEC_ONES:       //oness digit of seconds
            x=  X_CENTER + 30;
            y = Y_CENTER;              
            break;    
    }
    
    Set_Column_Address(0,(x-7),0,(x+7)); //X coordinate
    Set_Page_Address(0,(y-12),0,(y+12));    //y coordinate
    Write_Memory_Start();
     
    switch(Value)   //now draw the number in the time foield
    {
        Case NUM_ZERO:
            Draw_Zero();
            break;
            
        case NUM_ONE:
            Draw_One();
            break;

        Case NUM_TWO:
            Draw_Two();
            break;
            
        case NUM_THREE:
            Draw_Three();
            break ;           

         Case NUM_FOUR:
            Draw_Four();
            break;
            
        case NUM_FIVE:
            Draw_Five();
            break;       

         Case NUM_SIX:
            Draw_Six();
            break;
            
        case NUM_SEVEN:
           Draw_Seven();
            break;       
 
        Case NUM_EIGHT:
            Draw_Eight();
            break;
            
        case NUM_NINE:
            Draw_Nine();
            break;       
    }
 } 
    


/**
    Draw a blue circle using  5x5 blocks. If Degrees = 64 then full circle
    
**/
void    Draw_Circle(void)
{
    int8_t n,i,x,y;
    int8_t  const X_CENTER = 64;    //column(X) value for center
    int8_t  const Y_CENTER = 60;    //row (Y) value for center
    int8_t  Circle1[64][2]= {
        {50,0},
        {50,5},
        {49,10},
        {48,15},
        {46,19},
        {44,24},
       {41,28},
       {38,32},
       {35,36},
       {31,39},
       {27,42},
       {23,45},
       {18,47},
       {13,48},
       {8,49},
       {4,50},
       {-1,50},
       {-6,50},
       {-11,49},
       {-16,47},
       { -21,45},
       {-25,43},
       {-29,40},
       {-33,37},
       {-37,34},
       {-40,30},
       {-43,26},
       {-45,21},
       {-47,17},
       {-49,12},
       {-49,7},
       {-50,2},
       {-50,-3},
      {-49,-8},
      {-48,-13},
      {-47,-18},
      {-45,-22},
      {-42,-26},
      {-40,-31},
      {-36,-34},
      {-33,-38},
      {-29,-41},
      {-25,-44},
      {-20,-46},
      {-15,-48},
      {-11,-49},
      {-6,-50},
      {-1,-50},
      {4,-50},
      {9,-49},
      {14,-48},
      {19,-46},
      {23,-44},
      {28,-42},
      {32,-39},
      {35,-35},
      {39,-32},
      {42,-28},
      {44,-23},
      {46,-19},
      {48,-14},
      {49,-9},
      {50,-4},
      {50,1}
   }; 
      
    
   for(i=0; i<64; i++)
   {
        x= Circle1[i][0]; //new column coordinate
        y= Circle1[i][1];   //new pagecoordinate
        
        Set_Column_Address(0,(X_CENTER+x-2),0,(X_CENTER+x+2)); //X coordinate
        Set_Page_Address(0,(Y_CENTER+y-2),0,(Y_CENTER+y+2));    //y coordinate
    //    Set_Column_Address(0,(X_CENTER+x-1),0,(X_CENTER+x+1)); //X coordinate
    //    Set_Page_Address(0,(Y_CENTER+y-1),0,(Y_CENTER+y+1));    //y coordinate
        Write_Memory_Start();
        
        for(n=0; n<9; n++) //draw a block
        {
            Data_Out(0x00);
            Data_Out(0xFF);
            Data_Out(0xFF);
        }
    } 
}


/**
    Draw a 0 digit
**/
void    Draw_Zero(void)
{
    uint16_t    n;
    
    uint8_t Zero[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    for(n=0; n<375; n++) //draw a block
    {
       if(Zero[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       }
       else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 1 digit
**/
void    Draw_One(void)
{
    uint16_t    n;
    uint8_t One[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,
    0,0,1,1,1,1,0,1,1,1,0,0,0,0,0,
    0,0,1,1,1,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
   for(n=0; n<375; n++) //draw a block
    {
       if(One[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 2 digit
**/
void    Draw_Two(void)
{
    uint16_t    n;
    uint8_t Two[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,0,0,0,0,0,0,1,1,1,0,
    0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Two[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 3 digit
**/
void    Draw_Three(void)
{
    uint16_t    n;
    uint8_t Three[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,
    0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Three[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}



/**
    Draw a 4 digit
**/
void    Draw_Four(void)
{
    uint16_t    n;
    uint8_t Four[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,
    0,0,0,0,0,1,1,1,1,0,1,1,1,0,0,
    0,0,0,0,0,1,1,1,1,0,1,1,1,0,0,
    0,0,0,0,1,1,1,1,0,0,1,1,1,0,0,
    0,0,0,0,1,1,1,1,0,0,1,1,1,0,0,
    0,0,0,1,1,1,1,0,0,0,1,1,1,0,0,
    0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,
    0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,
    0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,
    1,1,1,1,0,0,0,0,0,0,1,1,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Four[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}



/**
    Draw a 5 digit
**/
void    Draw_Five(void)
{
    uint16_t    n;
    uint8_t Five[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    0,1,1,1,0,0,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Five[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 6 digit
**/
void    Draw_Six(void)
{
    uint16_t    n;
    uint8_t Six[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,
    0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,
    0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,
    0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,
    0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,
    0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,
    0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,
    0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,
    0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Six[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 7 digit
**/
void    Draw_Seven(void)
{
    uint16_t    n;
    uint8_t Seven[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Seven[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 8 digit
**/
void    Draw_Eight(void)
{
    uint16_t    n;
    uint8_t Eight[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,
    1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,0,0,1,1,1,1,1,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Eight[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw a 9 digit
**/
void    Draw_Nine(void)
{
    uint16_t    n;
    uint8_t Nine[375]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,
    1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,
    1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,1,1,1,1,1,1,1,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,
    1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,
    0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    for(n=0; n<375; n++) //draw a block
    {
       if(Nine[n] == 1)
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } else
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
       
    } 
}


/**
    Draw BLE logo
**/
void    Draw_BLE(void)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER -40;
    y = Y_CENTER + 80;
      
    Set_Column_Address(0,(x-14),0,(x+13)); //X coordinate
    Set_Page_Address(0,(y-13),0,(y+13));    //y coordinate
    Write_Memory_Start();
    
    uint8_t BLE[756]= {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,2,2,2,2,2,1,2,2,2,2,2,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,0,0,0,0,0,0,
    0,0,0,0,0,0,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,0,0,0,0,0,
    0,0,0,0,0,2,2,2,2,2,2,2,2,2,1,2,1,1,2,2,2,2,2,2,0,0,0,0,   
    0,0,0,0,2,2,2,2,2,2,2,2,2,2,1,2,2,1,1,2,2,2,2,2,2,0,0,0,
    0,0,0,0,2,2,2,2,1,2,2,2,2,2,1,2,2,2,1,1,2,2,2,2,2,0,0,0,
    0,0,0,2,2,2,2,2,1,1,2,2,2,2,1,2,2,2,2,1,1,2,2,2,2,0,0,0,
    0,0,2,2,2,2,2,2,2,1,1,2,2,2,1,2,2,2,1,1,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,1,1,2,2,1,2,2,1,1,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,2,1,1,2,1,2,1,1,2,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2,2,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,2,2,1,1,1,2,1,1,2,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,2,1,1,2,1,2,2,1,1,2,2,2,2,2,2,2,0,0,
    0,0,2,2,2,2,2,2,2,2,1,1,2,2,1,2,2,2,1,1,2,2,2,2,2,2,0,0,
    0,0,0,2,2,2,2,2,2,1,1,2,2,2,1,2,2,2,2,1,1,2,2,2,2,0,0,0,
    0,0,0,2,2,2,2,2,1,1,2,2,2,2,1,2,2,2,1,1,2,2,2,2,2,0,0,0,
    0,0,0,0,2,2,2,2,1,2,2,2,2,2,1,2,2,1,1,2,2,2,2,2,0,0,0,0,
    0,0,0,0,2,2,2,2,2,2,2,2,2,2,1,2,1,1,2,2,2,2,2,2,0,0,0,0,
    0,0,0,0,0,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,0,0,0,0,0,0,
    0,0,0,0,0,0,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,0,0,0,0,0,0,
    0,0,0,0,0,0,0,2,2,2,2,2,2,2,1,2,2,2,2,2,2,0,0,0,0,0,0,0,   
    0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0};
     
    for(n=0; n<=756; n++) //draw a block 28 across by 27 down
    {
        if(!g_BLE_Logo)  //turn BLE logo off  
        {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
        }  
        else    //turn BLE logo on
        {
           if(BLE[n] == 1)  //white
           {
                Data_Out(0xFF);
                Data_Out(0xFF);
                Data_Out(0xFF);
           } 
           
           if(BLE[n] == 0)  //black
           {
                Data_Out(0x00);
                Data_Out(0x00);
                Data_Out(0x00);
           }
     
           if(BLE[n] == 2)  //blue
           {
                Data_Out(0x00);
                Data_Out(0x00);
                Data_Out(0xFF);
           }
   
       } 
    }
   // fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}


/**
    Draw Battery_Full,

**/
void    Draw_Full_Battery(uint8_t Batt_Id)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
      
    if(Batt_ID == HEAD)
    {   y = Y_CENTER +30;
        x=  X_CENTER;
    }  
    else    //for the handset
     {
        y = Y_CENTER + 80;
        x=  X_CENTER ;
     }       
      
    Set_Column_Address(0,(x-14),0,(x+14)); //X coordinate
    Set_Page_Address(0,(y-5),0,(y+5));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Full_Batt[319]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1, 
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
      
    for(n=0; n<=319; n++) //draw a block 31 across by 11 down
    {
       if(Full_Batt[n] == 1)  //white
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } 
       
       if(Full_Batt[n] == 0)  //black
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
 
       if(Full_Batt[n] == 3)  //grey
       {
            Data_Out(0x60);
            Data_Out(0x60);
            Data_Out(0x60);
       }

    }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}




/**
    Draw 3_4_Battery,

**/
void    Draw_Three_Quart_Battery(uint8_t Batt_Id)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
     x=  X_CENTER;
    
    if(Batt_ID== HEAD)
        y = Y_CENTER +30;
    else
        y = Y_CENTER + 80;    
      
    Set_Column_Address(0,(x-14),0,(x+14)); //X coordinate
    Set_Page_Address(0,(y-5),0,(y+5));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Three_Quart_Batt[319]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,1,1, 
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
      
    for(n=0; n<=319; n++) //draw a block 31 across by 11 down
    {
       if(Three_Quart_Batt[n] == 1)  //white
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } 
       
       if(Three_Quart_Batt[n] == 0)  //black
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
 
       if(Three_Quart_Batt[n] == 3)  //green
       {
            Data_Out(0x60);
            Data_Out(0x60);
            Data_Out(0x60);
       }

    }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}



/**
    Draw Battery_Half Full

**/
void    Draw_Half_Battery(uint8_t Batt_Id)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER ;
    
    if(Batt_ID== HEAD)
        y = Y_CENTER +30;
    else
        y = Y_CENTER + 80;    
            
      
    Set_Column_Address(0,(x-14),0,(x+14)); //X coordinate
    Set_Page_Address(0,(y-5),0,(y+5));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Half_Batt[319]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1, 
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
      
    for(n=0; n<=319; n++) //draw a block 31 across by 11 down
    {
       if(Half_Batt[n] == 1)  //white
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } 
       
       if(Half_Batt[n] == 0)  //black
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
 
       if(Half_Batt[n] == 3)  //grey
       {
            Data_Out(0x60);
            Data_Out(0x60);
            Data_Out(0x60);
       }

    }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}



/**
    Draw Battery_Quarter Full

**/
void    Draw_Quart_Battery(uint8_t Batt_Id)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER;
    
    if(Batt_ID== HEAD)
        y = Y_CENTER +30;
    else
        y = Y_CENTER + 80;    
            
      
    Set_Column_Address(0,(x-14),0,(x+14)); //X coordinate
    Set_Page_Address(0,(y-5),0,(y+5));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Quart_Batt[319]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1, 
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
      
    for(n=0; n<=319; n++) //draw a block 31 across by 11 down
    {
       if(Quart_Batt[n] == 1)  //white
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } 
       
       if(Quart_Batt[n] == 0)  //black
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
 
       if(Quart_Batt[n] == 3)  //grey
       {
            Data_Out(0x60);
            Data_Out(0x60);
            Data_Out(0x60);
       }

    }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}




/**
    Draw Battery_Empty
**/
void    Draw_Empty_Battery(uint8_t Batt_ID)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER ;
   
    if(Batt_ID== HEAD)
        y = Y_CENTER +30;
    else
        y = Y_CENTER + 80;    
            
      
    Set_Column_Address(0,(x-14),0,(x+14)); //X coordinate
    Set_Page_Address(0,(y-5),0,(y+5));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Empty_Batt[319]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1, 
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0};
      
    for(n=0; n<=319; n++) //draw a block 31 across by 11 down
    {
       if(Empty_Batt[n] == 1)  //white
       {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
       } 
       
       if(Empty_Batt[n] == 0)  //black
       {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
       }
 
       if(Empty_Batt[n] == 3)  //red
       {
            Data_Out(0xFF);
            Data_Out(0x00);
            Data_Out(0x00);
       }

    }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}




/**
    Draw Alert(void)
**/
void    Draw_Alert(int8_t Display)
{
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER + 40;
    y = Y_CENTER + 80;    
            
      
    Set_Column_Address(0,(x-12),0,(x+12)); //X coordinate
    Set_Page_Address(0,(y-12),0,(y+12));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Alert[625]= {
    
    0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,0,0,0,   
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
    0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,   
    0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,   
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};       
     
    for(n=0; n<=625; n++) //draw a block 25 across by 25 down
    {
        if(Display)
        {
            if(Alert[n] == 1)  //yellow
            {
                Data_Out(0xFF);
                Data_Out(0xFE);
                Data_Out(0x00);
            }
 
            if(Alert[n] == 0)  //black
            {
                Data_Out(0x00);
                Data_Out(0x00);
                Data_Out(0x00);
            }
        }
         else   //blank alert icon
        {
            Data_Out(0x00);
            Data_Out(0x00);
            Data_Out(0x00);
        }
            
     }
    //fprintf(DEBUG,"\n\r %u, %u", g_BLE_Logo, g_Paired);
}




/******************************************************************************
*
* FUNCTION     : DisplayTime(void)
* INPUTS       : g_TreatMinutes , g_TreatSeconds
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Plot bitmaps of integers that represent treatment time as MM:SS 
*
******************************************************************************/
void    DisplayTime(void)
{
   
    uint8_t Minute_Tens;   //treatment minutes tens value
    uint8_t Minute_Ones;   //treatment minutes ones value
    uint8_t Second_Tens;   //treatment seconds tens value
    uint8_t Second_Ones;   //treatment seconds ones value
    
    static uint8_t LastMin_Tens = 4;   //treatment minutes tens value
    static uint8_t LastMin_Ones = 0;   //treatment minutes ones value
    static uint8_t LastSec_Tens = 0;   //treatment seconds tens value
     
    Minute_Tens = g_TreatMinutes /10;  //get tens value in minutes
    Minute_Ones = g_TreatMinutes %10;   //get ones value in minutes
    Second_Tens = g_TreatSeconds /10;  //get tens value in minutes
    Second_Ones = g_TreatSeconds %10;   //get ones value in minutes
   
    //Update treatment minutes tens field if it has change
    if(LastMin_Tens != Min_Tens)
         Draw_Digit(MIN_TENS, Minute_Tens);
    
  
    //Update treatment minutes ones field it has changed
    if(LastMin_Ones != Min_Ones)
        Draw_Digit(MIN_ONES, Minute_Ones);
 
     
    //Update treatment seconds tens field it has changed
    if(LastSec_Tens != Sec_Tens)
       Draw_Digit(SEC_TENS, Second_Tens);
  
    Draw_Digit(SEC_ONES,Second_Ones);
   
    //update
    LastMin_Tens = Minute_Tens;
    LastMin_Ones = Minute_Ones;
    LastSec_Tens = Second_Tens;
 }
 
 
 /**
    Turn on all pixesl in red, green and blue then black
 
 **/
 void Clear_Display(void)
{
    uint16_t i;
    uint8_t x,y;
    
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER;
    y = Y_CENTER;    
      
    Set_Column_Address(0,(x-63),0,(x+63)); //X coordinate
    Set_Page_Address(0,(y-59),0,(y+59));    //y coordinate
    Write_Memory_Start();

     
    for(i=0; i<20480; i++)   //for each pixel
    {
        Data_Out(0x00);     //   
        Data_Out(0x00);     //   
        Data_Out(0x00);     // all black
    }
}
 
 
 /**
    Draw a white circle for electrofe headset graphic
    
**/
void    Draw_Half_Circle(void)
{
    int8_t n,i,x,y;
    int8_t  const X_CENTER = 64;    //column(X) value for center
    int8_t  const Y_CENTER = 80;    //row (Y) value for center
    int8_t  Circle1[64][2]= {
        {50,0},
        {50,5},
        {49,10},
        {48,15},
        {46,19},
        {44,24},
       {41,28},
       {38,32},
       {35,36},
       {31,39},
       {27,42},
       {23,45},
       {18,47},
       {13,48},
       {8,49},
       {4,50},
       {-1,50},
       {-6,50},
       {-11,49},
       {-16,47},
       { -21,45},
       {-25,43},
       {-29,40},
       {-33,37},
       {-37,34},
       {-40,30},
       {-43,26},
       {-45,21},
       {-47,17},
       {-49,12},
       {-49,7},
       {-50,2},
       {-50,-3},
      {-49,-8},
      {-48,-13},
      {-47,-18},
      {-45,-22},
      {-42,-26},
      {-40,-31},
      {-36,-34},
      {-33,-38},
      {-29,-41},
      {-25,-44},
      {-20,-46},
      {-15,-48},
      {-11,-49},
      {-6,-50},
      {-1,-50},
      {4,-50},
      {9,-49},
      {14,-48},
      {19,-46},
      {23,-44},
      {28,-42},
      {32,-39},
      {35,-35},
      {39,-32},
      {42,-28},
      {44,-23},
      {46,-19},
      {48,-14},
      {49,-9},
      {50,-4},
      {50,1}
   }; 
      
    
   for(i=33; i<64; i++)
   {
        x= Circle1[i][0]; //new column coordinate
        y= Circle1[i][1];   //new pagecoordinate
        
        Set_Column_Address(0,(X_CENTER+x-2),0,(X_CENTER+x+2)); //X coordinate
        Set_Page_Address(0,(Y_CENTER+y-2),0,(Y_CENTER+y+2));    //y coordinate
    //    Set_Column_Address(0,(X_CENTER+x-1),0,(X_CENTER+x+1)); //X coordinate
    //    Set_Page_Address(0,(Y_CENTER+y-1),0,(Y_CENTER+y+1));    //y coordinate
        Write_Memory_Start();
        
        for(n=0; n<9; n++) //draw a block
        {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
        }
    } 
}
 
 
  /******************************************************************************
*
* FUNCTION     : Draw Forehead(uint8_t status)
* INPUTS       : electrode status, 0=good, 1 = error
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Plot 20x10 forehead electrode graphic status 
*
******************************************************************************/
 void   Draw_Forehead(uint8_t Contact) 
 {
 
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    x=  X_CENTER;
    y = Y_CENTER -50;    
            
      
    Set_Column_Address(0,(x-14),0,(x+15)); //X coordinate
    Set_Page_Address(0,(y-7),0,(y+8));    //y coordinate
    Write_Memory_Start();
    
    uint8_t Forehead[450]= {
    
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,       
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,       
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1, 
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    for(n=0; n<=449; n++) //draw a block 20 across by 10 down
    {
        if(Forehead[n] == 1)  //white
        {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
        }
        else
        {
            if(Contact == POOR)  
            {
                Data_Out(0xFF);  //yellow
                Data_Out(0xFE);
                Data_Out(0x00);
            }
            else   //electrode is green
            {
                Data_Out(0x00);
                Data_Out(0xFF);
                Data_Out(0x00);
            }
            
        }  
    }
 
 }
 
 
 
  
  /******************************************************************************
*
* FUNCTION     : Draw <Mastoid(uint8_t status, uint8_t Side)
* INPUTS       : electrode status: 0=poor, 1=good, side: 1=left, 0=right
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Plot forehead electrode graphic status 
uint8_t const       POOR = 0;
uint8_t const       GOOD = 1;
uint8_t const       LEFT = 1;
uint8_t const       RIGHT = 0;
*
******************************************************************************/
 void   Draw_Mastoid(uint8_t Contact, uint8_t Side) 
 {
 
    uint16_t    n;
    uint8_t x,y;
   
    uint8_t const  X_CENTER = 64;    //column(X) value for center
    uint8_t const  Y_CENTER = 60;    //row (Y) value for center
    
    if(Side == LEFT)
        x=  X_CENTER + 48;
   
    if(Side == RIGHT)
       x=  X_CENTER - 48;   
       
    y = Y_CENTER + 15;
 //   fprintf(DEBUG,"\n\r %u, %u,",Contact,Side);        
      
 //   Set_Column_Address(0,(x-5),0,(x+6)); //X coordinate
//    Set_Page_Address(0,(y-5),0,(y+6));    //y coordinate
    Set_Column_Address(0,(x-6),0,(x+7)); //X coordinate
    Set_Page_Address(0,(y-6),0,(y+7));    //y coordinate
    Write_Memory_Start();
    
   // uint8_t Mastoid[144]= {
    uint8_t Mastoid[196]= {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,   
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,    
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    
    //for(n=0; n<=143; n++) //draw a block 30 across by 15 down
     for(n=0; n<=195; n++) //draw a block 30 across by 15 down
    {
        if(Mastoid[n] == 1)  //white
        {
            Data_Out(0xFF);
            Data_Out(0xFF);
            Data_Out(0xFF);
        }
        else
        {
            if(Contact == POOR)  
            {
                Data_Out(0xFF);  //yellow
                Data_Out(0xFE);
                Data_Out(0x00);
            }
            else   //electrode is green
            {
                Data_Out(0x00);
                Data_Out(0xFF);
                Data_Out(0x00);
            }
            
        }  
    }
  
 }
 
 
 
 /******************************************************************************
*
* FUNCTION     : Draw Electrodes(void)
* INPUTS       : Left, Forehead and Right Electrode status, 0=good, 1 = bad
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Plot headset electrode graphic status 
*
******************************************************************************/
void    Draw_Electrodes(void)
{

  //  Clear_Display();     //clear display
    Draw_Circle();      //Draw a circle
    
    if(bit_test(g_Contact_Status,FORE_FLG))     //if forehead contact is good
        Draw_Forehead(GOOD);     //draw forehead, green
    else
         Draw_Forehead(POOR);     //draw forehead,yellow

    if(bit_test(g_Contact_Status,LEFT_FLG))     //if left contact is good
        Draw_Mastoid(GOOD,LEFT);    //draw Left mastoid, green
    else
        Draw_Mastoid(POOR,LEFT);    //draw Left mastoid, draw forehead,yellow
         
    if(bit_test(g_Contact_Status,RIGHT_FLG))     //if right contact is good
        Draw_Mastoid(GOOD,RIGHT);    //draw Left mastoid, green
    else
        Draw_Mastoid(POOR,RIGHT);    //draw Left mastoid, draw forehead,yellow  
  //  Draw_Mastoid(POOR,RIGHT);    //draw Left mastoid,
 //   Draw_Mastoid(POOR,LEFT);    //draw Left mastoid,
 //   Draw_Colon();
//    DisplayTime();
}



 /******************************************************************************
*
* FUNCTION     : void Display_Treat_Screen(void);
* INPUTS       : none
* OUTPUTS      : none
* RETURNS      : none
* DESCRIPTION  : Set up treatment screen
*
******************************************************************************/
void Display_Treat_Screen(void)
{
    fprintf(DEBUG,"\n\r Clear");
   //  Clear_Display();
  
  //  Draw_Circle();
 //   Draw_Colon();
  //   DisplayTime();
}

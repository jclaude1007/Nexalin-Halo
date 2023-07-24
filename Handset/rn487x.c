/////////////////////////////////////////////////////////////////////
//  Custom RN487x driver for the N devices
//
//  Primary functions:

int1 ble_start_scan(void);
int1 ble_connect(int8 entry);
int1 ble_disconnect(void);

int16 ble_find_handle(char * characteristic, int8 attrib);
int1 ble_set_characteristic(int16 handle, char * outgoing, char * result);

int1 ble_get_status_resp(char * buffer, int16 time=1);

//
// See the function documentation below for deatils on the function operation.
//
// Use ble_devices[] and ble_entries to find the remote devices found.
//
// ALSO:
//    BT_MODEL_NAME should be #defined to be used to identify this module.
//
//    INT_RDA and GLOBAL interrupts must be enabled.
//
// NOTES:
//    On the sample device the device information items did not seem to
//    to work so that code was not included.
//
//    This protocol does not use the data stream so that code is not included.
/////////////////////////////////////////////////////////////////////


#define BLE_SERVICES "C0"     // Turn on device information
#define BLE_FEATURES "1000"   // Ignore devices we can not connect to

#define BLE_LAST_DEVICE  0xFF  // Connect to last device we connected to
//9524

#use rs232(UART1,baud=9600,stream=BLE,errors)

int1 ble_command_mode = FALSE;
int1 ble_scan_mode = FALSE;


#define BLE_CMD_BUFFER_SIZE  128            // Can use 64 if the baud rate is reduced

char ble_cmd_buffer[BLE_CMD_BUFFER_SIZE];   // Incomming data from the command stream
int8 ble_cmd_next_in=0;
int8 ble_cmd_next_out=0;
int1 ble_overrun = FALSE;


#define BLE_STATUS_READY 0xff
#define BLE_STATUS_READ  0xfe
char ble_status[64];                   // Last status message
int8 ble_status_ptr=BLE_STATUS_READ;


int8 ble_entries = 0;  // Count of entries (1 means one entry)

struct {               // These are the remote devices found in a scan
   char name[16];
   char address[16];
   int8 rssi;
} ble_devices[32];


/////////////////////////////////////////////////////////////////////
//
// UTILITY functions used by the primary functions follow
//
/////////////////////////////////////////////////////////////////////

int8 toascii1(char c) {
   if(c<='9')
     return(c-'0');
   else
     return(c-('A'-10));
}

int8 toascii(char * p) {
   return(toascii1(*p)*16+toascii1(*(p+1)));
}

unsigned int8 hex2int1(char digit) {
   if(digit<='9')
     return(digit-'0');
   else
     return((digit-'A')+10);
}

unsigned int16 hex2int4(char d1, char d2, char d3, char d4) {
   return( hex2int1(d1)*4096+hex2int1(d2)*256+hex2int1(d3)*16+hex2int1(d4) );
}

void ble_clear_status(void) {
   ble_status_ptr=BLE_STATUS_READ;
}

void ble_cmd_putc(char c) {
   fputc(c,BLE);
}

int1 ble_cmd_kbhit(void) {
   return(ble_cmd_next_in!=ble_cmd_next_out);
}

char ble_cmd_getc(void) {
  char c;
  while(!ble_cmd_kbhit()) ;
   c=ble_cmd_buffer[ble_cmd_next_out];
   ble_cmd_next_out=(ble_cmd_next_out+1) % sizeof(ble_cmd_buffer);
   return(c);
}

char ble_cmd_tgetc(void) {
   int8 to;
   char c;
   
   to=0;
   c=0;
   do {
      if(ble_cmd_kbhit())
         c=ble_cmd_getc();
      else
         delay_ms(10);
   } while((c==0)&&(++to<100));
   return c;
}

void ble_gets(char * str,int8 max) {
   int n,to;
   char c;
   
   to=0;
   do {
      c=ble_cmd_tgetc();
   } while((c!='$')&&(c!=0));
   if(c==0) {
     *str=0;
     return;
   }
   *str=c;
   str++;
   n=1;
   do {
      c=ble_cmd_tgetc();
      *str=c;
      str++;
      n++;
   } while((c!='\r')&&(n<max)&&(c!=0));
   *str=0;
}

void ble_clear_cmd_buffer(void) {
   ble_cmd_next_in=0;
   ble_cmd_next_out=0;
   ble_overrun=FALSE;
}

int1 wait_for_prompt(void) {
   char c;
   int16 to=0;
   
   c=' ';
   while(c!='>') {
      if(ble_cmd_kbhit()) {
         c=ble_cmd_getc();
      } else {
         delay_ms(1);
         if(++to>5000) {
            return(TRUE);
         }
      }
   }
   delay_ms(1000);
   return(FALSE);
}

void ble_get_cmd_resp(char * buffer, int16 time=1) {
   char c;
   int8 max;
   int16 to;
   
   time*=1000;
   
   to=0;
   max=128;
   c=' ';
   while(c!='\r') {
      if(ble_cmd_kbhit()) {
         c=ble_cmd_getc();
         if(c==0)
           c=' ';
         if(c>=' ') {
            *buffer=c;
            buffer++;
            if(--max==0)
              break;
         }
      }
      delay_ms(1);
      if(++to>time) {
         break;
      }
   }
   *buffer=0;
   return;
}

void ble_enter_data_mode(void) {
      ble_command_mode=FALSE;
      delay_ms(100);
      printf(ble_cmd_putc,"---\r");
}

int1 ble_enter_cmd_mode(void) {
   delay_ms(150);    
   ble_command_mode=TRUE;
   fprintf(BLE,"$$$");
   return(!wait_for_prompt());
}

int1 strcmprom(char *s1, rom char *s2)
{
   for (; *s1 == *s2; s1++, s2++)
      if (*s1 == '\0')
         return(0);
   return(1);
}

int1 ble_check_config() {
   char resp[32];

     delay_ms(250);       //jpc  originally 500
   ble_clear_cmd_buffer();
   printf(ble_cmd_putc,"GS\r");   
   ble_get_cmd_resp(resp);
   resp[2]=0;
   if(strcmprom(resp,BLE_SERVICES))
     return(TRUE);
   delay_ms(250);       //jpc  originally 500
   ble_clear_cmd_buffer();    
   printf(ble_cmd_putc,"GR\r");   
   ble_get_cmd_resp(resp);
   resp[4]=0;
   if(strcmprom(resp,BLE_FEATURES))
     return(TRUE);
   return(FALSE);
}

/////////////////////////////////////////////////////////////////////
// End of utility functions
/////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////
// This is the interrupt function that receives and parses data from 
// the RN487x.
//
// INT_RDA and GLOBAL interrupts must be enabled outside these functions.
/////////////////////////////////////////////////////////////////////
int8 scan_field,scan_idx;

#int_rda
void serial_isr() {
   static int8 scan_field,scan_idx;
   char c;
   int t;

if(bit_test(RS232_ERRORS,1))
fprintf(DEBUG,"L");
   c=fgetc(BLE);
//if(c=='g')
//fprintf(DEBUG,"%c",c);
   if((c=='%') || (scan_field!=0)) {
      if(!ble_scan_mode&&(scan_field!=5)) {
            ble_status_ptr=0;
//fprintf(DEBUG,"S");
            scan_field=5;
            return;
      }
      if(scan_field==0) {
         scan_field++;
         scan_idx=2;
//fprintf(DEBUG,"0");
    } else if (scan_field==1) {
         if(c!=',')
            ble_devices[ble_entries].address[scan_idx++]=c;
         else {
           scan_field=2;
//fprintf(DEBUG,"2");
           ble_devices[ble_entries].address[scan_idx]=0;
         }
      } else if (scan_field==2) {
         if(c!=',')
            ble_devices[ble_entries].address[0]=c;
         else { 
            ble_devices[ble_entries].address[1]=c;
            scan_field=3;
//fprintf(DEBUG,"3");
            scan_idx=0;
         }
      } else if (scan_field==3) {
         if(c==',') {
           if(scan_idx!=0) {
               ble_devices[ble_entries].name[scan_idx]=0;
               ble_entries++;
//fprintf(DEBUG,"*");
               scan_idx=0;
           }
           scan_field=4;
         } else
            ble_devices[ble_entries].name[scan_idx++]=c;
      } else if (scan_field==4) {
         if((c=='%')||(c=='\r')) {
            scan_field=0;
            scan_idx=0;
         }
      } else if (scan_field==5) {
         if(c=='%') {
            scan_field=0;
            scan_idx=0;
            ble_status[ble_status_ptr]=0;
            ble_status_ptr=BLE_STATUS_READY;
//fprintf(DEBUG,"D");
         } else
             if(ble_status_ptr!=64)
                ble_status[ble_status_ptr++]=c;
      }
   } else {
      ble_cmd_buffer[ble_cmd_next_in]=c;
      t=ble_cmd_next_in;
      ble_cmd_next_in=(ble_cmd_next_in+1) % sizeof(ble_cmd_buffer);
      if(ble_cmd_next_in==ble_cmd_next_out)
        ble_cmd_next_in=t;           // Buffer full !!
   }
}

void ble_set_scan_mode(int1 on) {
   ble_scan_mode=on;
   if(on) {
      scan_field=0;
   }
}

/////////////////////////////////////////////////////////////////////
//
// ble_get_status_resp() returns a status message from the RN487x.
// 
// Various status messages may appear at any time.  
// See Apx D of DS50002466B for a full list.
//
// Maybe the most useful is a DISCONNECT if the remote device
// stops working.
//
// time is in seconds to wait for a new message.  Pass 0 to just
// check if a message is waiting.
//
// The return value is 0 if a message is being returned.
/////////////////////////////////////////////////////////////////////

int1 ble_get_status_resp(char * buffer, int16 time=1) {
   int16 to=0;
   
   time*=500;   //jpc  was 1000
   *buffer=0;

   while((ble_status_ptr!=BLE_STATUS_READY)&&(++to<time)) {
         delay_ms(1);
   }
   if(ble_status_ptr==BLE_STATUS_READY) {
      strcpy(buffer,ble_status);
      ble_status_ptr=BLE_STATUS_READ;
      return(FALSE);
   } else
      return(TRUE);
}



/////////////////////////////////////////////////////////////////////
//
// ble_start_scan() starts scanning for remote devices.  The devices
// are filled into the ble_devices table.  The function returns
// right away because it could take a long time for the table to fill.
// 
// Note that if the RN487x is not set up right this function will reset
// the module into the correct mode.
//
// BT_MODEL_NAME should be #defined for the string to be used to identify
// this module.
//
// The return value is 0 if successful.
/////////////////////////////////////////////////////////////////////

int1 ble_start_scan(void) {
   int1 err;

   output_low(RST_BLE);  // Reset module
   delay_ms(10);    
   output_high(RST_BLE);
 
   if(!ble_enter_cmd_mode()) {
      set_uart_speed(115200,BLE);
      delay_ms(10);
      printf(ble_cmd_putc,"$$$");
      delay_ms(10);
      printf(ble_cmd_putc,"SB,09\r");
      delay_ms(100);
      set_uart_speed(9600,BLE);
      output_low(RST_BLE);  // Reset module
      delay_ms(10);    
      output_high(RST_BLE);
      ble_enter_cmd_mode();
fprintf(DEBUG,"\r\CHANGING BAUD\r\n");
   }

   err=ble_check_config();
   if(err)
      err=ble_check_config();
// err=FALSE;
    if(err) {
      printf(ble_cmd_putc,"SS," BLE_SERVICES "\r");  
      wait_for_prompt();
      printf(ble_cmd_putc,"SR,1000\r");  
      wait_for_prompt();
      printf(ble_cmd_putc,"PZ\r");  // Clear Services
      wait_for_prompt();
      printf(ble_cmd_putc,"S-," BT_MODEL_NAME "\r");
      wait_for_prompt();
 //     printf(ble_cmd_putc,"R,1\r");      // Reset
      delay_ms(500);
      ble_enter_cmd_mode();
   }   
        
   printf(ble_cmd_putc,"GS\r");   
   err=wait_for_prompt();
   ble_entries=0;
   ble_clear_cmd_buffer(); 
   ble_set_scan_mode(TRUE);
   if(!err)
      printf(ble_cmd_putc,"F\r");   // Start scanning
//      printf(ble_cmd_putc,"F,%4X,%4X\r",BLE_SCAN_INTERVAL,BLE_SCAN_WINDOW);   // Start scanning
   return(err);
}

/////////////////////////////////////////////////////////////////////
//
// ble_rescan() Assumes you are now scanning and will stop the scan 
// and restart it.
//
// The return value is 0 if successful.
/////////////////////////////////////////////////////////////////////

int1 ble_rescan(void) {
   
   printf(ble_cmd_putc,"X\r");
   delay_ms(100);
   ble_set_scan_mode(FALSE);
   ble_clear_status();
   wait_for_prompt();
   ble_entries=0;
   ble_clear_cmd_buffer(); 
   ble_set_scan_mode(TRUE);
   printf(ble_cmd_putc,"F\r");   // Start scanning
//   printf(ble_cmd_putc,"F,%4X,%4X\r",BLE_SCAN_INTERVAL,BLE_SCAN_WINDOW);   // Start scanning
   return( 0);
}


/////////////////////////////////////////////////////////////////////
//
// ble_connect() connects to a remote device.  The device is specified
// by the index (first entery 0) in the ble_devices table.  After
// connecting this device is put in the client mode.
//
// The return value is 0 if successful.
/////////////////////////////////////////////////////////////////////

int1 ble_connect(int8 entry) {
   char resp[64];
   
   printf(ble_cmd_putc,"X\r");
   delay_ms(500);      //jpc  was 1000
   ble_set_scan_mode(FALSE);
   ble_clear_status();
   if(entry==BLE_LAST_DEVICE)
      printf(ble_cmd_putc,"C\r");
   else
      printf(ble_cmd_putc,"C,%s\r",ble_devices[entry].address);
   do {
      if(ble_get_status_resp(resp,10))
        return(TRUE);      
      if((resp[0]=='C')&&(resp[1]=='O')&&(resp[4]=='E')) {  
         delay_ms(500);      //jpc  was 1000
    //    fprintf(BLE,"B\r");
    //    wait_for_prompt();
        printf(ble_cmd_putc,"I\r");
       // wait_for_prompt();
        return(FALSE);
      }
   } while(TRUE);
}


/////////////////////////////////////////////////////////////////////
//
// ble_disconnect() simply disconnects froma remote device
//
// The return value is 1 if the RN487x does not respond.
/////////////////////////////////////////////////////////////////////

int1 ble_disconnect(void) {
   fprintf(DEBUG,"Disconnected\r\n");  
   printf(ble_cmd_putc,"K,1\r");
   return(wait_for_prompt());
}


/////////////////////////////////////////////////////////////////////
//
// ble_find_handle() searches for a specific characteristic with a
// specific attribute on the remote device.
//
// Note this version assumes a constant string for the characteristic.
//
// The return value is 0 if there was no match.
/////////////////////////////////////////////////////////////////////

int16 ble_find_handle(char * characteristic, int8 attrib) {
   char * rp;
   int8 i;
   char line[128];
   char attrib_txt[3];
   int8 retries;
  
   ble_clear_cmd_buffer();
   sprintf(attrib_txt,"%02X",attrib);
   retries=0;
   do {
      printf(ble_cmd_putc,"LC\r");
      do {
         ble_get_cmd_resp(line,2);
         if(line[0]==0) {
            continue;
         }
         i=0;
         while((line[i]!=0)&&(line[i]<' ')) i++;
         if(line[i]!=' ') {
           continue;
         }  
         rp=characteristic;
         while((line[i]!=0)&&(line[i]<=' ')) i++;
         if((line[i]=='E')&&(line[i+1]=='N')) {
            break;
         }
         if(line[i]==0) {
              continue;
         }
         while((line[i]!=',')&&(*rp==line[i])) {
            i++;
            rp++;
         }
         if(line[i]==',') {
            while((line[++i]!=',')&&(line[++i]!=0)) ;
            if((line[i+1]==attrib_txt[0])&&(line[i+2]==attrib_txt[1])) {
               return(hex2int4(line[i-4],line[i-3],line[i-2],line[i-1]));
            }
         }
      } while(TRUE);
      if(++retries==1) {
         ble_clear_cmd_buffer();
         printf(ble_cmd_putc,"CI\r");
         wait_for_prompt();
      }
   } while(retries<2);
   return(0);
}


/////////////////////////////////////////////////////////////////////
//
// ble_set_characteristic() writes to a characteristic on the remote
// server device.  It then grabs the notify response, converts it to 
// an ASCII string and returns it in result.  
//
//  Use ble_find_handle() to get the handle for a characteristic.
//
//  The return value is 0 if there were no errors.
/////////////////////////////////////////////////////////////////////

int1 ble_set_characteristic(int16 handle, char * outgoing, char * result) {
   char buffer[64]; 
   char * p;
   int8 i;
   
   ble_clear_cmd_buffer();
   p=buffer;
   i=0;
   while(outgoing[i]!=0) {
      sprintf(p,"%02X",outgoing[i]);
      i++;
      p+=2;
   } 
   sprintf(p,"00");
      
   printf(ble_cmd_putc,"CHW,%04X,%s\r",handle,buffer);
   do {
      ble_get_status_resp(buffer,20);
      if(buffer[0]==0) {
         *result=0;
         return(FALSE);      
      }
      if((buffer[0]=='N')&&(buffer[1]=='O')) {
         i=10;
         while(buffer[i]!=0) {
            *result=toascii(&buffer+i);
            i+=2;
            result++;
         }
         break;
      }
   }while(TRUE);
   return(TRUE);
}


int1 ble_get_characteristic(int16 handle, char * result) {
   char buffer[64]; 
   int8 i,retry;

   retry=0;
   do {
      ble_clear_cmd_buffer();
      printf(ble_cmd_putc,"CHR,%04X\r",handle);
      ble_get_cmd_resp(buffer,5);
      if(buffer[0]=='E') {
        i=0;
      } else {
         ble_get_cmd_resp(buffer,5);
         if(buffer[0]==0) {
            *result=0;
            return(FALSE);      
         }
         if(buffer[0]=='C')
           i=5;
         else
           i=0;
         while(buffer[i]!=0) {
            *result=buffer[i];
            i++;
            result++;
         }
         *result=0;
      }
      if(i>6)
         return(TRUE);
       delay_ms(500);      //jpc  was 1000
   } while(++retry<3);
   return(FALSE);
}

int16 ble_start(void) {
   int8 last_entry;
   int16 handle;
   int1 connected;
   
    connected=FALSE;
    if(ble_start_scan())
         fprintf(DEBUG,"Error - Scan\r\n");
    last_entry=0;           
    while(!connected) {
       if(ble_entries!=last_entry) {
          //fprintf(DEBUG,"Found #%u %s\r\n",last_entry+1,ble_devices[last_entry].name);
          if(strcmp(ble_devices[last_entry].name,BT_REMOTE_NAME )==0) {
             if(ble_connect(last_entry)) {
                fprintf(DEBUG,"Error Connecting\r\n");
             } else {
                fprintf(DEBUG,"Connected\r\n");  
                connected=TRUE;
               // handle=ble_find_handle(BT_CHARACTERISTIC_1);
               // fprintf(DEBUG,"\r\nHandle=%04X\r\n",handle); 
               // delay_ms(2000);
             }
            }
          last_entry++;
       }
    }
   if(!connected)
      handle=0;
   return handle;
}


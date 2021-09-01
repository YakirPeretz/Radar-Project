//#include  <msp430xG46x.h> // IDE library
#include <msp430.h>
#include  "bspGPIO.h"    // private library
#include  "halGPIO.h"    // private library
#include <stdlib.h>
#include <string.h>

volatile unsigned int State; // global variable
volatile unsigned int Option3State;
unsigned int EdgeRise;
unsigned int EdgeFall;
volatile unsigned int CapFlag;
volatile unsigned int index;
unsigned int Range[5];
unsigned int Angle;
volatile double InputAngle;
volatile int sendfinalAng;
char SendAng[6];
char SendRange[7];
unsigned int Delay;
unsigned int DelayScript;
char InAngStr[8];
unsigned int num_of_files;
char name_of_files[3][9];
char *start_of_file[3];
char size_of_file[3][4];
char File_Name[9];
char *Flash_p;
char ope1[4];

void MoveServo(unsigned int Delay,int* direction,double StartAng,double FinalAng);
void RGBblink();
void LedShiftLeft();
void LedShiftRight();
void DelayUs(unsigned int cnt);
void DelayMs(unsigned int cnt);
void int2str(char *str, unsigned int num);
//double str2double(char *s);
void insertionSort(unsigned int arr[], int n);
void SampMove(int *direction, double StartAng,double FinalAng);
void TimersInit(double angle);

int main(void)
{
  int cmpstring,operand1,direction;
  unsigned int i;
  double StartAng,FinalAng;
  char opc[3];
  char *eptr;
  InitGPIO();
  initUart();
  // initialize
  Flash_p =(char *) 0xFC00;
  start_of_file[0]=Flash_p;
  num_of_files=0;
  State=4; // Start with get the scripts to the Flash
  Delay=4; // servo Delay
  Option3State=0; // Flash mode State
  CapFlag=0; // Capture Flag
  DelayScript=500; // Default script Delay
  IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
  while(1)
  { 
    switch(State)              // FSM
    {
    case 0: // Sleep Mode
        _BIS_SR(CPUOFF+ GIE);  // Enter LPM0
        __no_operation();
        break;
    case 1:
            StartAng=0.0; // set start angle to 0
            FinalAng=180.0; // set final angle to 180
            index=0;
            direction=0; // initialize direction
            CapFlag=0;
             // Timer Initialize
             TimersInit(0.0); // initialize Timers( for servo and ultra sonic)
             while(State==1)
             {
               SampMove(&direction,StartAng,FinalAng); // sample the echo and move the servo
             }
             TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer (UltraSonic)
             TA0CTL = TASSEL_2+MC_0+TACLR; //halt timer (Servo)
             IE2 &= ~UCA0TXIE; // disable TX interrupt
        break;// case 1

    case 2:
             _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
             __no_operation();
             InputAngle=strtod(InAngStr,&eptr); // input angle- string to double
              // Timer Initialize
             i=0;
             CapFlag=0;
             TimersInit(InputAngle); // initialize Timers( for servo and ultra sonic)
             Angle=TA0CCR1;
             while(State==2)
             {
                 _BIS_SR(CPUOFF+ GIE);  // Enter LPM0
                 __no_operation();   // for debug
                 TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer (ultrasonic)
                 Range[i++]=EdgeFall-EdgeRise; // save the range
                 if(i==5) // every 5 samples
                 {
                   insertionSort(Range,5); // sort the Range we measured
                   int2str(SendRange,Range[2]); // send the median
                   UCA0TXBUF = SendRange[index];
                   IE2 |= UCA0TXIE;              // Enable USCI_A0 TX interrupt
                   index++;
                   _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
                   __no_operation();
                   i=0;
                 }
                 TA1CTL|=MC_1+TACLR; //upmode mode
             }
             TA0CTL = TASSEL_2+MC_0+TACLR; //halt timer (Servo)
             TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer (UltraSonic)
                break;
  
    case 3: 
             _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
             __no_operation();
             cmpstring=strcmp(File_Name,name_of_files[1]);
             // check which file to run
             if(!cmpstring)
               Flash_p =(char *)start_of_file[1];
             else if (cmpstring>0)
               Flash_p =(char *)start_of_file[2];
             else
               Flash_p =(char *)start_of_file[0];
             _BIS_SR(GIE);                // Enable Global interrupt
             DelayMs(2000);
            while(State==3)
            {
             switch(Option3State)
             {
              case 0:  // read the OpCode
                    opc[0]=*Flash_p++;
                    if(opc[0] == '\n') // end of Script
                    {
                      UCA0TXBUF= 'C'; // Send ack to the CPU side that we finish run a script
                     DelayMs(10);
                     State=0;
                     break;
                    }
                    opc[1]=*Flash_p++;
                    Option3State=(int)strtol(opc,NULL,16); // convert the Opcode to integer
                  __no_operation(); 
                break;
               
              case 1:
                    // insert next 2 bytes to Operand
                      ope1[0]=*Flash_p++;
                      ope1[1]=*Flash_p++;
                      operand1=(int)strtol(ope1,NULL,16); // convert operand to int
                      for(i=operand1;i>0;i--)
                       {
                        if(State==3)
                         RGBblink(); // Blink RGB operand1 times
                       }
                       Option3State=0; // read next Opcode
                 break;
                 
             case 2: 
                   // insert next 2  bytes to Operand
                   ope1[0]=*Flash_p++;
                   ope1[1]=*Flash_p++;
                   operand1=(int)strtol(ope1,NULL,16); // convert operand to int
                   for(i=operand1;i>0;i--)
                   {
                     if(State==3)
                      LedShiftLeft(); // rlc LEDs operand1 times
                   }
                    Option3State=0;
                   break;
                   
             case 3:
                   // insert next 2 bytes to Operand
                   ope1[0]=*Flash_p++;
                   ope1[1]=*Flash_p++;
                   operand1=(int)strtol(ope1,NULL,16); // convert operand to int
                   for(i=operand1;i>0;i--)
                       {
                        if(State==3)
                         LedShiftRight(); //rrc LEDs operand 1 times
                       }
                    Option3State=0;
                   break;
                   
             case 4:
                   // read next 2 bytes- delay Time
                   ope1[0]=*Flash_p++;
                   ope1[1]=*Flash_p++;
                   operand1=(int)strtol(ope1,NULL,16); // convert operand to int
                   DelayScript=operand1*10; // update Delay
                   Option3State=0;
                   break;
                   
             case 5: // Turn off Leds and RGB
                   Leds_Uart_Out&=0x06;
                   Leds_RGB_Sig_Out&=0x54;
                   Option3State=0;
               break;
               
             case 6:
                    UCA0TXBUF= 'Q'; //Ack to CPU side for servo deg mode
                     DelayMs(10);
                     CapFlag=0;
                   //read 2 bytes and insert it to InputAngle
                    ope1[0]=*Flash_p++;
                    ope1[1]=*Flash_p++;
                    InputAngle=(double)strtol(ope1,NULL,16); // convert operand to double - the angle to move the servo
                    // Timer Initialize
                     TimersInit(InputAngle); // initialize timers
                     Angle=TA0CCR1;
                     i=0;
                     // check for halt timer and all other shit that we add
                     for(i=5;i>0;i--)
                     {
                       _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
                         __no_operation();                     // for debug
                         Range[i]=EdgeFall-EdgeRise;
                      }
                     int2str(SendAng,Angle);
                     insertionSort(Range,5); // sort the Ranges
                     int2str(SendRange,Range[2]); // send the median
                     UCA0TXBUF = SendAng[index];
                     IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
                     index++;
                     _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
                     __no_operation();
                     DelayMs(2);
                     UCA0TXBUF = SendRange[index];
                     IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
                     index++;
                     _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
                     __no_operation();
                     TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer
                     TA0CTL = TASSEL_2+MC_0+TACLR; //smclk clk,upmode mode
                     i=0;
                     DelayMs(2500);
                     Option3State=0;
                   break;
               
             case 7:
                    UCA0TXBUF= 'W'; // Ack to CPU side for servo_scan mode
                     DelayMs(10);
                     CapFlag=0;
                     sendfinalAng=0;
                    //read 2 bytes- insert to StartAngle
                     ope1[0]=*Flash_p++;
                     ope1[1]=*Flash_p++;
                     StartAng= (double)strtol(ope1,NULL,16);
                    //read 2 bytes- insert to Final Angle
                     ope1[0]=*Flash_p++;
                     ope1[1]=*Flash_p++;
                     FinalAng= (double)strtol(ope1,NULL,16);
                     ope1[2]='\n';
                     UCA0TXBUF = ope1[index];
                     IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
                     index++;
                     _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
                     __no_operation();
                     DelayMs(2000);
                     direction=0;
                     index=0;
                     // Timer Initialize
                     TimersInit(StartAng);
                     while(!direction && State==3)
                     {
                       SampMove(&direction,StartAng,FinalAng); // sample the echo and scan between the Start to Final angles
                     }
                   TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer ( Ultrasonic)
                   TA0CTL = TASSEL_2+MC_0+TACLR; //halt timer (Servo)
                   Option3State=0;
                   break;
               
             case 8:
                   UCA0TXBUF= 'C'; // Ack to CPU side that we enter to sleep mode
                   DelayMs(10);
                   State=0; //
                   Option3State=0;
                   DelayScript=500;
                   break;
             }//switch
            }//while state3
            
        break; // case state 3
        
    case 4: // Flash Write
            FCTL1=FWKEY+ERASE;
            FCTL3 = FWKEY;           // Clear Lock bit
            *Flash_p = 0;                           // Dummy write to erase Flash segment
            FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
            _BIS_SR(CPUOFF+ GIE);    // Enter LPM0
            __no_operation();       // for debug
            FCTL1 = FWKEY;//Clear WRT bit
            FCTL3 = FWKEY + LOCK;// Set LOCK bit
            State = 0;
      break;
    }
  }
}

void MoveServo(unsigned int Delay,int* direction,double StartAng,double FinalAng)
{
         DelayMs(Delay); // 4Ms Delay for Servo move
      
        if (TA0CCR1<=400+(int)(10.87*StartAng))
           {
               *direction=0; // if we reach to 0 angle change rotate direction
           }
        else if(TA0CCR1>400+(int)(10.87*FinalAng)-6)
        {
            *direction=1; // if we reach to 180 angle change rotate direction
        }
         if(!(*direction))
         {
             TA0CCR1+=10; // move counter clock wise in 0.92 degree
         }
         else
         {
             TA0CCR1-=10; // move clock wise in 0.92 degree
         }
}

void RGBblink()
{
    Leds_RGB_Sig_Out&=0x57;
    Leds_RGB_Sig_Out |=0x08; // turn on blue
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out ^=0x28; // turn on green
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out |=0x28; // turn on blue+green
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out ^=0xA8; // turn on red
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out |=0x08; // turn on blue+red
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out ^=0x28; // turn on red+green
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out |=0x08; // turn on blue+red+green
    DelayMs(DelayScript); //wait for Delay MS
}

void LedShiftLeft()
{

    Leds_RGB_Sig_Out&=0xFC; // Turn off Leds 7-8(from Port 2)
    Leds_Uart_Out |= 0x01; // turn on first led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0x09; // turn on 2 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0x18; // turn on 3 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0x30; // turn on 4 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0x60; // turn on 5 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0xC0; // turn on 6 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_Uart_Out ^= 0x80; // turn off 6 led from the right
    Leds_RGB_Sig_Out |=0x01; // turn on 7 led from the right
    DelayMs(DelayScript); //wait for Delay MS
    Leds_RGB_Sig_Out ^=0x03; // turn on 8 led from the right
    DelayMs(DelayScript); //wait for Delay MS
}

void LedShiftRight()
{
  Leds_Uart_Out&=0x06; // turn off leds 1-6(Port 1)
  Leds_RGB_Sig_Out|=0x02; // turn on 8 led from the right
  DelayMs(DelayScript); //wait for Delay MS
  Leds_RGB_Sig_Out ^=0x03; // turn on 7 led from the right
  DelayMs(DelayScript); //wait for Delay MS
  Leds_RGB_Sig_Out ^=0x01; // turn off 7 led from the right
  Leds_Uart_Out ^= 0x80; // turn on 6 led from the left
  DelayMs(DelayScript); //wait for Delay MS
  Leds_Uart_Out ^= 0xC0; // turn on 5 led from the right
  DelayMs(DelayScript); //wait for Delay MS
  Leds_Uart_Out ^= 0x60; // turn on 4 led from the right
  DelayMs(DelayScript); //wait for Delay MS
  Leds_Uart_Out ^= 0x30; // turn on 3 led from the right
  DelayMs(DelayScript); //wait for Delay MS
  Leds_Uart_Out ^= 0x18; // turn on 2 led from the right
  DelayMs(DelayScript); //wait for Delay MS 
  Leds_Uart_Out ^= 0x09; // turn on 1 led from the right
  DelayMs(DelayScript); //wait for Delay MS
}

//******************************************************************
// Delay usec functions
//******************************************************************
void DelayUs(unsigned int cnt){
    unsigned char i;
        for(i=cnt ; i>0 ; i--) 
          __no_operation();// tha command asm("nop") takes raphly 1usec
}

//******************************************************************
// Delay msec functions
//******************************************************************
void DelayMs(unsigned int cnt){
    unsigned int i;
        for(i=cnt ; i>0 ; i--) 
          DelayUs(1000); // tha command asm("nop") takes raphly 1usec
    
}

// Int to String Function
void int2str(char *str, unsigned int num)
{
    int j,strSize = 0;
    long tmp = num, len = 0;
    // Find the size of the intPart by repeatedly dividing by 10
    while(tmp){
        len++;
        tmp /= 10;
    }
    // Print out the numbers in reverse
    for(j = len - 1; j >= 0; j--){
        str[j] = (num % 10) + '0';
        num /= 10;
    }
    strSize += len;
    str[strSize] = '\n';
}
// Insertion sort function
void insertionSort(unsigned int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;
 
        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}
 
void SampMove(int *direction, double StartAng,double FinalAng)
{
    unsigned int m;
    for(m=0;m<3;m++)
    {
    _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
   __no_operation();                     // for debug
   TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer
   Range[m]=EdgeFall-EdgeRise;
   TA1CTL|=MC_1+TACLR; //upmode mode
    }
    TA1CTL= TASSEL_2+MC_0+TACLR; //halt timer
    insertionSort(Range,3); // sort the Range we measured
   Angle=TA0CCR1;
   //UART send degree and range
   int2str(SendAng,Angle);
   int2str(SendRange,Range[1]);
   UCA0TXBUF = SendAng[index];  // send angle
   IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
   index++;
   _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
   __no_operation();
   DelayMs(2);
   UCA0TXBUF = SendRange[index]; // send Range
   IE2 |= UCA0TXIE;                          // Enable USCI_A0 TX interrupt
   index++;
   _BIS_SR(CPUOFF+ GIE);                // Enter LPM0
   __no_operation();
   MoveServo(Delay,direction,StartAng,FinalAng);
   TA1CTL|=MC_1+TACLR; //upmode mode
}

void TimersInit(double angle)
{
   // Servo engine PWM
    TA0CTL = TASSEL_2+MC_1+TACLR; //smclk clk,upmode mode - Servo Timer
    TA0CCTL1|=OUTMOD_7; // reset/set
    //UltraSonic Trig
    TA1CTL= TASSEL_2+MC_1+TACLR; //smclk clk,upmode mode - Ultrasonic timer
    TA1CCTL1|=OUTMOD_7; // reset/set
   TA0CCR0=25000; // 40 Hz
   TA0CCR1=400+(int)(10.87*angle); // calc the Duty cycle needed for the input angle
   DelayMs(1000); // Delay for the Servo to reach the correct angle
   //UltraSonic Trig
   TA1CCR0=60260; // 16.6Hz
   TA1CCR1=20; // 20 us high
   // UltraSonic echo
   TA1CCTL2 =CM_3+SCS+CAP+CCIE+CCIS_0; // TACCR2 interrupt enabled,Synchronious, capture mode,rising and falling edge
}

//#include  <msp430xG46x.h> // IDE library
#include <msp430.h>
#include  "bspGPIO.h"    // private library
#include  "halGPIO.h"    // private library


void InitGPIO(void)
{
  volatile unsigned int i;
  
  WDTCTL = WDTPW + WDTHOLD;		// Stop WDT
  if (CALBC1_1MHZ==0xFF)       // If calibration constant erased
   {
      while(1);  // do not load, trap CPU!!
    }
    DCOCTL = 0;                           // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                // Set DCO
    DCOCTL = CALDCO_1MHZ;
    FCTL2 = FWKEY + FSSEL0 + FN1; // MCLK/3 for Flash Timing Generator
                
    Leds_Uart_Sel=0x06;     // P1.0-Led 0 +P1.3-P1.7 Led 1-5 GPIO, P1.1-RX Uart,P1.2-TX Uart
    Leds_Uart_Dir=0xFF;     // P1.0-Led 0 +P1.3-P1.7 Led 1-5 Output mode, P1.1-RX Uart,P1.2-TX Uart
    Leds_Uart_Ped=0x00;     // clear interrupt pending
    Leds_Uart_Out=0x00;     // Turn off LEDS
    Leds_RGB_Sig_Sel=0x54;  //P2.0+P2.1-LEDS as GPIO, P2.3+P2.5+P2.7- RGP as GPIO, P2.2-TA1.1,P2.4-TA1.2,P2.6-TA0.1
    Leds_RGB_Sig_Dir=0xEF;  //P2.0+P2.1-LEDS as Output, P2.3+P2.5+P2.7- RGP as Output, P2.2-TA1.1,P2.4-TA1.2,P2.6-TA0.1
    Leds_RGB_Sig_Ped=0x00;  // clear interrupt pending
    Leds_RGB_Sig_Out=0x00;   // Turn off LEDS and RGB
    P1SEL = BIT1 + BIT2 ;    // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2 ;  // P1.1 = RXD, P1.2=TXD

}

// UART initialize
void initUart(void)
{
    UCA0CTL1 |= UCSWRST;                     // **Initialize USCI state machine**
    UCA0CTL1 |= UCSSEL_2;                     // CLK = SMCLK
    UCA0BR0 = 104;                           // bound rate 9600
    UCA0BR1 = 0x00;                           
    UCA0MCTL = UCBRS_0;      // Modulation UCBRSx = 0
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

#ifndef _bspGPIO_H_
#define _bspGPIO_H_

#define Leds_Uart_Ped		P1IFG
#define Leds_Uart_Sel   	P1SEL
#define Leds_Uart_Dir  		P1DIR
#define Leds_Uart_Out       P1OUT
#define Leds_RGB_Sig_Sel    P2SEL
#define Leds_RGB_Sig_Dir    P2DIR
#define Leds_RGB_Sig_Out    P2OUT
#define Leds_RGB_Sig_Ped    P2IFG

extern void InitGPIO(void);
extern void initUart(void);

#endif

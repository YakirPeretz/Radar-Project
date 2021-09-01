#ifndef _halGPIO_H_
#define _halGPIO_H_


extern  volatile unsigned int State;
extern  unsigned int EdgeRise;
extern  unsigned int EdgeFall;
extern  volatile unsigned int CapFlag;
extern  unsigned int Range[5];
extern  unsigned int Angle;
extern  char InAngStr[8];
extern  volatile unsigned int index;
extern  volatile double InputAngle;
extern  char SendAng[6];
extern  char SendRange[7];
extern unsigned int num_of_files;
extern char name_of_files[3][9];
extern char *start_of_file[3];
extern char size_of_file[3][4];
extern char *Flash_p;
extern char File_Name[9];
extern volatile unsigned int Option3State;
extern char ope1[4];
extern volatile int sendfinalAng;
extern __interrupt void Timer_A1 (void);
extern __interrupt void USCI0TX_ISR(void);
extern __interrupt void USCI0RX_ISR(void);
#endif


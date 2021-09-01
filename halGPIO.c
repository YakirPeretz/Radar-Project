//#include  <msp430xG46x.h> // IDE library
#include <msp430.h>
#include  "bspGPIO.h"    // private library
#include  "halGPIO.h"    // private library

unsigned int k=0;
void initUart(void);
int AngRangeFlag=0;
int StateFlash=0;
int angorstate=0;
int fileorstate=0;

void ChangeState();
//******************************************************************
//  Timer A1 Interrupt Service Rotine
//******************************************************************
#pragma vector=TIMER1_A1_VECTOR
__interrupt void Timer_A1 (void)
{
    switch( TA1IV )
      {
      case 0: break;
      case 2: break;
      case 4:
                if (!CapFlag) // check if it's the first or second capture
                {
                    EdgeRise = TA1CCR2; // capture the rising edge
                    CapFlag++;
                }
                else
                {
                    EdgeFall = TA1CCR2; //capture the falling edge
                    CapFlag=0;
                    __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
                }
        break;
      case 10: break;
      }
}

//******************************************************************
//  UART TX Interrupt Service Rotine
//******************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(State)
  {
  case 0:
      IE2 &= ~UCA0TXIE; //disable interrupt from TX
      __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      break;
  case 1:
    if(!AngRangeFlag) // check for sending angle or Range
    {
      if (SendAng[index-1] == '\n')        // check for last char in angle was send
      {
        AngRangeFlag=1;
        index=0;
         IE2 &= ~UCA0TXIE;   //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
       } // if send anglee done
      else
      {
         UCA0TXBUF = SendAng[index];   // TX next character
         index++; 
      }//else
    }//if Ang-Range Flag
    else
    {
       if (SendRange[index-1] == '\n')       // check for last char in Range was send
      {
        AngRangeFlag=0;
        index=0;
        IE2 &= ~UCA0TXIE;   //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if finish send range
      else
      {
        UCA0TXBUF = SendRange[index];   // TX next character
        index++; 
      } // else 
    }// else Ang-Range Flag 
    break;//case 1

  case 2: 
     if (SendRange[index-1] == '\n')      // check for last char in Range was send
      {
        index=0;
        IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      }
      else
      {
        UCA0TXBUF = SendRange[index];   // TX next character
        index++; 
      }
    break; //case 2

  case 3:
    switch (Option3State) // check for modes 6 and 7 at Script mode
    {
    case 6:
    if(!AngRangeFlag) // check for sending angle or Range
    {
      if (SendAng[index-1] == '\n')     // check for last char in Angle was send
      {
        AngRangeFlag=1;
        index=0;
         IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if send anglee done
      else
      {
         UCA0TXBUF = SendAng[index];   // TX next character
         index++; 
      }//else
    }//if Ang-Range Flag
    else
    {
       if (SendRange[index-1] == '\n')      // check for last char in Range was send
      {
        AngRangeFlag=0;
        index=0;
        IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if finish send range
      else
      {
        UCA0TXBUF = SendRange[index];   // TX next character
        index++; 
      } // else 
    }// else Ang-Range Flag ();
    break; //case 6 option3 state
    
    case 7:
      if(!sendfinalAng) // check for sending final or angle/Range
      {
        if (ope1[index-1] == '\n')        // check for last char in final Angle was send
      {
        sendfinalAng=1;
        index=0;
        IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if finish send range
      else
      {
        UCA0TXBUF = ope1[index];   // TX next character
        index++; 
      } // else 
       
      }//if FinalAng
      else
      {
        if(!AngRangeFlag)
    {
      if (SendAng[index-1] == '\n')              // TX over?
      {
        AngRangeFlag=1;
        index=0;
         IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if send anglee done
      else
      {
         UCA0TXBUF = SendAng[index];   // TX next character
         index++; 
      }//else
    }//if Ang-Range Flag
    else
    {
       if (SendRange[index-1] == '\n')              // TX over?
      {
        AngRangeFlag=0;
        index=0;
        IE2 &= ~UCA0TXIE;    //disable interrupt from TX
        __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
      } // if finish send range
      else
      {
        UCA0TXBUF = SendRange[index];   // TX next character
        index++; 
      } // else 
    }// else Ang-Range Flag ();
        
      }
      
      break; // case 7
    
    }
    break;//case 3
  }//Switch state
  
}
//******************************************************************
//  UART RX Interrupt Service Rotine
//******************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch (State)
  {
  case 0: 
     ChangeState(); // change state
     IE2 &= ~UCA0TXIE; //disable interrupt from TX
     __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
    break; // break case 0
   
  case 1:
       ChangeState(); // change state
       IE2 &= ~UCA0TXIE; //disable interrupt from TX
     __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
      break; //case 1
  
  case 2:
    if(!angorstate) // check for input angle or changing the state
    {
        InAngStr[k]=UCA0RXBUF;
        k++;
        if(InAngStr[k-1]=='\n')  // check for last char in input angle  was received
        {
          k=0;
          angorstate=1;
          __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
        }
    } // angle or state
    else
    {
        ChangeState(); // change state
      __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
      angorstate=0;
    } // else
    break; // break case 2

  case 3: 
      if(!fileorstate) // check for file name or changing the state
      {
        // read the script name
        File_Name[k]=UCA0RXBUF;
        k++;
        if(File_Name[k-1]=='\n') // check for last char in file name was received
         {
           k=0;
           fileorstate=1;
           __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
         }
      }
      else
      {
          ChangeState(); // change state
          fileorstate=0;
          __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
      } // else
    break;// case 3
    
  case 4:
     switch(StateFlash) // check for which information we receive
     {
     case 0:
         name_of_files[num_of_files][k++]=UCA0RXBUF;
       if(name_of_files[num_of_files][k-1]=='\n') // check if the last char in the file name was received
       {
        StateFlash=1; // change the state for size receive
        k=0;
       }
       break;
     case 1:
       size_of_file[num_of_files][k++]=UCA0RXBUF;
       if(size_of_file[num_of_files][k-1]=='\n') // check if the last char in the file size was received
       {
        StateFlash=2; // change the state for script receive
        k=0;
       }
       break;
      
     case 2:
      if(UCA0RXBUF=='\n') // check if the last char in the script was received
       {
        StateFlash=0; 
        k=0;
        num_of_files++;
        *Flash_p++ = UCA0RXBUF; // write to flash the script
        if (num_of_files!=3)
          start_of_file[num_of_files]=Flash_p; // save the memory location that the pointer points at
        else
          __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0
       }//if
      else
      {
       *Flash_p++ = UCA0RXBUF; // write to flash the script
      }// else
     }// switch state flash
    break; // case 4 
  }// switch state
}

// Change the state according to the RX buffer
void ChangeState()
{
    switch(UCA0RXBUF)
       {
     case '0':
         State=0;
         AngRangeFlag=0;
         break;
     case '1':
       State=1;
       AngRangeFlag=0;
       break;
     case '2':
       State=2;
       AngRangeFlag=0;
       break;
     case '3':
       State=3;
       fileorstate=0;
       AngRangeFlag=0;
       break;
       } //switch buf RX

}


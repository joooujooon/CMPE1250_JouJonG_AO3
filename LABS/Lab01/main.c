/*********************************************************************
*                    SEGGER Microcontroller GmbH                     
*                        The Embedded Experts                        
**********************************************************************
File    : main.c - ICA05 
Author  : Jou Jon Galenzoga

*/

#include <stdio.h>                     
#include "stm32g031xx.h"                
#include "gpio.h"                       
#include "usart.h"      

/*********************************************************************
*
*       main()
*
*  Function description
*   Application entry point.
*/
int main(void) {
  int i;

  for (i = 0; i < 100; i++) {
    printf("Hello World %d!\n", i);
  }
  do {
    i++;
  } while (1);
}

/*************************** End of file ****************************/

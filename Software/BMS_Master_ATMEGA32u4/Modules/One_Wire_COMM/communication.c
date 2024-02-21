/*************************/
/*  communication.c      */
/*						 */
/*  Battery Management   */
/*      System           */
/* 						 */
/*  Functions for        */
/*  communication with   */
/*  upper and lower uC   */
/*************************/

#include <stdint.h>
#include "communication.h"

uint16_t calc_data_bal (uint8_t address)
{
    uint8_t par_cnt=0;
    for(int i=0; i<8;i++)  //count number of "1"
    {
        if(address&(1<<i))
        {
            par_cnt++;
        }
    }
    if(par_cnt%2)   //if uneven number of "1"
    {
        return COM_BLC_A + 0x10 + address;   //set paritybit of dataword to "1"
    }
    else    //if even number of "1"
    {
        return COM_BLC_A + address;   //set paritybit of dataword to "0"
    }
}
uint8_t calc_parity(uint16_t data)
{
    uint8_t par_cnt=0;
    for(int i=0; i<16;i++)  //count number of "1"
    {
        if(data&(1<<i))
        {
            par_cnt++;
        }
    }
    if(par_cnt%2)   //if uneven number of "1"
    {
        return 1;   //return 1
    }
    else    //if even number of "1"
    {
        return 0;   //return 0
    }
}
/*************************/
/*  communication.h      */
/*						 */
/*  Battery Management   */
/*      System           */
/* 						 */
/*  Functions for        */
/*  communication with   */
/*  upper and lower uC   */
/*************************/
#ifndef COMM_H
#define COMM_H

#define REQ_TEMP_G 0xD4AA   //global request for temperature 1 10 10101 0 0101010
#define REQ_VOLT_G 0xC633    //global request for voltage 1 10 00110 0 0110011
#define COM_SLP_G 0xFF0F    //global command for sleep 1 11 1111 1 00001111
#define COM_BLC_OFF_G 0xB955 //global command balancer off
#define COM_BLC_A 0xA600    //adressed command for balancing 1 01 00110 0 0000000 COM_BLC_A + adress = data

#define PARITY_BIT_COM_A (1<<7) //Position of the Parity bit in an addressed command
#define ADDRESS_MASK 0x007F     //Mask to isolate the address 0000 0000 0111 1111

uint16_t calc_data_bal (uint8_t);
//calculate parity for adressed command. argument is address.

uint8_t calc_parity(uint16_t);
//calculate parity based on data input. Returns 1 if uneven numer of "1" returns 0 if even number

#endif //COMM_H
//"I geh ned mit ins gym. I hob duachfall. I hob mi a bissl ogschissn." -kati 23.02.2022

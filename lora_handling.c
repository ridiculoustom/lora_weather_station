/*
    \file   lora_handling.c

    \brief  lora handler source file.

    (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "atmel_start_pins.h"
#include "clock_config.h"
#include <util/delay.h>
#include "lora2_click.h"
#include "driver_rn2903.h"
#include "lora_handling.h"
#include <stdbool.h>
#include "lora_keys.h"

#define NO_FREE_CH                              "no_free_ch\r\n"
#define OK                                      "ok\r\n"
#define ACCEPTED                                "accepted\r\n"
#define FIRST_CHARACTER                         0
#define RESPONSE_LENGTH                         40
#define TRANSMISSION_LENGTH                     100
#define COMMAND_LENGTH                          100
#define SHORT_DELAY                             2
#define MEDIUM_DELAY                            15
#define LONG_DELAY                              1000
#define EQUALS                                  0
#define TEMPERATURE_MAGNITUDE                   100
#define NO_CHAR_YET                             0

static void handleResponse(void);
static void loopUntilOk(const char* command);
static void createTransmission(char* sensor_data);
static void sendKey(const char* command, const char* key);

#ifdef OTAA

const char RN_cmd_deveui[] = "mac set deveui";
const char RN_cmd_appeui[] = "mac set appeui";
const char RN_cmd_appkey[] = "mac set appkey";

const char RN_cmd_join_otaa[] = "mac join otaa";

#elif ABP

const char RN_cmd_devaddr[] = "mac set devaddr";
const char RN_cmd_nwkskey[] = "mac set nwkskey";
const char RN_cmd_appskey[] = "mac set appskey";

const char RN_cmd_join_abp[] = "mac join abp";

#endif

const char RN_cmd_mac_get[] = "mac get";
const char band[] = "band";
const char gwnb[] = "gwnb";
const char status[] = "status";

const char RN_cmd_mac_set[] = "mac set";

const char channel[] = "ch";


const char RN_cmd_mac_save[] = "mac save";
const char RN_cmd_mac_set_dr[] = "mac set dr 5";
const char RN_cmd_mac_set_adr[] = "mac set adr on";
const char RN_cmd_mac_set_lnkchk[] = "mac set linkchk 120";
const char RN_cmd_sys_sleep[] = "sys sleep";
const char RN_cmd_mac_tx[] = "mac tx";
const char confirmed_transmission[] = "cnf 1";
const char unconfirmed_transmission[] = "uncnf 22";

char response[RESPONSE_LENGTH];
char complete_command[COMMAND_LENGTH];
char transmission_command[TRANSMISSION_LENGTH];

extern bool has_entered_low_power_at_least_once;
extern volatile uint8_t LoRa2ResponseIndex;
extern volatile char LoRa2ResponseBuffer[];

void LORA_HANDLING_createCommand(const char* command, const char* value)
{
    snprintf(complete_command, sizeof complete_command, "%s %s", command, value);
}

static void sendKey (const char* command, const char* key)
{
    LORA_HANDLING_createCommand(command, key);
    rn2903_SendString(complete_command);
	printf("%s", complete_command);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
}

static void handleResponse(void)
{
    do{
		;
	}while(LoRa2ResponseBuffer[LoRa2ResponseIndex - 1] != '\n');
	
    strncpy(response,LoRa2_GetResponse(),responseBufferSize);
    printf("%s", response);
}

static void loopUntilOk(const char* command_to_be_sent)
{
    do
    {
        LoRa2_ReadyReceiveBuffer();
        
        rn2903_SendString(command_to_be_sent);
		printf("%s", command_to_be_sent);

        handleResponse();
        
        if(strcmp(response, NO_FREE_CH) == EQUALS)
        {
            LoRa2_blockingWait(LONG_DELAY);
        }
    }while(strcmp(response, OK) != EQUALS);
}

void LORA_HANDLING_loraInit(void) // Call After System Init; Above while(1) Loop in Main.c
{
    
    LoRa2_RegisterISRCallback();
    LoRa2_blockingWait(SHORT_DELAY);
    
    rn2903_SetHardwareReset(false);
    LoRa2_blockingWait(SHORT_DELAY);
    rn2903_SetHardwareReset(true);
    LoRa2_blockingWait(MEDIUM_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
	snprintf(complete_command, sizeof complete_command, "%s %s", RN_cmd_mac_get, band);
	printf(complete_command);
	rn2903_SendString(complete_command);
	LoRa2_blockingWait(SHORT_DELAY);
	handleResponse();
	LoRa2_ReadyReceiveBuffer();
	
    /*
    snprintf(complete_command, sizeof complete_command, "sys factoryRESET");
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
    */
    #ifdef OTAA

    sendKey(RN_cmd_deveui, my_deveui);
    sendKey(RN_cmd_appeui, my_appeui);
    sendKey(RN_cmd_appkey, my_appkey);
	
    #elif ABP
	
    sendKey(RN_cmd_devaddr, my_devaddr);
    sendKey(RN_cmd_nwkskey, my_nwkskey);
    sendKey(RN_cmd_appskey, my_appskey);

    #endif
    /*
    snprintf(complete_command, sizeof complete_command, "%s %s status 0 off", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 1 off", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 2 off", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 3 864862500", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 3 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 3 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 3 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 4 865062500", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 4 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 4 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 4 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 5 865402500", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 5 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 5 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 5 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
    
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 6 865602500", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 6 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 6 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 6 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 8 866200000", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 8 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 8 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s status 8 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s freq 9 866400000", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 9 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 9 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 9 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
		
    snprintf(complete_command, sizeof complete_command, "%s %s freq 10 866600000", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
			
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 10 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
			
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 10 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
			
    snprintf(complete_command, sizeof complete_command, "%s %s status 10 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s freq 7 865985000", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s dcycle 7 0", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s drrange 7 0 6", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
    snprintf(complete_command, sizeof complete_command, "%s %s status 7 on", RN_cmd_mac_set, channel);
    printf(complete_command);
    rn2903_SendString(complete_command);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
    */
	
    /*
    rn2903_SendString(RN_cmd_mac_set_adr);
    printf("%s", RN_cmd_mac_set_adr);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
	
	
    rn2903_SendString(RN_cmd_mac_set_lnkchk);
    printf("%s", RN_cmd_mac_set_lnkchk);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();
    */
	

    rn2903_SendString(RN_cmd_mac_set_dr);
	printf("%s\r\n", RN_cmd_mac_set_dr);
    LoRa2_blockingWait(SHORT_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();

    rn2903_SendString(RN_cmd_mac_save);
	printf("%s", RN_cmd_mac_save);
    LoRa2_blockingWait(LONG_DELAY);
    handleResponse();
    LoRa2_ReadyReceiveBuffer();

    do
    {  //rejoins until the message is "accepted" instead of "denied"	
        snprintf(complete_command, sizeof complete_command, "%s %s", RN_cmd_mac_get, status);
        printf(complete_command);
        rn2903_SendString(complete_command);
        LoRa2_blockingWait(SHORT_DELAY);
        handleResponse();
        LoRa2_ReadyReceiveBuffer();

#ifdef OTAA

        loopUntilOk(RN_cmd_join_otaa);

#elif ABP

        loopUntilOk(RN_cmd_join_abp);

#endif
 
        LoRa2_ReadyReceiveBuffer();
        handleResponse();

    }while(strcmp(response, ACCEPTED) != EQUALS);
	
	snprintf(complete_command, sizeof complete_command, "%s %s", RN_cmd_mac_get, status);
	printf(complete_command);
	rn2903_SendString(complete_command);
	LoRa2_blockingWait(SHORT_DELAY);
	handleResponse();
	LoRa2_ReadyReceiveBuffer();

}

static void createTransmission(char* sensor_data)
{
    LORA_HANDLING_createCommand(RN_cmd_mac_tx, unconfirmed_transmission);
    snprintf(transmission_command, sizeof transmission_command, "%s %s", complete_command, sensor_data);
}

void LORA_HANDLING_transmit(char* sensor_data)
{
    
    if(has_entered_low_power_at_least_once == true)
    {
        LoRa2_blockingWait(MEDIUM_DELAY);
        //handleResponse();
        LoRa2_blockingWait(SHORT_DELAY);
        /*	
        snprintf(complete_command, sizeof complete_command, "%s %s", RN_cmd_mac_get, status);
        printf(complete_command);
        rn2903_SendString(complete_command);
        LoRa2_blockingWait(SHORT_DELAY);
        handleResponse();
        LoRa2_ReadyReceiveBuffer();
        */
    }

    createTransmission(sensor_data);

    loopUntilOk(transmission_command);

    LoRa2_blockingWait(SHORT_DELAY);
    LoRa2_ReadyReceiveBuffer();

    handleResponse();
	
}
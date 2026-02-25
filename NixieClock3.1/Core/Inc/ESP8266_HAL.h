/*
 * ESP8266_HAL.h
 *
 *  Created on: Apr 14, 2020
 *      Author: Controllerstech
 */

#ifndef INC_ESP8266_HAL_H_
#define INC_ESP8266_HAL_H_


int ESP_Init (char *SSID, char *PASSWD);

int Server_Start (void);
RTC_TimeTypeDef AskTime(RTC_DateTypeDef *Date);

#endif /* INC_ESP8266_HAL_H_ */

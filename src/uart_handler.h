/*
 * Copyright 2026 4BRAINS Investigación y Desarrollo EIRL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * uart_handler.h — interfaz del lector/escritor del puerto serie.
 */

#ifndef UART_HANDLER_H_
#define UART_HANDLER_H_

#include <stdbool.h>
#include <pthread.h>

#define UART_PORT       "/dev/ttyS1"   /* puerto serie — ajustar al sistema     */
#define BAUDRATE        B9600          /* velocidad — ajustar al dispositivo    */
#define BUFFER_SIZE     256
#define END_FRAME_CHAR  '\n'           /* delimitador de fin de trama (config.) */

int  UART_Init(void);
void UART_dInit(void);
int  UART_Write(const char *data);

extern char             frame_buffer[BUFFER_SIZE];
extern volatile bool    frame_ready;
extern pthread_mutex_t  frame_mutex;

#endif /* UART_HANDLER_H_ */

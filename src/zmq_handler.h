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
 * zmq_handler.h — interfaz de la capa de mensajería ZeroMQ.
 */

#ifndef ZMQ_HANDLER_H_
#define ZMQ_HANDLER_H_

#include <stdbool.h>
#include "uart_handler.h"

/* Endpoints IPC — solo accesibles localmente en el mismo equipo */
#define ZMQ_PUB_ENDPOINT    "ipc:///tmp/serial_rx.ipc"
#define ZMQ_PULL_ENDPOINT   "ipc:///tmp/serial_tx.ipc"

/* Topics del socket PUB */
#define ZMQ_TOPIC_FRAME     "FRAME "   /* seguido del contenido de la trama */
#define ZMQ_TOPIC_TIMEOUT   "TIMEOUT"  /* sin contenido adicional           */

int  ZMQ_Init(void);
void ZMQ_dInit(void);
void ZMQ_Publish_Frame(const char *frame);
void ZMQ_Publish_Timeout(void);
bool ZMQ_Pull_Write(char *out_buf, int buf_size);

#endif /* ZMQ_HANDLER_H_ */

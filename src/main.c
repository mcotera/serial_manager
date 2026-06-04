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
 * main.c — bucle principal del puente serie <-> ZeroMQ.
 *
 * Lee tramas delimitadas del puerto serie y las publica por un socket PUB;
 * recibe datos a escribir por un socket PULL y los vuelca al puerto serie.
 * Es agnóstico al protocolo: solo enmarca por un delimitador configurable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "uart_handler.h"
#include "zmq_handler.h"

#define FRAME_TIMEOUT_SEC   2

static int run = 1;
volatile sig_atomic_t frame_timeout = 0;

static void sighandler(int sig) {
    (void)sig;
    fprintf(stderr, "Terminación solicitada\n");
    run = 0;
}

static void alarm_handler(int sig) {
    (void)sig;
    fprintf(stderr, "timeout uart communication\n");
    frame_timeout = 1;
}

int main(void) {
    char write_buf[BUFFER_SIZE];

    signal(SIGABRT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGINT,  sighandler);
    signal(SIGALRM, alarm_handler);

    if (UART_Init() != 0) {
        fprintf(stderr, "UART Initialization Failure\n");
        return EXIT_FAILURE;
    }

    if (ZMQ_Init() != 0) {
        fprintf(stderr, "ZMQ Initialization Failure\n");
        UART_dInit();
        return EXIT_FAILURE;
    }

    frame_timeout = 0;
    alarm(FRAME_TIMEOUT_SEC);

    while (run) {

        /* Trama recibida — publicar y reiniciar watchdog */
        if (frame_ready) {
            pthread_mutex_lock(&frame_mutex);
            frame_ready = false;
            ZMQ_Publish_Frame(frame_buffer);
            pthread_mutex_unlock(&frame_mutex);

            frame_timeout = 0;
            alarm(FRAME_TIMEOUT_SEC);
        }

        /* Timeout serial — publicar evento y esperar próxima trama */
        if (frame_timeout) {
            frame_timeout = 0;
            ZMQ_Publish_Timeout();
        }

        /* Solicitud de escritura desde un cliente — reenviar al serial */
        if (ZMQ_Pull_Write(write_buf, BUFFER_SIZE)) {
            UART_Write(write_buf);
        }

        usleep(10000);
    }

    ZMQ_dInit();
    UART_dInit();

    return EXIT_SUCCESS;
}

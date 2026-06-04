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
 * uart_handler.c — lectura por hilo del puerto serie con enmarcado por
 * delimitador y escritura síncrona.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <stdbool.h>
#include "uart_handler.h"

int             uart_fd;
pthread_t       uart_thread;
volatile sig_atomic_t auxthread_run = 1;

char            frame_buffer[BUFFER_SIZE];
volatile bool   frame_ready = false;
pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;

void* uart_reader(void* arg) {
    (void)arg;
    char ch;
    int idx = 0;
    char local_rx_buffer[BUFFER_SIZE];

    while (auxthread_run) {
        int n = read(uart_fd, &ch, 1);
        if (n > 0) {
            if (idx < BUFFER_SIZE - 1) {
                local_rx_buffer[idx++] = ch;

                if (ch == END_FRAME_CHAR) {
                    local_rx_buffer[idx] = '\0';

                    fprintf(stdout, "[UART RX] %s\n", local_rx_buffer);
                    fflush(stdout);

                    pthread_mutex_lock(&frame_mutex);
                    strncpy(frame_buffer, local_rx_buffer, BUFFER_SIZE);
                    frame_ready = true;
                    pthread_mutex_unlock(&frame_mutex);

                    idx = 0;
                }
            } else {
                fprintf(stderr, "Error: Trama demasiado larga. Reiniciando buffer.\n");
                idx = 0;
            }
        }
    }

    pthread_exit(NULL);
}

int UART_Init(void) {
    struct termios options;

    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        perror("Error abriendo el puerto UART");
        return -1;
    }

    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CRTSCTS;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &options);

    if (pthread_create(&uart_thread, NULL, uart_reader, NULL) != 0) {
        perror("Error creando hilo UART");
        close(uart_fd);
        return -1;
    }

    return 0;
}

int UART_Write(const char *data) {
    int len = (int)strlen(data);
    int n = write(uart_fd, data, len);
    if (n < 0) {
        perror("Error escribiendo al UART");
        return -1;
    }
    return 0;
}

void UART_dInit(void) {
    fprintf(stdout, "Cerrando UART\n");
    auxthread_run = 0;
    close(uart_fd);
    pthread_cancel(uart_thread);
    pthread_join(uart_thread, NULL);
}

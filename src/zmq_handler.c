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
 * zmq_handler.c — publica tramas por PUB y recibe escrituras por PULL.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
#include "zmq_handler.h"

/* Extrae la ruta del filesystem de un endpoint "ipc:///ruta" */
static const char *ipc_path(const char *endpoint) {
    return endpoint + 6;   /* salta "ipc://" */
}

static void *zmq_context = NULL;
static void *zmq_pub     = NULL;
static void *zmq_pull    = NULL;

int ZMQ_Init(void) {
    zmq_context = zmq_ctx_new();
    if (!zmq_context) {
        perror("zmq_ctx_new");
        return -1;
    }

    /* Limpiar sockets IPC huérfanos de ejecuciones anteriores */
    unlink(ipc_path(ZMQ_PUB_ENDPOINT));
    unlink(ipc_path(ZMQ_PULL_ENDPOINT));

    /* Socket PUB — publica tramas a los suscriptores */
    zmq_pub = zmq_socket(zmq_context, ZMQ_PUB);
    if (!zmq_pub) {
        perror("zmq_socket PUB");
        goto err_ctx;
    }
    if (zmq_bind(zmq_pub, ZMQ_PUB_ENDPOINT) != 0) {
        perror("zmq_bind PUB");
        goto err_pub;
    }

    /* Socket PULL — recibe solicitudes de escritura al serial (desde un cliente) */
    zmq_pull = zmq_socket(zmq_context, ZMQ_PULL);
    if (!zmq_pull) {
        perror("zmq_socket PULL");
        goto err_pub;
    }
    if (zmq_bind(zmq_pull, ZMQ_PULL_ENDPOINT) != 0) {
        perror("zmq_bind PULL");
        goto err_pull;
    }

    printf("ZMQ PUB : %s\n", ZMQ_PUB_ENDPOINT);
    printf("ZMQ PULL: %s\n", ZMQ_PULL_ENDPOINT);
    return 0;

err_pull:
    zmq_close(zmq_pull);
err_pub:
    zmq_close(zmq_pub);
err_ctx:
    zmq_ctx_destroy(zmq_context);
    return -1;
}

void ZMQ_dInit(void) {
    if (zmq_pub)     zmq_close(zmq_pub);
    if (zmq_pull)    zmq_close(zmq_pull);
    if (zmq_context) zmq_ctx_destroy(zmq_context);
    printf("ZMQ cerrado.\n");
}

void ZMQ_Publish_Frame(const char *frame) {
    char msg[BUFFER_SIZE + 16];
    int n = snprintf(msg, sizeof(msg), "%s%s", ZMQ_TOPIC_FRAME, frame);
    zmq_send(zmq_pub, msg, n, 0);
}

void ZMQ_Publish_Timeout(void) {
    zmq_send(zmq_pub, ZMQ_TOPIC_TIMEOUT, sizeof(ZMQ_TOPIC_TIMEOUT) - 1, 0);
}

bool ZMQ_Pull_Write(char *out_buf, int buf_size) {
    int n = zmq_recv(zmq_pull, out_buf, buf_size - 1, ZMQ_DONTWAIT);
    if (n > 0) {
        out_buf[n] = '\0';
        return true;
    }
    return false;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clavesRPC.h"
#include "claves.h"

bool_t destroy_rpc_1_svc(int *result, struct svc_req *rqstp) {
    *result = destroy();
    return TRUE;
}

/* CORRECCIÓN: argp se pasa por valor (sin el *) y se accede con punto en lugar de flecha */
bool_t set_value_rpc_1_svc(set_value_args argp, int *result, struct svc_req *rqstp) {
    struct Paquete pkt;
    pkt.x = argp.value3.x;
    pkt.y = argp.value3.y;
    pkt.z = argp.value3.z;
    *result = set_value(argp.key, argp.value1, argp.n_value2, argp.v_value2.v_value2_val, pkt);
    return TRUE;
}

/* CORRECCIÓN: argp es un char* directo, no un char** */
bool_t get_value_rpc_1_svc(char *argp, get_value_res *result, struct svc_req *rqstp) {
    memset(result, 0, sizeof(*result));
    result->value1 = (char *)malloc(256);
    if (result->value1 == NULL) {
        result->error = -1;
        result->value1 = strdup("");
        result->v_value2.v_value2_val = NULL;
        result->v_value2.v_value2_len = 0;
        return TRUE;
    }
    float *v2_buf = (float *)malloc(32 * sizeof(float));
    if (v2_buf == NULL) {
        free(result->value1);
        result->value1 = strdup("");
        result->error = -1;
        result->v_value2.v_value2_val = NULL;
        result->v_value2.v_value2_len = 0;
        return TRUE;
    }
    int N = 0;
    struct Paquete pkt;
    
    /* Se le pasa argp directamente */
    result->error = get_value(argp, result->value1, &N, v2_buf, &pkt);
    
    if (result->error == 0) {
        result->n_value2 = N;
        result->v_value2.v_value2_val = v2_buf;
        result->v_value2.v_value2_len = (u_int)N;
        result->value3.x = pkt.x;
        result->value3.y = pkt.y;
        result->value3.z = pkt.z;
    } else {
        free(v2_buf);
        result->v_value2.v_value2_val = NULL;
        result->v_value2.v_value2_len = 0;
        result->value1[0] = '\0';
    }
    return TRUE;
}

bool_t modify_value_rpc_1_svc(set_value_args argp, int *result, struct svc_req *rqstp) {
    struct Paquete pkt;
    pkt.x = argp.value3.x;
    pkt.y = argp.value3.y;
    pkt.z = argp.value3.z;
    *result = modify_value(argp.key, argp.value1, argp.n_value2, argp.v_value2.v_value2_val, pkt);
    return TRUE;
}

bool_t delete_key_rpc_1_svc(char *argp, int *result, struct svc_req *rqstp) {
    *result = delete_key(argp);
    return TRUE;
}

bool_t exist_rpc_1_svc(char *argp, int *result, struct svc_req *rqstp) {
    *result = exist(argp);
    return TRUE;
}

int claves_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result) {
    xdr_free(xdr_result, result);
    return 1;
}
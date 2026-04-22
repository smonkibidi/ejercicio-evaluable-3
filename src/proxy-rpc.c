#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves.h"
#include "clavesRPC.h"

static CLIENT *crear_cliente(void) {
    const char *ip = getenv("IP_TUPLAS");
    if (ip == NULL) {
        fprintf(stderr, "[proxy-rpc] ERROR: variable IP_TUPLAS no definida\n");
        return NULL;
    }
    CLIENT *clnt = clnt_create(ip, CLAVES_PROG, CLAVES_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(ip);
        return NULL;
    }
    return clnt;
}

int destroy(void) {
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    int result = 0;
    
    /* CORRECCIÓN: (resultado, cliente) */
    enum clnt_stat stat = destroy_rpc_1(&result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    return result;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32) return -1;
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    
    set_value_args args;
    memset(&args, 0, sizeof(args));
    args.key = key;
    args.value1 = value1;
    args.n_value2 = N_value2;
    args.v_value2.v_value2_val = V_value2;
    args.v_value2.v_value2_len = (u_int)N_value2;
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;
    
    int result = 0;
    /* CORRECCIÓN: Se pasa 'args' por valor, luego &result, luego clnt */
    enum clnt_stat stat = set_value_rpc_1(args, &result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    return result;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    if (key == NULL || value1 == NULL || N_value2 == NULL || V_value2 == NULL || value3 == NULL) return -1;
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    
    get_value_res result;
    memset(&result, 0, sizeof(result));
    
    /* CORRECCIÓN: Se pasa 'key' por valor (ya es char*), &result, clnt */
    enum clnt_stat stat = get_value_rpc_1(key, &result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    if (result.error != 0) {
        xdr_free((xdrproc_t)xdr_get_value_res, (char *)&result);
        return -1;
    }
    
    strncpy(value1, result.value1, 255);
    value1[255] = '\0';
    *N_value2 = result.n_value2;
    u_int n = result.v_value2.v_value2_len;
    for (u_int i = 0; i < n && i < 32; i++) {
        V_value2[i] = result.v_value2.v_value2_val[i];
    }
    value3->x = result.value3.x;
    value3->y = result.value3.y;
    value3->z = result.value3.z;
    
    xdr_free((xdrproc_t)xdr_get_value_res, (char *)&result);
    return 0;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32) return -1;
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    
    set_value_args args;
    memset(&args, 0, sizeof(args));
    args.key = key;
    args.value1 = value1;
    args.n_value2 = N_value2;
    args.v_value2.v_value2_val = V_value2;
    args.v_value2.v_value2_len = (u_int)N_value2;
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;
    
    int result = 0;
    /* CORRECCIÓN */
    enum clnt_stat stat = modify_value_rpc_1(args, &result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    return result;
}

int delete_key(char *key) {
    if (key == NULL) return -1;
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    
    int result = 0;
    /* CORRECCIÓN */
    enum clnt_stat stat = delete_key_rpc_1(key, &result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    return result;
}

int exist(char *key) {
    if (key == NULL) return -1;
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;
    
    int result = 0;
    /* CORRECCIÓN */
    enum clnt_stat stat = exist_rpc_1(key, &result, clnt);
    
    clnt_destroy(clnt);
    if (stat != RPC_SUCCESS) return -1;
    return result;
}
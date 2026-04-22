/**
 * @file proxy-rpc.c
 * @brief Biblioteca cliente del servicio de tuplas usando ONC RPC.
 *
 * Implementa la API definida en claves.h redirigiendo cada llamada al
 * servidor remoto mediante ONC/TI-RPC. La IP del servidor se obtiene
 * de la variable de entorno IP_TUPLAS (el puerto lo gestiona rpcbind).
 *
 * Compilado como libproxyclaves.so, junto con los ficheros _clnt y _xdr
 * generados por rpcgen.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/rpc.h>
#include "claves.h"
#include "clavesRPC.h"


/* ------------------------------------------------------------------ */
/*  Helper: crear y destruir manejador de cliente RPC                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Crea un CLIENT* conectado al servidor cuya IP está en IP_TUPLAS.
 *
 * Usa TCP como transporte. El puerto lo negocia rpcbind automáticamente,
 * por lo que no es necesario conocerlo en el cliente.
 *
 * @return Puntero al manejador de cliente, o NULL si falla.
 */
static CLIENT *crear_cliente(void)
{
    const char *ip = getenv("IP_TUPLAS");
    if (ip == NULL) {
        fprintf(stderr, "[proxy-rpc] ERROR: variable IP_TUPLAS no definida\n");
        return NULL;
    }

    CLIENT *clnt = clnt_create(ip, CLAVES_PROG, CLAVES_VERS, "tcp");
    if (clnt == NULL) {
        /* clnt_pcreateerror imprime el motivo del fallo en stderr */
        clnt_pcreateerror(ip);
        return NULL;
    }
    return clnt;
}


/* ------------------------------------------------------------------ */
/*  Implementación de la API (proxy RPC)                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Destruye todas las tuplas del servidor.
 * @return 0 si éxito, -1 en caso de error (incluye fallo RPC).
 */
int destroy(void)
{
    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    int result = 0;
    enum clnt_stat stat = destroy_rpc_1(clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;
    return result;   /* 0 ó -1 según el servidor */
}


/**
 * @brief Inserta una nueva tupla en el servidor.
 * @return 0 si éxito, -1 en caso de error.
 */
int set_value(char *key, char *value1, int N_value2,
              float *V_value2, struct Paquete value3)
{
    /* Validación local previa al RPC */
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32)                     return -1;

    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    /* Rellenar la estructura de argumentos RPC */
    set_value_args args;
    memset(&args, 0, sizeof(args));
    args.key    = key;
    args.value1 = value1;
    args.n_value2 = N_value2;
    args.v_value2.v_value2_val = V_value2;
    args.v_value2.v_value2_len = (u_int)N_value2;
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int result = 0;
    enum clnt_stat stat = set_value_rpc_1(&args, clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;
    return result;
}


/**
 * @brief Obtiene los valores asociados a la clave key del servidor.
 * @return 0 si éxito, -1 en caso de error.
 */
int get_value(char *key, char *value1, int *N_value2,
              float *V_value2, struct Paquete *value3)
{
    if (key == NULL || value1 == NULL || N_value2 == NULL ||
        V_value2 == NULL || value3 == NULL) return -1;

    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    /*
     * La estructura resultado debe inicializarse a cero antes de pasarla
     * a la función RPC: el XDR decodificador reservará memoria dinámica
     * para los campos string y array, que luego liberaremos con xdr_free.
     */
    get_value_res result;
    memset(&result, 0, sizeof(result));

    enum clnt_stat stat = get_value_rpc_1(&key, clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;

    if (result.error != 0) {
        /* El servidor devolvió error lógico; liberamos y propagamos -1 */
        xdr_free((xdrproc_t)xdr_get_value_res, (char *)&result);
        return -1;
    }

    /* Copiar los datos a los buffers del llamador */
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

    /* Liberar la memoria dinámica reservada por el decodificador XDR */
    xdr_free((xdrproc_t)xdr_get_value_res, (char *)&result);
    return 0;
}


/**
 * @brief Modifica los valores de una tupla existente.
 * @return 0 si éxito, -1 en caso de error.
 */
int modify_value(char *key, char *value1, int N_value2,
                 float *V_value2, struct Paquete value3)
{
    if (key == NULL || value1 == NULL || V_value2 == NULL) return -1;
    if (N_value2 < 1 || N_value2 > 32)                     return -1;

    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    /* modify_value_rpc usa la misma estructura de argumentos que set_value_rpc */
    set_value_args args;
    memset(&args, 0, sizeof(args));
    args.key    = key;
    args.value1 = value1;
    args.n_value2 = N_value2;
    args.v_value2.v_value2_val = V_value2;
    args.v_value2.v_value2_len = (u_int)N_value2;
    args.value3.x = value3.x;
    args.value3.y = value3.y;
    args.value3.z = value3.z;

    int result = 0;
    enum clnt_stat stat = modify_value_rpc_1(&args, clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;
    return result;
}


/**
 * @brief Elimina la tupla cuya clave es key.
 * @return 0 si éxito, -1 en caso de error.
 */
int delete_key(char *key)
{
    if (key == NULL) return -1;

    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    int result = 0;
    enum clnt_stat stat = delete_key_rpc_1(&key, clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;
    return result;
}


/**
 * @brief Comprueba si existe la clave key en el servidor.
 * @return 1 si existe, 0 si no existe, -1 en caso de error.
 */
int exist(char *key)
{
    if (key == NULL) return -1;

    CLIENT *clnt = crear_cliente();
    if (clnt == NULL) return -1;

    int result = 0;
    enum clnt_stat stat = exist_rpc_1(&key, clnt, &result);
    clnt_destroy(clnt);

    if (stat != RPC_SUCCESS) return -1;
    return result;   /* 0, 1, ó -1 según el servidor */
}

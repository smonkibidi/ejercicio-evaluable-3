/**
 * @file clavesRPC_server.c
 * @brief Implementación de los procedimientos RPC del servidor de tuplas.
 *
 * Este fichero rellena las funciones _svc generadas por rpcgen a partir de
 * clavesRPC.x. Cada función delega en la API real de claves.c, convirtiendo
 * entre los tipos RPC (paquete_rpc, set_value_args, get_value_res) y los
 * tipos nativos de claves.h (struct Paquete).
 *
 * El main() del servidor RPC lo proporciona el fichero clavesRPC_svc.c
 * generado automáticamente por rpcgen; este fichero no define main().
 *
 * Compilación: se incluye en el ejecutable clavesRPC_server junto con
 * clavesRPC_svc.c, clavesRPC_xdr.c y libclaves.so.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clavesRPC.h"   /* Tipos y prototipos generados por rpcgen */
#include "claves.h"      /* API real del servicio de tuplas         */


/* ------------------------------------------------------------------ */
/*  DESTROY                                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC DESTROY_RPC: elimina todas las tuplas.
 *
 * @param result  Salida: 0 éxito, -1 error.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre (el resultado se comunica mediante *result).
 */
bool_t
destroy_rpc_1_svc(int *result, struct svc_req *rqstp)
{
    *result = destroy();
    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  SET_VALUE                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC SET_VALUE_RPC: inserta una nueva tupla.
 *
 * Convierte los tipos RPC a los tipos nativos y delega en set_value().
 *
 * @param argp    Argumentos: clave, value1, N_value2, V_value2, value3.
 * @param result  Salida: 0 éxito, -1 error.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre.
 */
bool_t
set_value_rpc_1_svc(set_value_args *argp, int *result, struct svc_req *rqstp)
{
    /* Convertir paquete_rpc → struct Paquete */
    struct Paquete pkt;
    pkt.x = argp->value3.x;
    pkt.y = argp->value3.y;
    pkt.z = argp->value3.z;

    *result = set_value(argp->key,
                        argp->value1,
                        argp->n_value2,
                        argp->v_value2.v_value2_val,
                        pkt);
    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  GET_VALUE                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC GET_VALUE_RPC: obtiene los valores de una clave.
 *
 * Los buffers de salida (value1 y v_value2_val) se asignan dinámicamente
 * con malloc para que el marco RPC pueda serializarlos y liberarlos
 * correctamente mediante xdr_free / claves_prog_1_freeresult.
 *
 * @param argp    Puntero a la clave de búsqueda (char**).
 * @param result  Salida: error + datos de la tupla.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre.
 */
bool_t
get_value_rpc_1_svc(char **argp, get_value_res *result, struct svc_req *rqstp)
{
    memset(result, 0, sizeof(*result));

    /* Reservar memoria para value1 (el XDR la liberará con free) */
    result->value1 = (char *)malloc(256);
    if (result->value1 == NULL) {
        result->error = -1;
        result->value1 = strdup("");          /* evitar puntero NULL en XDR */
        result->v_value2.v_value2_val = NULL;
        result->v_value2.v_value2_len = 0;
        return TRUE;
    }

    /* Reservar memoria para el vector de floats */
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
    result->error = get_value(*argp, result->value1, &N, v2_buf, &pkt);

    if (result->error == 0) {
        result->n_value2 = N;
        result->v_value2.v_value2_val = v2_buf;
        result->v_value2.v_value2_len = (u_int)N;
        result->value3.x = pkt.x;
        result->value3.y = pkt.y;
        result->value3.z = pkt.z;
    } else {
        /* Clave no encontrada: liberar el vector, valor1 queda vacío */
        free(v2_buf);
        result->v_value2.v_value2_val = NULL;
        result->v_value2.v_value2_len = 0;
        result->value1[0] = '\0';
    }

    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  MODIFY_VALUE                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC MODIFY_VALUE_RPC: modifica una tupla existente.
 *
 * @param argp    Argumentos con los nuevos valores (mismo tipo que SET).
 * @param result  Salida: 0 éxito, -1 error.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre.
 */
bool_t
modify_value_rpc_1_svc(set_value_args *argp, int *result, struct svc_req *rqstp)
{
    struct Paquete pkt;
    pkt.x = argp->value3.x;
    pkt.y = argp->value3.y;
    pkt.z = argp->value3.z;

    *result = modify_value(argp->key,
                           argp->value1,
                           argp->n_value2,
                           argp->v_value2.v_value2_val,
                           pkt);
    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  DELETE_KEY                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC DELETE_KEY_RPC: elimina la tupla con clave dada.
 *
 * @param argp    Puntero a la clave a eliminar.
 * @param result  Salida: 0 éxito, -1 error.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre.
 */
bool_t
delete_key_rpc_1_svc(char **argp, int *result, struct svc_req *rqstp)
{
    *result = delete_key(*argp);
    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  EXIST                                                               */
/* ------------------------------------------------------------------ */

/**
 * @brief Procedimiento RPC EXIST_RPC: comprueba si existe una clave.
 *
 * @param argp    Puntero a la clave a buscar.
 * @param result  Salida: 1 existe, 0 no existe, -1 error.
 * @param rqstp   Contexto de la petición (no utilizado).
 * @return TRUE siempre.
 */
bool_t
exist_rpc_1_svc(char **argp, int *result, struct svc_req *rqstp)
{
    *result = exist(*argp);
    return TRUE;
}


/* ------------------------------------------------------------------ */
/*  Liberación de recursos                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Libera los recursos dinámicos del resultado de un procedimiento.
 *
 * Llamado automáticamente por el dispatcher del servidor tras serializar
 * la respuesta. xdr_free libera los campos string y array de get_value_res.
 */
int
claves_prog_1_freeresult(SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)
{
    xdr_free(xdr_result, result);
    return 1;
}

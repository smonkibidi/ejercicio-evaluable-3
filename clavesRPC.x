/* clavesRPC.x */

/* Estructura equivalente a struct Paquete de claves.h */
struct paquete_rpc {
    int x;
    int y;
    int z;
};

/* Parámetros para set_value y modify_value */
struct set_value_args {
    string key<256>;
    string value1<256>;
    int n_value2;
    float v_value2<32>; /* Array de longitud variable (máx 32) */
    struct paquete_rpc value3;
};

/* Resultado para get_value */
struct get_value_res {
    int error; /* 0 éxito, -1 error */
    string value1<256>;
    int n_value2;
    float v_value2<32>;
    struct paquete_rpc value3;
};

program CLAVES_PROG {
    version CLAVES_VERS {
        int DESTROY_RPC(void) = 1;
        int SET_VALUE_RPC(struct set_value_args) = 2;
        struct get_value_res GET_VALUE_RPC(string) = 3;
        int MODIFY_VALUE_RPC(struct set_value_args) = 4;
        int DELETE_KEY_RPC(string) = 5;
        int EXIST_RPC(string) = 6;
    } = 1;
} = 0x20000001;
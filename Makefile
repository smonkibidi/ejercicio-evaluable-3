# Variables de compilación
CC = gcc
CFLAGS = -Wall -g -I/usr/include/tirpc
LDLIBS = -lnsl -ltirpc

# Archivos generados por rpcgen
RPC_SOURCES = clavesRPC_clnt.c clavesRPC_svc.c clavesRPC_xdr.c
RPC_HEADERS = clavesRPC.h

.PHONY: all clean generate_rpc

all: generate_rpc libclaves.so servidor cliente

# REGLA OBLIGATORIA: Invocar rpcgen
generate_rpc: clavesRPC.x
	rpcgen -aNM clavesRPC.x

# Biblioteca compartida con la lógica de almacenamiento (Ejercicio 1 y 2)
libclaves.so: src/claves.c include/claves.h
	$(CC) $(CFLAGS) -fPIC -shared src/claves.c -o libclaves.so -lpthread

# El servidor ahora incluirá los ficheros _svc y _xdr generados [cite: 44]
servidor: clavesRPC_svc.c clavesRPC_xdr.c libclaves.so
	$(CC) $(CFLAGS) clavesRPC_svc.c clavesRPC_xdr.c -o servidor ./libclaves.so $(LDLIBS)

# El cliente (para pruebas)
cliente: tests/app-cliente.c libclaves.so
	$(CC) $(CFLAGS) tests/app-cliente.c -o cliente ./libclaves.so $(LDLIBS)

clean:
	rm -f servidor cliente libclaves.so $(RPC_SOURCES) $(RPC_HEADERS) *.o
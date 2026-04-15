# Variables de compilación
CC = gcc
CFLAGS = -Wall -g -I/usr/include/tirpc
LDLIBS = -lnsl -ltirpc

# Archivos generados por rpcgen
RPC_SOURCES = clavesRPC_clnt.c clavesRPC_svc.c clavesRPC_xdr.c
RPC_HEADERS = clavesRPC.h

.PHONY: all clean generate_rpc

all: generate_rpc libclaves.so servidor cliente

# Invocar rpcgen
generate_rpc: clavesRPC.x
	rpcgen -aNM clavesRPC.x

# Biblioteca compartida con la lógica de almacenamiento
libclaves.so: src/claves.c include/claves.h
	$(CC) $(CFLAGS) -fPIC -shared src/claves.c -o libclaves.so -lpthread

# EPara incluir los ficheros _svc y _xdr generados
servidor: clavesRPC_svc.c clavesRPC_xdr.c libclaves.so
	$(CC) $(CFLAGS) clavesRPC_svc.c clavesRPC_xdr.c -o servidor ./libclaves.so $(LDLIBS)

# El cliente
cliente: tests/app-cliente.c libclaves.so
	$(CC) $(CFLAGS) tests/app-cliente.c -o cliente ./libclaves.so $(LDLIBS)

clean:
	rm -f servidor cliente libclaves.so $(RPC_SOURCES) $(RPC_HEADERS) *.o
CC      = gcc
CFLAGS  = -Wall -g -I/usr/include/tirpc -Iinclude -I.
LDLIBS  = -ltirpc -lpthread

RPC_GEN_HDRS = clavesRPC.h
RPC_GEN_SRC  = clavesRPC_clnt.c clavesRPC_svc.c clavesRPC_xdr.c

.PHONY: all clean generate_rpc

all: libclaves.so libproxyclaves.so clavesRPC_server cliente

generate_rpc: clavesRPC.x
	rpcgen -NM clavesRPC.x

libclaves.so: src/claves.c include/claves.h
	$(CC) $(CFLAGS) -fPIC -shared src/claves.c -o libclaves.so $(LDLIBS)

clavesRPC_server: clavesRPC_svc.c clavesRPC_xdr.c clavesRPC_server.c libclaves.so
	$(CC) $(CFLAGS) clavesRPC_svc.c clavesRPC_xdr.c clavesRPC_server.c -o clavesRPC_server -L. -lclaves -Wl,-rpath,. $(LDLIBS)

libproxyclaves.so: src/proxy-rpc.c clavesRPC_clnt.c clavesRPC_xdr.c include/claves.h
	$(CC) $(CFLAGS) -fPIC -shared src/proxy-rpc.c clavesRPC_clnt.c clavesRPC_xdr.c -o libproxyclaves.so $(LDLIBS)

cliente: tests/app-cliente.c libproxyclaves.so
	$(CC) $(CFLAGS) tests/app-cliente.c -o cliente -L. -lproxyclaves -Wl,-rpath,. $(LDLIBS)

clean:
	rm -f clavesRPC_server cliente libclaves.so libproxyclaves.so *.o $(RPC_GEN_HDRS) $(RPC_GEN_SRC)
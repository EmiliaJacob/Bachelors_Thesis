mosquitto = /home/emi/mosquitto
ydb = $(shell pkg-config --variable=prefix yottadb)

all: ydb.so

ydb.so : ydb.c
	gcc -Wall -I $(mosquitto)/include  -I$(ydb) -L$(ydb) -shared -fPIC ydb.c -o ydb.so  -lyottadb -Wl,-rpath=$(ydb)

clean:
	-rm -f *.o ${PLUGIN_NAME}.so *.gcda *.gcno


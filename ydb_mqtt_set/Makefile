 # 20220908
ydb=$(shell pkg-config --variable=prefix yottadb)
mosq=/home/emi/mosquitto/
copt= -I $(mosq)/include/  -I $(mosq)/client -I $(ydb) -Wall
lopts= -L$(ydb) -lyottadb

ydb-mqtt-fetch:
	gcc $(mosq)/client/sub_client.o \
			ydb-mqtt-set.o \
			$(mosq)/client/client_shared.o \
			$(mosq)/client/client_props.o \
			$(lopts) \
			/media/sf_websocketfreigabe/mosquitto-master/lib/libmosquitto.so.1

       
clean:
		rm -f *.o

clean-all: clean
		rm -f $(executables)

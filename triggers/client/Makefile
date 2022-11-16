mosq=/home/emi/mosquitto
ydb=$(shell pkg-config --variable=prefix yottadb)
copts= -I $(mosq)/include -I $(ydb)
lopts= -L $(ydb) $(mosq)/lib/libmosquitto.so.1

mqttClient: 
	gcc -c -Wall -Werror -fpic mqttClient.c $(copts) 
	gcc -o libmqttclient.so -shared mqttClient.o $(lopts) \
	/home/emi/mosquitto/lib/libmosquitto.so.1 -lyottadb

clean:
	rm -f *.o

clean-all:
	rm -f $(executables)
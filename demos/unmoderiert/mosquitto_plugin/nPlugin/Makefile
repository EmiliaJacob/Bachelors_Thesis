
mosquitto = /home/emi/mosquitto

all: eP

eP : emisPlugin.c
	gcc -I $(mosquitto)/include -I . -shared -fPIC eventCallbacks.c nPlugin.c -o nPlugin.so /home/emi/mosquitto/lib/libmosquitto.so.1

clean:
	-rm -f *.o ${PLUGIN_NAME}.so *.gcda *.gcno


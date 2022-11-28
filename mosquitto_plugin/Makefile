mosquitto = /home/emi/mosquitto
ydb=$(shell pkg-config --variable=prefix yottadb)
prefix=$(shell pwd)

cfg=mosquitto_yottadb.conf

plugin.so: plugin.cpp 
	g++ -Wall -I $(mosquitto)/include  -I$(ydb) -L$(ydb) -I ../dependencies/ydb-global/ -I ../dependencies/jsoncpp/include/ ../dependencies/jsoncpp/jsoncpp.o ../dependencies/ydb-global/ydb-global.o -shared -fPIC plugin.cpp -o plugin.so  -lyottadb -Wl,-rpath=$(ydb)

jsoncpp.o: ./jsoncpp/jsoncpp.cpp
	g++ -fPIC -c ./jsoncpp/jsoncpp.cpp -I ./jsoncpp/include

.PHONY: clean
clean:
			rm -f plugin.so
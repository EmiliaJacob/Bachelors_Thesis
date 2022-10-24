ydb=$(shell pkg-config --variable=prefix yottadb)

trigger_trigger: trigger_trigger.cpp
	g++ trigger_trigger.cpp -I$(ydb) -I /home/emi/ydbay/aabay/mosquitto_plugin /home/emi/ydbay/aabay/mosquitto_plugin/ydb-global.o -L$(ydb) -lyottadb

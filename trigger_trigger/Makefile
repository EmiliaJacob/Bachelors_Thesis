ydb = $(shell pkg-config --variable=prefix yottadb)
loptions = -L$(ydb) -I ../dependencies
coptions = -I$(ydb)

trigger_trigger: trigger_trigger.cpp
	g++ trigger_trigger.cpp -o trigger_trigger  -I /home/emi/ydbay/aabay/mosquitto_plugin /home/emi/ydbay/aabay/mosquitto_plugin/ydb-global.o  -lyottadb

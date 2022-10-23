ydb=$(shell pkg-config --variable=prefix yottadb)

trigger_trigger: trigger_trigger.cpp
	g++ trigger_trigger.cpp -I$(ydb) -I .. ../ydb-global.o -L$(ydb) -lyottadb

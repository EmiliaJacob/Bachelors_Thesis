ydb = $(shell pkg-config --variable=prefix yottadb)
loptions = -L$(ydb) -I /home/emi/ydbay/dependencies/ydb-global/
coptions = -I$(ydb)

set_global_var: set_global_var.cpp
	g++ set_global_var.cpp -o set_global_var  $(loptions)  /home/emi/ydbay/dependencies/ydb-global/ydb-global.o  -lyottadb
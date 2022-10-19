ydb=$(shell pkg-config --variable=prefix yottadb)

mq_trigger: mq_trigger.c
	gcc -c -o mq_trigger.o -Wall -fpic mq_trigger.c -I$(ydb)
	gcc -o libmqtrigger.so -shared mq_trigger.o -L$(ydb) -lyottadb
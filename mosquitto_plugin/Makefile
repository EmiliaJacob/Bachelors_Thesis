mosquitto = /home/emi/mosquitto
ydb=$(shell pkg-config --variable=prefix yottadb)
#
# Prefixpfad anpassen wenn nÃ¶tig.
#
prefix=$(shell pwd)

cfg=mosquitto_yottadb.conf

plugin.so: plugin.cpp 
	g++ -Wall -I $(mosquitto)/include  -I$(ydb) -L$(ydb) -I ../dependencies/ydb-global/ -I ../dependencies/jsoncpp/include/ ../dependencies/jsoncpp/jsoncpp.o ../dependencies/ydb-global/ydb-global.o -shared -fPIC plugin.cpp -o plugin.so  -lyottadb -Wl,-rpath=$(ydb)

jsoncpp.o: ./jsoncpp/jsoncpp.cpp
	g++ -fPIC -c ./jsoncpp/jsoncpp.cpp -I ./jsoncpp/include

mosquitto_yottadb.conf:
			echo "Create $@ file."
			sed \
			 -e 's:@PREFIX@:$(prefix):g' \
			 -e 's:@YDB_DIR@:${ydb_dir}:g' \
			 -e 's:@YDB_GBLDIR@:${ydb_gbldir}:g' \
			 -e 's:@YDB_ROUTINES@:${ydb_routines}:g' \
			 -e 's:@YDB_REL@:${ydb_rel}:g' \
			 $@.tmpl > $@

.PHONY: clean
clean:
			rm -f plugin.so
			rm -f mosquitto_yottadb.conf

mosquitto = /home/emi/mosquitto
ydb=$(shell pkg-config --variable=prefix yottadb)
#
# Prefixpfad anpassen wenn nÃ¶tig.
#
prefix=$(shell pwd)

cfg=mosquitto_yottadb.conf

plugin.so: plugin.c
			g++ -Wall -I $(mosquitto)/include  -I$(ydb) -L$(ydb) ydb-global.h ydb-global.o -shared -fPIC plugin.c -o plugin.so  -lyottadb -Wl,-rpath=$(ydb)

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

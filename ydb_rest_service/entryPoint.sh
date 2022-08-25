. /opt/yottadb/current/ydb_env_set
source ./node_modules/nodem/resources/environ
export ydb_routines="/app/ydb_triggers $ydb_routines"
ls /data/r1.34_x86_64/o/utf8
mupip trigger -triggerfile=./ydb_triggers/testTrig.trig
./fifoReadScript.sh&
node index.js
#/bin/bash


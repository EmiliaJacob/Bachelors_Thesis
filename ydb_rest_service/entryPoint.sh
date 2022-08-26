. /opt/yottadb/current/ydb_env_set

source ./node_modules/nodem/resources/environ

cp ./ydb_triggers/*.m /app/node_modules/nodem/src

# mupip trigger -triggerfile=./ydb_triggers/testTrig.trig
mupip trigger -triggerfile=/app/ydb_triggers/deactivate.trig

mosquitto&

sleep 4

(node index.js)&

./fifoReadScript.sh&

 mosquitto_sub -t /test

#/usr/bin/bash

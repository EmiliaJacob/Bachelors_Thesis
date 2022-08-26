. /opt/yottadb/current/ydb_env_set

source ./node_modules/nodem/resources/environ

cp ./ydb_triggers/*.m /app/node_modules/nodem/src

mupip trigger -triggerfile=/app/ydb_triggers/deactivateArticle.trig

mosquitto&

sleep 4

(node index.js)&

./readFifo.sh&

(mosquitto_sub -t /test)&

/usr/bin/bash

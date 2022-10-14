source /opt/yottadb/current/ydb_env_set

source /app/node_modules/nodem/resources/environ

cp /app/ydb_triggers/*.m /app/node_modules/nodem/src

mupip trigger -triggerfile=/app/ydb_triggers/test.trig

mupip trigger -triggerfile=/app/ydb_triggers/deactivateArticle.trig

mosquitto&

sleep 4

(node index.js)&

/app/readFifo.sh&

(mosquitto_sub -t /deactivated)&
(mosquitto_sub -t /test)&

/usr/bin/bash
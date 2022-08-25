. /opt/yottadb/current/ydb_env_set
source /root/nodem/resources/environ
cp ./ydb_triggers/*.m /data/r1.34_x86_64/o/utf8
ls /data/r1.34_x86_64/o/utf8
mupip trigger -triggerfile=./ydb_triggers/testTrig.trig
./fifoReadScript.sh&
node index.js


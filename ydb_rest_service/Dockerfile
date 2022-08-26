FROM yottadb/yottadb-base:latest

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install curl make g++ -y

RUN curl -sL https://deb.nodesource.com/setup_16.x -o /tmp/nodesource_setup.sh

RUN bash /tmp/nodesource_setup.sh

RUN apt-get install nodejs -y

RUN apt-get install mosquitto mosquitto-clients -y

WORKDIR /app

COPY package.json . 

RUN . /opt/yottadb/current/ydb_env_set && npm install 

COPY . .

ENTRYPOINT ./entryPoint.sh

#RUN npm install nodemon -g

#RUN . /opt/yottadb/current/ydb_env_set && mupip trigger -triggerfile=./ydb_triggers/test_trig.trig

#CMD /usr/bin/bash

#ENTRYPOINT . /opt/yottadb/current/ydb_env_set && source ~/nodem/resources/environ && node --verbose index.js& && ./fifoReadScript&

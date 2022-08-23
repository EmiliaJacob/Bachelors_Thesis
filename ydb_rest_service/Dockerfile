FROM yottadb/yottadb-base:latest

ENV ydb_dist=/opt/yottadb/current

RUN apt-get update && apt-get install curl make g++ -y

RUN curl -sL https://deb.nodesource.com/setup_16.x -o /tmp/nodesource_setup.sh

RUN bash /tmp/nodesource_setup.sh

RUN apt-get install nodejs -y

WORKDIR /app

COPY . .

RUN . /opt/yottadb/current/ydb_env_set && npm install 

RUN npm install nodemon -g

#CMD /usr/bin/bash
#ENTRYPOINT /usr/bin/bash -c ". /opt/yottadb/current/ydb_env_set ; echo source ~/nodem/resources/environ"
ENTRYPOINT . /opt/yottadb/current/ydb_env_set && echo "source ~/nodem/resources/environ" >> ~/.profile && nodemon --verbose index.js

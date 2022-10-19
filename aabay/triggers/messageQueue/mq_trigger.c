#include <libyottadb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <mqueue.h>

#define MQ_NAME "/mqttspool"
#define DELIMITER " "
#define MQ_MSG_PRIO 1

void addMqttMessage(int count, ydb_char_t *topic, ydb_char_t *payload) // TODO: Do I have to cast the ydb-types / Can I?
{
    int mq_descriptor = mq_open(MQ_NAME, O_WRONLY | O_CREAT | O_NONBLOCK, S_IRWXU, NULL);
    
    if(mq_descriptor == -1) {
        int latest_errno = errno;
        printf("%s %s\n", strerrorname_np(latest_errno), strerror(latest_errno));
        return;
    }

    char *mq_message = (char*)malloc(strlen(topic) + strlen(DELIMITER) +strlen(payload) + 1);

    strcpy(mq_message, topic);
    strcat(mq_message, DELIMITER);
    strcat(mq_message, payload);

    int mq_sending_result = mq_send(mq_descriptor, mq_message, sizeof(mq_message), MQ_MSG_PRIO);

    if(mq_sending_result == -1) {
        int latest_errno = errno;
        printf("%s %s\n", strerrorname_np(latest_errno), strerror(latest_errno));
    }

    free(mq_message);

    return;
}
#include <libyottadb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <time.h>
#include <mqueue.h>

#include <unistd.h>

#include <signal.h>

#define MQ_NAME "/mqttspool" 
#define DELIMITER " "
#define MQ_MSG_PRIO 1

struct mq_attr mqttspool_attributes = {
    .mq_maxmsg = 100,
    .mq_msgsize = 8192 // default groesse in Ubuntu 20
};

void sig_handler(int signo)
{
    printf("%d SIGNAL \n", signo);
}

void addMqttMessage(int count, ydb_char_t *topic, ydb_char_t *payload) 
{
    //signal(SIGALRM, sig_handler);


    static int mq_descriptor = -1;
    
    if(mq_descriptor == -1) {
        mq_descriptor = mq_open(MQ_NAME, O_WRONLY | O_CREAT , S_IRWXU, &mqttspool_attributes); 

        if(mq_descriptor == -1) {
            perror("mq_open failed");
            return;
        }
    }

    char *mq_message = (char*)malloc(strlen(topic) + strlen(DELIMITER) +strlen(payload) + 1);

    strcpy(mq_message, topic);
    strcat(mq_message, DELIMITER);
    strcat(mq_message, payload);

    int mq_sending_result = mq_send(mq_descriptor, mq_message, strlen(mq_message), MQ_MSG_PRIO);

    if(mq_sending_result == -1) {
        int latest_errno = errno;
        perror("mq_send failed");

        while(latest_errno == EINTR) { // mq_send wurde von signal unterbrochen 
            printf("Trying mq_send again\n");

            mq_sending_result = mq_send(mq_descriptor, mq_message, strlen(mq_message), MQ_MSG_PRIO);

            if(mq_sending_result == -1)
                latest_errno = errno;
            else {
              break;
            }
        }
    }

    free(mq_message);

    return;
}

//#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <libyottadb.h>
#include <mosquitto.h>

static int get_time(struct tm **ti, long *ns);

extern struct mosq_config cfg;
extern struct mosquitto *g_mosq;

#include <client_shared.h>
// int ydb_ci_tab_open(char *fname, uintptr_t *ret_value);

void print_message(struct mosq_config *lcfg, const struct mosquitto_message *message, const mosquitto_property *properties) {
	printf("Topic %s Nachricht %s\n",message->topic, (const char *)message->payload); // Dangerous, is payload null-terminated???
	ydb_buffer_t	glo, val, msg;
	char valuebuff[64];
	int	status, nr;
	
	val.buf_addr = valuebuff;
	val.len_alloc = sizeof(valuebuff);
	
	YDB_LITERAL_TO_BUFFER("^MQTT", &glo);
	status = ydb_incr_s(&glo, 0, NULL, NULL, &val);
	val.buf_addr[val.len_used] = '\0';
	nr = atoi(valuebuff);
	printf("inc-status=%d\nvalue=%d\n", status, nr);
	
	msg.buf_addr = message->payload, msg.len_alloc = msg.len_used = message->payloadlen;
	status = ydb_set_s(&glo, 1, &val, &msg);
	printf("set status=%d\n", status);

	
	fflush(stdout);
}

 
static int get_time(struct tm **ti, long *ns)
{
#ifdef WIN32
	SYSTEMTIME st;
#elif defined(__APPLE__)
	struct timeval tv;
#else
	struct timespec ts;
#endif
	time_t s;
	
#ifdef WIN32
	s = time(NULL);
	
	GetLocalTime(&st);
	*ns = st.wMilliseconds*1000000L;
#elif defined(__APPLE__)
	gettimeofday(&tv, NULL);
	s = tv.tv_sec;
	*ns = tv.tv_usec*1000;
#else
	if(clock_gettime(CLOCK_REALTIME, &ts) != 0){
		//err_printf(&cfg, "Error obtaining system time.\n");
		return 1;
	}
	s = ts.tv_sec;
	*ns = ts.tv_nsec;
#endif
	
	*ti = localtime(&s);
	if(!(*ti)){
		//err_printf(&cfg, "Error obtaining system time.\n");
		return 1;
	}
	
	return 0;
}


void rand_init(void)
{
#ifndef WIN32
	struct tm *ti = NULL;
	long ns;
	
	if(!get_time(&ti, &ns)){
		srand((unsigned int)ns);
	}
#endif
}

int main()
{
}

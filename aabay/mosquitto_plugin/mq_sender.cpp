#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <iostream>

using namespace std;
int main()
{
    mq_unlink("/mqttspool");
	int mq_d = mq_open("/mqttspool", O_WRONLY | O_CREAT | O_NONBLOCK, S_IRWXU, NULL); //

	if(mq_d == -1) {
		int errsv = errno;
		cout << "open: " << strerrorname_np(errsv) << "  " << strerror(errsv) << endl;
		return 0;
	}

    int success = mq_send(mq_d, "hello", sizeof("hello"), 1);
    if(success == -1) {
		int errsv = errno;
		cout << strerrorname_np(errsv) << "  " << strerror(errsv) << endl;
		return 0;
    }
    return 0;
}
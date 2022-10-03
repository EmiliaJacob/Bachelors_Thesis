#include <libyottadb.h>

void createClientAndPublish(int count, ydb_char_t *topic, ydb_char_t *payload);

int main()
{
  createClientAndPublish(1, "j","jj");
}

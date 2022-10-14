#include <libyottadb.h>
#include <stdio.h>

int main() 
{
  ydb_buffer_t globalVar, subscript, value;
  char globVal[2] = "^y";

  //YDB_LITERAL_TO_BUFFER("^y", &globalVar);
  globalVar.buf_addr = &globVal;
  globalVar.len_used = 2;
  globalVar.len_alloc = sizeof(globVal);

  YDB_LITERAL_TO_BUFFER("d", &subscript);
  YDB_LITERAL_TO_BUFFER("b", &value);

  int result = ydb_set_s(&globalVar, 1, &subscript, &value);

  if(result == YDB_OK)
    printf("SUCCESS\n");

  return 0;
}

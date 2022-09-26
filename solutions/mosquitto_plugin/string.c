#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

int main()
{
  char *prependix= "^";
  char *name= "hello";
  size_t global_size = strlen(prependix) + strlen(name) + 1;
  char *global = malloc(global_size);
  strcat(global, prependix);
  strcat(global, name);

  printf("%s\n", global);

  return 0;
}

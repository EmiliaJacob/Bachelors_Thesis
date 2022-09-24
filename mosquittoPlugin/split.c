#include <string.h>
#include <stdio.h>

int main()
{
  char *alphabet = "a/b/c/d";
  char alphabetArray[sizeof(char*)];

  strcpy(alphabetArray, alphabet);
  
  const char *delim = "/";
  char *token;

  token = strtok(alphabetArray, delim);

  while(token != NULL) {
    printf("%s\n", token);
    token = strtok(NULL, delim);
  }

  return 0;
}

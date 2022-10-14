#include <string.h>
#include <stdio.h>
#include <fnmatch.h> 
 
// char *concatenate_string(int number_of_strings, char *strings[]) // TODO: vllt double pointer array um type zu geawehrleisten
// {
// 	char *result = "\0";
	
// 	for(int i=0; i<number_of_strings; i++) {
// 		result = strcat(result, strings[i]);
// 	}
// 	return result;
// }
 
int main() {
 //concatenate();
 int result = fnmatch("hello", "heldlo", FNM_NOESCAPE);
 
 printf("%d\n", result);
 return 0;
}
/*#include<stdio.h>
#include<string.h>
int main()
{
	char str[20]={"hello world"};
	char *p=str;
	char *q=NULL;
	q=strsep(&str," ");
	printf("strsep === %s\n",q);


}*/
#include <stdio.h>
#include <string.h>
 
int main(void) {
	char source[] = "hello, world! welcome to china!";
	char delim[] = " ,!";
 
	char *s = strdup(source);
	char *token;
	for(token = strsep(&s, delim); token != NULL; token = strsep(&s, delim)) {
		printf(token);
		printf("+");
	}
	printf("\n");
	return 0;
}

#include "../basics.h"

int main(int argc, char** argv){

	char c1[] = "Hello";
	char *c2 = c1;
	printf("%c\n",c2[1]);
	printf("%c\n",*(c2+1));
	c2 = c2 + 1;
	printf("%c\n",*(c1+1));
	return 0;
}

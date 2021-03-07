#include "../basics.h"
#include "mycode.h"

int main(int argc, char** argv){
	if (argc>1){
		printf("%s\n",argv[1]);
		printf("%s\n",reverse(argv[1]));
	} 
	
	//rpthread_t tid;
	//rpthread_create(&tid, NULL,(void*)foo,NULL);
	return 0;
}

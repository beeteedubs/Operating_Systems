#include "basics.h"
unsigned ctx_id = 0;
int main(){
	unsigned ctx_cur = ctx_id;
	unsigned ctx_next = (ctx_id + 1) & 0x03;
	ctx_id = (ctx_id + 1) & 0x03;
	printf("ctx_id:%d, ctx_cur:%d, ctx_next:%d\n",ctx_id,ctx_cur,ctx_next);
	return 0;
}

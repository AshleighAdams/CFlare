#include "cflare/handle.h"
#include "cflare/util.h"


int main(int argc, char** argv)
{
	cflare_handle_load();
	
	cflare_handle hd = cflare_handle_new("test", 0, 0);
	printf("hd = %lu\n", hd);
	cflare_handle_unreference(hd);
	
	cflare_handle_unload();
	return 0;
}

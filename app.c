#include "server.h"

int main(int argc, char **argv){
	int ret;
	ret = parseMainArg(argc, argv);
	if(ret){
		return 0;
	}
	ret = setUpListener(Status.Port, Status.Address);
	if(ret){
		printf("Runtime error::::Probably could not set up server...:(\n");
		return -1;
	}
	printf("Serving\n");
	while(true){
		ret = server(&Status);
	}
	ret = destroyListener(&Status);
	if( Status.Listening && Status.Accepting){
		ret = destroyListener(&Status);
	}
	printf("App closed.\n");
	return 0;
}

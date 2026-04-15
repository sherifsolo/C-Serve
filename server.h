#ifndef SERVER_H
#define SERVER_H
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sendfile.h> 
#include <sys/epoll.h>
#include <fcntl.h>
#include <signal.h>
#define DEFAULT_PORT 80
#define DEFAULT_ADDRESS "0.0.0.0"
#define MAX_CONNECTIONS 60000

typedef struct {
	char *Request;
	char *Headers;
	char *Body;
	const char *RequestMethod;
	char *Path;
	int PathSize;
	char Data[1024];
	bool KeepAlive;
}REQUEST;

typedef struct {
	int ClientId;
	int FileDescriptor;
	char Address[20];
	REQUEST *Request;
	bool KeepAlive;
}CLIENT;
typedef struct {
 	char *Address;
 	int Port;
 	int FileDescriptor;
 	bool Listening;
 	bool Accepting;
 	bool Peers; //PRESERVED
 	int  PeerCount; //PRESERVED
 	char *ServingDirectory;
 	struct sockaddr_in ActiveClientsAddr[MAX_CONNECTIONS];
 	long long int ActiveClients;
 	CLIENT Clients[MAX_CONNECTIONS];
}SERVER_STATUS;


// extern --- should be used for variables in .h files since they migth be
// included in multiple .c files resulting in redefination of variables which causes linking errors
//REQUEST METHODS
extern const char *Get;
extern const char *Head;
extern const char *Options;
extern const char *Post;
extern const char *Put;
//FILE EXTENTIONS
extern const char *CssExt;
extern const char *JpegExt;
extern const char *JsExt;
extern const char *GifExt;
extern const char *HTMLExt;
extern const char *PngExt;
extern const char *SvgExt;
extern const char *TsExt;
//VARIABLES
extern const char* NotFound;
extern const int NotFoundLen;
//EPOLL
extern int EpollFd;
extern struct epoll_event EpollEvent;
extern struct epoll_event EpollEventQueue[MAX_CONNECTIONS + 1];
extern long long int CurrentConnections;
extern struct sockaddr_in SocketTemplate;
extern SERVER_STATUS Status;

extern const char * Dummy;
int extractPostData();
int extractQueryData();
int destroyListener(SERVER_STATUS *); //  --- program ending      -----------DONE
int handleClient(CLIENT *); // ---  called in program mail loop --- handles clients after they have been accept()ed --- 
int parseHeaders(REQUEST *); // called to process the headers of a request 
int parseMainArg(int, char**); // ---- called by main to parse its arguments after process starts(can be ignored ) --- not a priority ---------- DONE
int parseRequest(REQUEST *); // called by handleClient() to process a request
int printSatatus(SERVER_STATUS *);
int router(CLIENT *);
int sendCSS(int, char*);  // sends a CSS file back to client only sets ContetType header to the correct MIMETYPE 
int sendDummyHTTP(int);
int sendHTML(int, char*); // sends HTML file back to client only sets ContentType header to the correct MIMETYPE  
int sendJavascript(int, char*); //sends Javascript fil back to the client by setting ContentType header  to the correct MIMETYPE 
int server(SERVER_STATUS *); //main loop 
int setUpListener(int, char *); // --- called after arguments have been parsed --- Critical any error process fails  --1 ----------DONE


/*

start/run main()--> setUpListener() ---> [ main loop ] while(accept() ) { handleClient() ---> parseRequest() ---> parseHeader() ---> sendHTML() / sendCSS() / sendJavascript()  } destroyListener()

*/
//TO DO
#endif

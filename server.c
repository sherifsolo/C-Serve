#include "server.h"
//REQUEST METHODS
const char *Get = "GET";
const char *Head = "HEAD";
const char *Options = "OPTIONS";
const char *Post = "POST";
const char *Put = "PUT";
//FILE Extensions
const char *CssExt = ".css";
const char *JpegExt = ".jpeg";
const char *JsExt = ".js";
const char *GifExt = ".gif";
const char *HTMLExt = ".html";
const char *PngExt = ".png";
const char *SvgExt = ".svg";
const char *TsExt = ".ts";
//VARIABLES

int EpollFd = 0;
struct epoll_event EpollEvent = {0};
struct epoll_event EpollEventQueue[MAX_CONNECTIONS + 1] = {0};

struct sockaddr_in SocketTemplate = {0};
SERVER_STATUS Status;
long long int CurrentConnections = 0;
const char * Dummy = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>Served By a Server written in C</title></head><body><p>This page was served from a server <span>3NCRYPT3D C0D3R<span> is writtig in C programming language.</p></body></html>";
const int DummyLen = 297;
const char *NotFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\n404 Not Found";
const int NotFoundLen = 85;
//FUNCTIONS
int parseMainArg(int Count, char **Options){
	char *CurrentArgument;
	if(Count == 1){
		Status.Port = DEFAULT_PORT;
		Status.Address = DEFAULT_ADDRESS;
		return 0;
	}
	for(int X = 1; X < Count; X += 2){
		CurrentArgument = Options[X];
		if(*CurrentArgument != '-'){
			printf("Usage: ./app.exe {-P [ Port ] } { -A [ Address] } { -T [n] } { -D [ serving directory ] }\n\n");
			return -1;
		}
		CurrentArgument++;
		if(*CurrentArgument == 'P'){
			Status.Port = (atoi(Options[X+1]) > 0) ? atoi(Options[X+1]) : DEFAULT_PORT ;
		}else if(*CurrentArgument == 'A'){
			Status.Address = Options[X+1];
		}else{
			printf("Usage: ./app.exe {-P [ Port ] } { -A [ Address] } { -T [n] } { -D [ serving directory ] }\n\n");
			return -1;
		}
	}
	return 0;
}
int printStatus(SERVER_STATUS *Stats){
	if(!Stats){
		return -1;
	}
	printf(" status for server socket ==> \n\tAddress : %s\n\tPort : %d  \n\tSocket file descriptor : %d\n\tListening-4-Connections : %d\n\tAccepting-Connections : %d\n", Stats->Address, Stats->Port, Stats->FileDescriptor, Stats->Listening, Stats->Accepting);
	return 0;
}
int setUpListener(int Port, char *Address){
	int Prt;
	char *Addr;
	int SocketFd;
	int Ret, Active;
	if(Port == 0){	
		Prt = DEFAULT_PORT;
	}else{
		Prt = Port;
	}
	if(Address == NULL){
		Addr = DEFAULT_ADDRESS;
		Status.Address = Addr;
	}else{
		Addr = Address;
	}
	signal(SIGPIPE, SIG_IGN);
	EpollFd = epoll_create1(0);
	if(EpollFd <= 0){
		perror("Failed to create an epoll instance::epoll_create(int size)::ERROR::");
		return -1;
	}
	printf("Successfully set epoll instance::Epoll instance Fd =>  %d\n", EpollFd);
	SocketTemplate.sin_family = AF_INET;
	SocketTemplate.sin_port = htons(Prt);
	SocketTemplate.sin_addr.s_addr = inet_addr(Addr);
	SocketFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if( SocketFd == -1){
		perror("Failed to get a socket file descriptor.:::ERROR::");
		return -1;
	}
	Active = 1;
	if(setsockopt(SocketFd, SOL_SOCKET, SO_REUSEADDR, &Active, sizeof(Active)) < 0){
		perror("setsockopt(REUSEADDR) failed");
	}
	Prt = bind(SocketFd, (struct sockaddr*)&SocketTemplate, sizeof(SocketTemplate));
	if(Prt == -1){
		perror("Failed to bind to socket address:::ERROR::");
		return -1;
	}
	Prt = listen(SocketFd, MAX_CONNECTIONS);
	if(Prt == -1){
		perror("Failed to set up Listener:::ERROR::");
		return -1;
	}
	EpollEvent.data.fd = SocketFd;
	EpollEvent.events = EPOLLIN;
	
	Ret = epoll_ctl(EpollFd, EPOLL_CTL_ADD, SocketFd, &EpollEvent);
	printf("Added a socket to event queue %d ---> %d\n", SocketFd, Ret);
	Status.FileDescriptor = SocketFd;
	Status.Listening = true;
	Status.Accepting = true;
	Status.Peers = false;
	Status.PeerCount = 0;
	Status.ActiveClients = 0;
	printf("Succesfully set up TCP listener::details below.\n");
	printStatus(&Status);
	return 0;
}
int destroyListener(SERVER_STATUS* Listener){
	int SockFd;	
	Listener->Address = NULL;
	Listener->Port = 0;
	SockFd = Listener->FileDescriptor;
	Listener->Listening = false;
	Listener->Accepting = false;
	//handle threads in the future
	close(EpollFd);
	close(SockFd);
	return 0;
}

int server(SERVER_STATUS *ServerInstance){
	int SockFd;
	socklen_t PeerAddrLen;
	struct sockaddr_in *PeerAddr;
	char *Temp;
	int PeerSockFd, ReadyFd;
	int Ret, Flags;
	struct in_addr *Addr;
	SERVER_STATUS *Server;
	CLIENT *Peer;
	Server = ServerInstance;
	SockFd = Server->FileDescriptor;
	PeerAddr = Server->ActiveClientsAddr;
	PeerAddrLen = sizeof(struct sockaddr_in);
	Peer = Server->Clients;
	while(1){
		ReadyFd = 0;
		ReadyFd = epoll_wait(EpollFd, EpollEventQueue, MAX_CONNECTIONS, -1);
		//printf("Got an epoll event\n");
		for(int Count = 0; Count < ReadyFd; Count++){
			if(EpollEventQueue[Count].data.fd == Server->FileDescriptor){
				//add socket to event queue
				printf("\nRecieved a connection\n");
				PeerSockFd = accept(SockFd, (struct sockaddr *)PeerAddr, &PeerAddrLen);
				Flags = fcntl(PeerSockFd, F_GETFL, 0);
				Flags |= O_NONBLOCK;
				if(fcntl(PeerSockFd, F_SETFL, Flags) == -1){
					perror("Failed to set client socket to SOCK_NONBLOCK::Using blocking socket\r\n\tThis will cause the server to under perform::UPTIME 25%% gaurantee.::ERROR::");
				}
				EpollEvent.events = EPOLLIN;
				EpollEvent.data.ptr = Peer;
				Ret = epoll_ctl(EpollFd, EPOLL_CTL_ADD, PeerSockFd, &EpollEvent);
				if(Ret == -1){
					perror("Failed to add socket to epoll event watchdog");
				}
				printf("Added a socket to event queue %d ---> %lld\n", PeerSockFd, Server->ActiveClients);
				printf("Client Details\t");
				Peer->ClientId = Server->ActiveClients;
				Peer->FileDescriptor = PeerSockFd;
				Peer->Request = NULL;
				Addr = &PeerAddr->sin_addr;
				Temp = inet_ntoa(*Addr);
				strncpy(Peer->Address, Temp, sizeof(Peer->Address)-1);
				printf("Client address: %s connection id : %lld\n", Peer->Address, CurrentConnections);
				CurrentConnections++;
				Server->ActiveClients = CurrentConnections;
				Peer++;
				PeerAddr++;
			}else{	
				if(EpollEventQueue[Count].data.ptr){
					printf("\nRecieved a request\n");
					Ret = handleClient(EpollEventQueue[Count].data.ptr);
				}
			}
					
		}
	}
	return 0;
}
int handleClient(CLIENT *Master){
	char Window[1024];
	int WindowSize = 1023;
	CLIENT *Client;
	REQUEST *Request; //should we malloc? / will it be passed as a copy of the original
	int SockFd;
	ssize_t ReceivedBytes;
	//bool KeepAlive;
	int Ret;
	Request = (REQUEST *)malloc(sizeof(REQUEST));
	if(Request == NULL){
		perror("\tFailed to allocate memory while handling request::handleclient(CLIENT *):::ERROR::");
		return -1;
	}
	memset(Request, 0, sizeof(REQUEST));
	Window[WindowSize] = '\0';
	Request->Data[WindowSize] = '\0';
	memset(Window,0,WindowSize);
	memset(Request->Data,0,WindowSize);
	Client = (CLIENT *)Master;
	SockFd = Client->FileDescriptor;
	Client->Request = Request;
	memset(Window, 0, WindowSize);

	ReceivedBytes = read(SockFd, Window, WindowSize);
	if(ReceivedBytes  == 0){
		printf("\tClient Id ->  %d\n", Client->ClientId);
		perror("\tConection closed::");
		Client->Request = NULL;
		free(Request);
		epoll_ctl(EpollFd, EPOLL_CTL_DEL, SockFd, NULL);
		close(SockFd);
		return -1;
	}
	
	if (ReceivedBytes == -1) {
    		if (errno == EAGAIN || errno == EWOULDBLOCK) {
    			Client->Request = NULL;
    			free(Request);
        		return 0;
    		}else {
        		printf("\tClient Id ->  %d\n", Client->ClientId);
        		perror("\tHandle Client when recv()::ERROR::");
        		epoll_ctl(EpollFd, EPOLL_CTL_DEL, SockFd, NULL);
        		close(SockFd);
        		Client->Request = NULL;
        		free(Request);
        		return -1;
    		}
	}
	printf("received %zd B:\n", ReceivedBytes);
	if(ReceivedBytes <= 0){
		        epoll_ctl(EpollFd, EPOLL_CTL_DEL, SockFd, NULL);
        		close(SockFd);
        		Client->Request = NULL;
        		free(Request);
        		return -1;
	}
	memcpy(Request->Data, Window, ReceivedBytes);
	//printf("%s\n\n", Request->Data);
	Request->Request = Request->Data;
	Ret = parseRequest(Request);
	
	//wrong request method used:: Close the connection immediately
	if(Ret == 5){
		printf("\tConnection closed Client used unknown method\n\n");
		free(Request);
		Client->Request = NULL;
		epoll_ctl(EpollFd, EPOLL_CTL_DEL, SockFd, NULL);
		close(SockFd);
		return 0;
	}
	//parseHeaders(Request);
	router(Client);
	Client->Request = NULL;	
	free(Request);
	/*impliment parse headers first if(!Client->KeepAlive){
		close(SockFd);	
	}*/
	epoll_ctl(EpollFd, EPOLL_CTL_DEL, SockFd, NULL);
	close(SockFd);
	return 0;
}
int parseHeaders(REQUEST *Request){
	//if coonection:Close close client socket and end the connection after processing and returning data if requested
	//obtain platform, browser, referer;
	return 0;
}
//url decode paths with special characters /BAD%20BOYZ%20CLUB%20-%20%20Buruklyn%20Boyz%20X%20Double%20Trouble,%20%20Big%20Yasa%20X%20Young%20NC%20[D8_N7fiVQgE].webm
int parseRequest(REQUEST *Request){
	char *Data, *Temp;
	int  Size;
	Temp = Data = Request->Request;
	if(!Request || !Request->Request) return -1;
	for(Size = 0; *Temp != '\r' && *Temp; Temp++ ){
		//printf("%c", *Temp);
		Size +=1;
	}
	printf("\tRequest size %d B\n", Size);
	Request->Headers = Temp + 2;
	if(*Request->Headers == '\r' || *Request->Headers == '\n' || *Request->Headers == '\0'){
		Request->Headers = Request->Data; //no headers case
	}
	if ( strncmp(Data, Get, 3) == 0 ){
		Request->RequestMethod = Get;
		printf("\tMethod: %s\n", Get);
		Data += 4;
		Size -= 4;
	}else if( strncmp(Data, Post, 4) == 0 ){
		Request->RequestMethod = Post;
		printf("\tMethod: %s\n", Post );
		Data += 5;
		Size -= 5;
	}else if ( strncmp(Data, Head, 4) == 0 ){
		Request->RequestMethod = Head;
		printf("\tMethod: %s\n", Head);
		Data += 5;
		Size -= 5;
	}else if( strncmp(Data, Options, 7) == 0 ){
		Request->RequestMethod = Options;
		printf("\tMethod: %s\n", Options);
		Data += 8;
		Size -= 8;
	}else if( strncmp(Data, Put, 3) == 0 ){
		Request->RequestMethod = Put;
		printf("\tMethod: %s\n", Put);
		Data += 4;
		Size -= 4;
	}else {
		return 5; 
	}
	Request->Path = Data;
	Request->PathSize = Size - 8;
	for(int X = Request->PathSize; X > 0 ; Data++){
		if(*Data == ' '){
			*Data = '\0';
		}
		X--;
	}
	Request->PathSize -= 1;
	printf("\tRequest path: %s size : %d\n", Request->Path, Request->PathSize);
//GET HEAD OPTIONS POST PUT 
	return 0;
}
int router(CLIENT *Client){
	char *Temp, *SecondPathBackslash, *File, *QueryStart, *FragementStart, *Extension;
	int ClientSocketFd, PathSize;
	REQUEST *Request; 
	char RelativePath[255];
	Temp = SecondPathBackslash = File = QueryStart = FragementStart = Extension = NULL;
	if(Client == NULL){
		perror("Can not process data for non existing client:: router(CLIENT *) invalid request data address\n");
		return -1;
	}
	printf("\tRouting client to resource\n");
	Request = Client->Request;
	ClientSocketFd = Client->FileDescriptor;
	File = Temp = Request->Path + 1;
	PathSize = Request->PathSize - 1;
	for(int C = 0; C < PathSize ; Temp++, C++){
		if(*Temp == '/'){
			SecondPathBackslash = Temp;
		}
		if(*Temp == '?'){
			//query
			QueryStart = Temp;
			*Temp = '\0';
		}
		if(*Temp == '.'){
			Extension = Temp;
		}
		//printf("[ %c ], ", *Temp);
	}
	//we know if location, query, fragements are present
	if(QueryStart != NULL){
		//{ log/logapi?v=2&admin=true&secret=111111}
	}
	if(*Request->Path == '/' && *(Request->Path + 1) == '\0'){
		if(strcmp(Request->RequestMethod, Get) == 0 ){
			sendHTML(ClientSocketFd, "index.html");
			Client->KeepAlive = true;
			return 0;
		}else if(strncmp(Request->RequestMethod, Post, 4) == 0){
			extractPostData();
			//more post logic later
		}else{
			sendDummyHTTP(ClientSocketFd);
		}
	}
	if(SecondPathBackslash  != NULL && (strncmp(Request->RequestMethod, Get, 3) == 0)){
		if(Extension != NULL){
			//printf("2nd / && ext present [ %c ] \n", *(Request->Path + PathSize));
			sendHTML(ClientSocketFd, File);
			return 0;
		}else if(*(Request->Path + PathSize) != '/'){
			//printf("2nd / &&  no ext path ends with no / [  %c ]\n", *(Request->Path + PathSize));
			snprintf(RelativePath, 255, "./%s/index.html", File);
			sendHTML(ClientSocketFd, RelativePath);
			return 0;		
		}else if( *(Request->Path + PathSize)  == '/'){
			//printf("2nd / && no ext but path ends with a / [ %c ] \n", *(Request->Path + PathSize));
			snprintf(RelativePath, 255, "./%sindex.html", File);
			sendHTML(ClientSocketFd, RelativePath);
		}
		return 0;
	}
	if(SecondPathBackslash  == NULL && (strncmp(Request->RequestMethod, Get, 3) == 0)){
		if(Extension != NULL){
			//printf("No 2nd / && ext present [ %c ] \n", *(Request->Path + PathSize));
			sendHTML(ClientSocketFd, File);
			return 0;
		}
		//printf("No 2nd / && no ext [ %c ]\n", *(Request->Path + PathSize));
		snprintf(RelativePath, 255, "./%s/index.html", File);
		sendHTML(ClientSocketFd, RelativePath);
		return 0;
	}
	if(strncmp(Request->RequestMethod, Post, 4) == 0){
		extractPostData();
	}
	return 0;
}//func
//query data application/x-www-form-urlencoded
int extractQueryData(){
	return 0;
}
//multipart/form-data
//json
int extractPostData(){
	return 0;
}
int sendCss(int SocketFileDescriptor, char*FileName){
	return 0;
}

//handle large files later 
//biggest upload so far was 4.12 MB (10.24 MB  with curl and 22.91MB with wget) others uploads range from 3.44 MB - 3.77 MB on browser files larger than this are not sent completely 
//only one successfull upload of 772.38M video file with wget as client after trial and error 
//a70b45188a74b563f742ebd8ad40546f0adf91e2.mp4     [                   <=>                                                              ] 772.38M  26.3MB/s    in 30s  
int sendHTML(int SocketFileDescriptor, char*FileName){
	FILE *FileDescriptor;
	size_t BytesSent, TotalBytesSent;
	const char *MIMEType = "HTTP/1.1 200 OK\r\nContent-Type: */*\r\n\r\n";//this handles mimetype there might be no need to implement other functions that handle different mimitypes
	int HeaderLen = strlen(MIMEType);
	BytesSent = TotalBytesSent = 0;
	if(SocketFileDescriptor <= 0){
		perror("Failed to send data to invalid socket in sendHTML(int, char *)::ERROR::");
		return -1;
	}
	FileDescriptor = fopen(FileName, "r");
	if (FileDescriptor == NULL){
		printf("\t[ Not Found ] Failed to open file [ %s ] during sendHTML(int, char*)\n", FileName);
		write(SocketFileDescriptor, NotFound, NotFoundLen);
		return -1;
	}
	//printf("Sending request\n");
	//send headers first
	write(SocketFileDescriptor, MIMEType, HeaderLen);
	//zero read --- sendfile(socket_fd, file_fd, NULL, file_size);
	while ((BytesSent = sendfile(SocketFileDescriptor, fileno(FileDescriptor), NULL, 4096)) > 0) {
		if(BytesSent == -1 ){
			break;
		}
		TotalBytesSent += BytesSent;
		//printf("Bytes read  from file : Bytes sent to client socket : %ld\n", BytesSent);
	}
	printf("\tRequest sent, Total Bytes transferred: [ %zd B]\n\n", TotalBytesSent);
	fclose(FileDescriptor);
	return 0;
}
int sendJavascript(int SocketFileDescriptor, char*FileName){
	return 0;
}
int sendDummyHTTP(int SocketFileDescriptor){
	const char *MIMEType = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	int HeaderLen = 44;
	if(!SocketFileDescriptor || SocketFileDescriptor < 0){
		perror("Bad file descriptor in sendDummyHTTP(int).\n");
		return -1;
	}
	write(SocketFileDescriptor, MIMEType, HeaderLen);
	write(SocketFileDescriptor, Dummy, DummyLen);
	return 0;	
}

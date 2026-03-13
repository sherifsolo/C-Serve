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
struct sockaddr_in SocketTemplate = {};
SERVER_STATUS Status;
const char * Dummy = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>Served By a Server written in C</title><head><body><p>This page was served from a server <span>3NCRYPT3D C0D3R<span> is writtig in C programming language.</p></body></html>";
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
	for(int X = 1; X < Count -1; X += 2){
		CurrentArgument = Options[X];
		if(*CurrentArgument != '-'){
			printf("Usage: ./app.exe {-P [ Port ] } { -A [ Address] } { -T [n] } { -D [ serving directory ] }\n\n");
			return -1;
		}
		CurrentArgument++;
		if(*CurrentArgument == 'P'){
			Status.Port = atoi(Options[X+1]);
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
	printf(" status for server socket ==> \n\n\tAddress : %s\n\n\tPort : %d  \n\n\tSocket file descriptor : %d\n\n\tListening-4-Connections : %d\n\n\tAccepting-Connections : %d\n\n", Stats->Address, Stats->Port, Stats->FileDescriptor, Stats->Listening, Stats->Accepting);
	return 0;
}
int setUpListener(int Port, char *Address){
	int Prt;
	char *Addr;
	int SocketFd;	
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
	SocketTemplate.sin_family = AF_INET;
	SocketTemplate.sin_port = htons(Prt);
	SocketTemplate.sin_addr.s_addr = inet_addr(Addr);
	SocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if( SocketFd == -1){
		printf("Failed to get a socket file descriptor\n");
		return -1;
	}
	Prt = bind(SocketFd, (struct sockaddr*)&SocketTemplate, sizeof(SocketTemplate));
	if(Prt == -1){
		printf("Failed to bind to socket to address : %s:%d probably already in use \n", Addr, Port);
		return -1;
	}
	Prt = listen(SocketFd, MAX_CONNECTIONS);
	if(Prt == -1){
		printf("Failed to set up Listener:::ERRNO == %d", errno);
		return -1;
	}
	Status.FileDescriptor = SocketFd;
	Status.Listening = true;
	Status.Accepting = true;
	Status.Peers = false;
	Status.PeerCount = 0;
	Status.ActiveClients = 0;
	//Status.ActiveClientsAddr = 0;  memset 
	printf("Succesfully set up TCP listener::details below\n\n");
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
	close(SockFd);
	return 0;
}

int server(SERVER_STATUS *ServerInstance){
	int SockFd;
	socklen_t PeerAddrLen;
	struct sockaddr_in *PeerAddr;
	char *Temp;
	int PeerSockFd, CurrentConnections;
	struct in_addr *Addr;
	SERVER_STATUS *Server;
	CLIENT *Peer;
	Server = ServerInstance;
	SockFd = Server->FileDescriptor;
	PeerAddr = Server->ActiveClientsAddr;
	PeerAddrLen = sizeof(struct sockaddr_in);
	Peer = Server->Clients;
	printf("Serving\n");
	PeerSockFd = 0 ;
	CurrentConnections = 0;
	while ((PeerSockFd = accept(SockFd, (struct sockaddr *)PeerAddr, &PeerAddrLen)) >= 0){
		printf("\nRecieved a connection\n");
		printf("Client Details\t");
		Peer->ClientId = Server->ActiveClients;
		Peer->FileDescriptor = PeerSockFd;
		Peer->Request = NULL;
		Addr = &PeerAddr->sin_addr;
		Temp = inet_ntoa(*Addr);
		strncpy(Peer->Address, Temp, sizeof(Peer->Address)-1);
		printf("Client address: %s connection id : %d\n", Peer->Address, CurrentConnections);
		handleClient(Peer);
		CurrentConnections++;
		Server->ActiveClients = CurrentConnections;
		if(CurrentConnections == MAX_CONNECTIONS){
			Peer = Server->Clients;
			PeerAddr = Server->ActiveClientsAddr;
			CurrentConnections = 0;
		}{
			Peer++;
			PeerAddr++;
		}
	}
	printf("Loop\n");
	return 0;
}
int handleClient(CLIENT *Master){
	char Window[1024];
	int WindowSize = 1023;
	int Ret;
	CLIENT *Client;
	REQUEST *Request; //should we malloc? / will it be passed as a copy of the original
	int SockFd;
	ssize_t ReceivedBytes;
	bool KeepAlive;
	Request = malloc(sizeof(REQUEST));
	if(Request == NULL){
		printf("Failed to allocate memory while handling request::handleclient(CLIENT *)\n");
		return -1;
	}
	Window[1023] = '\0';
	Request->Data[1023] = '\0';
	Client = Master;
	SockFd = Client->FileDescriptor;
	Client->Request = Request;
	memset(Window, 0, 1023);
	KeepAlive = true;
	ReceivedBytes = read(SockFd, Window, 1023);
	if(ReceivedBytes  <= 0){
		printf("Could not read from client socket handleClient(CLIENT *)");
	}
	memcpy(Request->Data, Window, ReceivedBytes);
	printf("received %dB:\n", ReceivedBytes);
	//printf("%s\n\n", Request->Data);
	Request->Request = Request->Data;
	Ret = parseRequest(Request);
	//wrong request method used:: Close the connection immediately
	if(Ret == 5){
		close(SockFd);
		printf("Connection closed Client used unknown method\n");
		return 0;
	}
	parseHeaders(Request);
	router(Client);
	//conti
	//we know : req method, path, headers
	free(Request);
	close(SockFd);
	return 0;
}
int parseHeaders(REQUEST *Request){
	//if coonection:Close close client socket and end the connection after processing and returning data if requested
	//obtain platform, browser, referer;
}
int parseRequest(REQUEST *Request){
	char *Data, *Temp;
	int  Size;
	Temp = Data = Request->Request;
	if(!Request || !Request->Request) return -1;
	for(Size = 0; *Temp != '\r' && *Temp; Temp++ ){
		//printf("%c", *Temp);
		Size +=1;
	}
	printf("\tRequest size %dB\n", Size);
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
	//call a cgi by parsing the request path { log/logapi?v=2&admin=true&secret=111111} variables { key=pair&key=pair } from the path
	//if method = POST extract data from the request and return it
	char *Temp, *SecondPathBackslash, *File;
	int ClientSocketFd;
	REQUEST *Request;
	char *QueryStart;
	char *FragementStart;
	char *Extension;
	char RelativePath[255];
	if(Client == NULL){
		printf("Can not process data for non existing client:: router(CLIENT *) invalid request data address\n");
		return -1;
	}
	Request = Client->Request;
	ClientSocketFd = Client->FileDescriptor;
	if(*Request->Path == '/' && *(Request->Path + 1) == '\0'){
		sendHTML(ClientSocketFd, "index.html");
		return 0;
	}
	QueryStart, FragementStart, SecondPathBackslash, Extension = NULL;
	File = Temp = Request->Path + 1;
	for(int C = 1; C < Request->PathSize; Temp++, C++){
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
	Temp -=1;
	//we know location, query, fragements are present 
	if(SecondPathBackslash != NULL && *Request->RequestMethod == *Get){
		if(Extension){
			//printf("2nd / && ext present\n");
			sendHTML(ClientSocketFd, File);
			return 0;
		}else if(*Temp != '/'){
			//printf("2nd / &&  no ext path ends with no / ");
			snprintf(RelativePath, 255, "./%s/index.html", File);
			sendHTML(ClientSocketFd, RelativePath);
			return 0;		
		}else if( *Temp == '/'){
			//printf("2nd / && no ext but path ends with a / %c \n", *--Temp);
			snprintf(RelativePath, 255, "./%sindex.html", File);
			sendHTML(ClientSocketFd, RelativePath);
		}
		return 0;
	}
	if(SecondPathBackslash  == NULL && *Request->RequestMethod == *Get){
		//printf("no 2nd / && no ext\n");
		snprintf(RelativePath, 255, "./%s/index.html", File);
		sendHTML(ClientSocketFd, RelativePath);
	}
	return 0;
}
int sendCss(int SocketFileDescriptor, char*FileName){

}
int sendHTML(int SocketFileDescriptor, char*FileName){
	char *Data;
	FILE *FileDescriptor;
	size_t DataSize, DataLen, BytesRead, BytesSent, TotalBytesSent;
	const char *MIMEType = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
	int HeaderLen = 44;
	DataLen = 2048;
	DataSize =1;
	TotalBytesSent = 0;
	if(SocketFileDescriptor <= 0){
		printf("Failed to send data to invalid socket in sendHTML(int, char *)\n");
		return -1;
	}
	Data = (char *)malloc(DataLen);
	if(Data == NULL){
		printf("Failed to allocate memory:: sendHTML(int, char *)\n");
		return -1;
	}
	FileDescriptor = fopen(FileName, "r");
	if (FileDescriptor == NULL){
		printf("[ Not Found ] Failed to open file [ %s ] during sendHTML(int, char*)\n", FileName);
		write(SocketFileDescriptor, NotFound, NotFoundLen);
		free(Data);
		return -1;
	}
	//send headers first later
	//printf("Sending request\n");
	
	//zero read --- sendfile(socket_fd, file_fd, NULL, file_size);
	write(SocketFileDescriptor, MIMEType, HeaderLen);
	//sendfile(SocketFileDescriptor, fileno(FileDescriptor), NULL, 4096);
	while ((BytesSent = sendfile(SocketFileDescriptor, fileno(FileDescriptor), NULL, 4096)) > 0) {
		TotalBytesSent += BytesSent;
		//printf("Bytes read  from file : %d\tBytes sent to client socket : %d\n", BytesRead, BytesSent);
	}
	
	printf("\tRequest sent, Total Bytes transferred: [ %d B]\n\n", TotalBytesSent);
	free(Data);
	return 0;
}
int sendJavascript(int SocketFileDescriptor, char*FileName){
}

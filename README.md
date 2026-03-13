A minimalis Web Server written in the C programming language.

about me:
	Author: Solomon Muchiri
	mail to: Sherifsolo9113@gmail.com
 	Education: Degree in Applied Computing
 	
{this is a LINUX IMPLIMENTATION }

	server.h --- header file --- contains all server side variables and function declarations.
	server.c ---- source file (server engine)--- contains all server side variables and functions definations and implementations. 
	app.c ---- our app source file --- borrows most basic server side implimentations provided by server.c  
	  	   leaving your to handle request and data processing. 
	   
PROCESS EXECUTION FLOW

	start/run main() ---> parseMainArgs() --> setUpListener()  ---> main loop [ Server() while(accept() ) { handleClient() ---> parseRequest() ---> parseHeader() ---> sendHTML() / sendCSS() / sendJavascript()  } ] ---end---> destroyListener()


RUNNING PROGRAM

The program accepts command line arguments -P, -A, [support for more later].
	where: 
		-P  denotes the port to server on { defaiult -P 80}
		-A denotes the address the server will use to serve { default -A 0.0.0.0} binds on all network interfaces. 
	
	./app.exe [ -P { port } ] [ -A {address} ]
	
		eg; ./app.exe -P 8080 -A 127.0.0.1 
		    ./app.exe is the same as ./app.exe -P 80 -A 0.0.0.0 


This program servers the current working directory it was run from:

	*Clients get only read access
	*Request methods supported
		1. GET --- fully functional
		2. POST --- there's need to extract and process the data sent
		3. HEAD --- TODO
		4. OPTIONS --- TODO
		5.PUT --- TODO
		
		(2, 3, 5 & 5) ---- Handle this yourself, tailor it to your needs
	*Errors are handled gracefully -- not found requests
	
COMPILATION

if you have your own app.c implementation just link with server.c 
ordinary linking 
	gcc -o app.exe app.c server.c

object file linking
	first compile to object files 
	gcc -c app.o app.c
	gcc -c server.o server.c
	then link the object files into an executable
	gcc app.o server.o -o app.exe
 

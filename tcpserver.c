#include <stdio.h>														// Code based on the https://github.com/mafintosh/echo-servers.c/blob/master/tcp-echo-server.c
#include <stdlib.h>														// Open license
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "iocontroller.h"												// Responsible for GPIO
#include "inputprocessing.h"											// Responsible for processing user's inputs

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

// Client should initiate connection by entering:
// nc IPADRESS PORTNUMBER
// where IPADRESS is the IP adress of the server and PORTNUMBER is the portnumber that the server has opened a socket at

int main (int argc, char *argv[]) {
	if (argc < 2) on_error("Usage: %s [port]\n", argv[0]);				// The port used
	
	int port = atoi(argv[1]);											// Converts the 2nd argument to int
	
	int server_fd;														// Server socket
	int client_fd;														// Client socket
	int err;															// Variable responsible for error handling of writing messages
	struct sockaddr_in server;
	struct sockaddr_in client;
	char buf[BUFFER_SIZE];												//Buffer that stores client sent messages
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);						//Creating a socket on the server side
	if (server_fd < 0) on_error("Could not create socket\n");
	
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int opt_val = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);
	
	err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
	if (err < 0) on_error("Could not bind socket\n");
	
	err = listen(server_fd, 128);
	if (err < 0) on_error("Could not listen on socket\n");
	
	printf("Server is listening on %d\n", port);
	
	while (1) {
		socklen_t client_len = sizeof(client);
		client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

		if (client_fd < 0) on_error("Could not establish new connection\n");
		
		button();
		//printf("whatever\n");

		while (1) {
		  memset(&buf[0], 0, sizeof(buf));								// Clearing the buffer before receiving the next message
		  int read = recv(client_fd, buf, BUFFER_SIZE, 0);				// Reading the message sent by client

		  if (!read) break; 											// Reading client's message
		  if (read < 0) on_error("Client read failed\n");
		  printf(buf);

		  process(buf);

		  err = send(client_fd, buf, read, 0);							// Echoing the message
		  if (err < 0) on_error("Client write failed\n");
		  
		  char message[] = "Server's response\n";						// Sending a static response
		  err = send(client_fd, message, strlen(message), 0);
		  if (err < 0) on_error("Client write failed\n");
		  
		  //printf(buf);													// Printing out client's message (debugging)
		}
	  }
	
	return 0;
	}

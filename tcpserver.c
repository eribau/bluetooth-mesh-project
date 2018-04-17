#include <stdio.h>														// Code based on the https://github.com/mafintosh/echo-servers.c/blob/master/tcp-echo-server.c
#include <stdlib.h>														// Open license
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "iocontroller.h"												// Responsible for GPIO
#include "inputprocessing.h"											// Responsible for processing user's inputs
#include "l2cap-server.h"												// l2cap bluetooth low energy server

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

typedef enum {false, true} bool;

void delay(int);
static int *global_variable;
bool connection_check = false;
extern int ble_client;

/**
 TCP RPi server that has a LED and a button connected to it.
 Able to send and receive messages from a remote client.

 Client should initiate connection by entering:
 nc IPADRESS PORTNUMBER
 where IPADRESS is the IP adress of the server;
 PORTNUMBER is the portnumber that the server has opened a socket at. **/
int main (int argc, char *argv[]) {
    global_variable = mmap(NULL, sizeof *global_variable, PROT_READ | PROT_WRITE,
                           MAP_SHARED | MAP_ANONYMOUS, -1, 0);					// Creating shareable memory for all of the child processes

    *global_variable = 1;
    pid_t childpid;
    
    	ble_client = ble_server();

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

            client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len); // This is a blocking line, waits until a socket is accepted
            if (client_fd < 0) on_error("Could not establish new connection\n");
            connection_check = true;

            if ((childpid = fork()) == 0) {
                while (1) {
                    memset(&buf[0], 0, sizeof(buf));						// Clearing the buffer before receiving the next message
                    int read = recv(client_fd, buf, BUFFER_SIZE, 0);		// Reading the message sent by client

                    if (!read) break; 										// Reading client's message
                    if (read < 0) on_error("Client read failed\n");
                    printf(buf);

                    toggle_led(buf);											// This one toggles the led (Function is in inputprocessing.c)

                    err = send(client_fd, buf, read, 0);					// Echoing the message
                    if (err < 0) on_error("Client write failed\n");
                    
                    //
					printf("ble_client : %d\n", ble_client);
					printf("buffer: %s\n", buf);
					write(ble_client, buf, read);	
					//
                    char message[] = "Server's response\n";					// Sending a static response
                    err = send(client_fd, message, strlen(message), 0);
                    if (err < 0) on_error("Client write failed\n");
                }
            }
            else {
                printf("CHECK\n");
                button(client_fd);											// Calls the button method for turning lights on/off
            }
            connection_check = false;
        }
	
    
		
    
    return 0;
}

#include <stdio.h>														 
#include <stdlib.h>														
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "iocontroller.h"												// Responsible for GPIO
#include "inputprocessing.h"											// Responsible for processing user's inputs
#include "l2cap-server.h"												// l2cap bluetooth low energy server

#define BUFFER_SIZE 1024
#define on_error(...) { fprintf(stderr, __VA_ARGS__); fflush(stderr); exit(1); }

void delay(int);
static int *g_global_memory;
bool g_connection_check = false;
int g_ble_client;

/**
 TCP RPi server that works as a gateway between a TCP client
 and a BLE connected device.
 When running tcpserver it should take a port number as an argument.
 The TCP client should initiate connection by entering:
 IPADRESS PORTNUMBER
 In a network utility (such as telnet or netcat),
 where IPADRESS is the IP adress of the server;
 PORTNUMBER is the portnumber that the server has opened a socket at. 
**/
int main (int argc, char *argv[]) {
    g_global_memory = mmap(NULL, sizeof *g_global_memory, PROT_READ | 			// Creating shareable memory for all of the child processes
						PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);			

    *g_global_memory = 1;
    pid_t childpid;														

    g_ble_client = ble_server();											// ble_server (l2cap-server.c)
	

    if (2 > argc) on_error("Usage: %s [port]\n", argv[0]);				

    int port = atoi(argv[1]);											// Converts the 2nd argument to int
    int server_fd;														// Server socket
    int client_fd;														// Client socket
    int err;															// Variable responsible for error handling of writing messages
    struct sockaddr_in server;
    struct sockaddr_in client;
    char buf[BUFFER_SIZE];												//Buffer that stores client sent messages

    server_fd = socket(AF_INET, SOCK_STREAM, 0);						//Creating a socket on the server side
    if (0 > server_fd) on_error("Could not create socket\n");

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server)); // Bind socket#
    if (0 > err) on_error("Could not bind socket\n");

    err = listen(server_fd, 128);
    if (0 > err) on_error("Could not listen on socket\n");

    printf("Server is listening on %d\n", port);

    while (1) {	
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, 		// This is a blocking line, waits until a socket is accepted
											&client_len); 
        if (0 > client_fd) on_error("Could not establish new connection\n");
        g_connection_check = true;

        if (0 == (childpid = fork())) {									// Fork, one process handles communication with pc terminal, the other gets to be a buttonlistener. 
            while (1) {
                memset(&buf[0], 0, sizeof(buf));						// Clearing the buffer before receiving the next message
				int read = recv(client_fd, buf, BUFFER_SIZE, 0);		// Reading the message sent by client

                if (!read) break; 										// Reading client's message
                if (0 > read) on_error("Client read failed\n");
                printf(buf);

                toggle_led(buf);										// This one toggles the led (Function is in inputprocessing.c)

                err = send(client_fd, buf, read, 0);					// Echoing the message
                if (0 > err) on_error("Client write failed\n");

                printf("g_ble_client : %d\n", g_ble_client);
                printf("buffer: %s\n", buf);
                write(g_ble_client, buf, read);
                char message[] = "Server's response\n";					// Sending a static response
                err = send(client_fd, message, strlen(message), 0);
                if (0 > err) on_error("Client write failed\n");
            }
        } else {
            printf("CHECK\n");
            if (0 == (childpid = fork())) {								// Fork, one process listens to messages from pi2, the other one listens to messages from button.
                char *pimessage;
                while(1) {
					if(false == g_connection_check) exit(1);
						pimessage = ble_read(g_ble_client);					// ble_read (l2cap-server) 
						printf("Pi2 Button Pressed: %s\n", pimessage);
                    char message[] = "Pi2 says hi 	\n";				// Sending a static response
                    send(client_fd, message, strlen(message), 0);
                    free(pimessage);
                }
            } else{
				button(client_fd);											// Calls the button (iocontroller) method for turning lights on/off
			}

        }
        g_connection_check = false;
    }

    return 0;
}

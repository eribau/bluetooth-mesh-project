#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <wiringPi.h>

#include "iocontroller.h"

#define ATT_CID 4																			// ATT_CID = 4, For l2cap socket to use BLE

/**
  The client-side first hardcodes a destination address of the adapter 
  it is connecting to. This method then opens a socket, and then binds 
  it to the first available physical bluetooth adapter on the chip. 
  After, it also sets up configurations for the destination address. 
  It then attemps to connect to the destination address specified. 
  It can then read and write data in the connection with the server.
  This node will act as a slave. 
  **/
typedef enum {false, true} bool;
bool g_connection_check = true;

int main(int argc, char **argv)
{
    struct sockaddr_l2 loc_addr = { 0 }; 													// Local bluetooth address
    struct sockaddr_l2 rem_addr = { 0 };													// Remote bluetooth address
    socklen_t opt = sizeof(rem_addr);
    int connection_socket;
    int connection_fd; 
    int bytes_read;
    int device_id;
    int device_descriptor;
    int advertise;
    char buf[1024] = { 0 };
    char buf_input[1024] = { 0 };
    pid_t childpid;
    
    struct timeval tv;						//Allocate timeout for read() in socket options
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	device_id = hci_get_route(NULL);
    device_descriptor = hci_open_dev(device_id);

    connection_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);				// Allocate a socket
    
    loc_addr.l2_family = AF_BLUETOOTH;														// Set up local bluetooth adapter
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_cid = htobs(ATT_CID);                                   					// ATT_CID = 4, For l2cap to use BLE
    loc_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    
    bind(connection_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));				// Bind socket
    
	advertise = hci_le_set_advertise_enable(device_descriptor, 1, 10000);								// Advertise LE
	printf("leadv on %d\n" , advertise);
	
	listen(connection_socket, 10);
	
	connection_fd = accept(connection_socket, (struct sockaddr *)&rem_addr, &opt);		// Accept a connection from the server
	
	setsockopt(connection_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);	// Set socket options for read() to use a timeout
	
	ba2str( &rem_addr.l2_bdaddr, buf );													// Print bluetooth address of the server 
	fprintf(stderr, "accepted connection from %s\n", buf);
	
    
    if (0 == (childpid = fork())) {
		if (0 == (childpid = fork())) {
			while(1) {
				if(0 < connection_fd) {																	
					memset(buf, 0, sizeof(buf));			
					bytes_read = read(connection_fd, buf, sizeof(buf));							//Read a message from the server
					//strtok(buf, "\n");
					if (0 < bytes_read) {
						printf("%s\n", buf);			
					}
					char *point = strstr(buf, "toggleLED\n");
					printf("hejs%s\n", point);
					if(0 == strcmp("toggleLED\n", point)) {							//Potentially toggle the LED
						toggle_led();
					}
					delay(100);
				} else {
					close(connection_socket);
				}
			}
		} else {
			wiringPiSetup();
			pinMode (2, INPUT);													// Sets button as an input
			int prev_button = LOW;												// Last state of the pull-up circuit
			while (1) {
				if(g_connection_check == false) exit(0);							// If connection to the client was closed, terminate
				if(prev_button == HIGH && digitalRead(2) == LOW) {				// A falling edge
					prev_button = LOW;
					char reply[] = "button pressed\n";
					write(connection_fd, reply, sizeof(reply));				// [Debugging] Send message to client (Maxsize 32)
					printf("Server message\n");									// [Debugging] Send message to server
				}
				else if(prev_button == LOW && digitalRead(2) == HIGH) {			// a rising edge, do nothing
					prev_button = HIGH;
				}
				delay(100);
			}
		}
	} else {
		while(1) {
			memset(buf_input, 0, sizeof(buf_input));						// Send messages to the server
			fgets(buf_input, sizeof(buf_input), stdin);
			write(connection_fd, buf_input, strlen(buf_input)-1);
		}
	}
}


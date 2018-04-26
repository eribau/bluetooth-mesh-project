#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <wiringPi.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define ATT_CID 4 																		// For l2cap socket to use BLE

#define KNRM  "\x1B[0m"																	// Color for terminal outputs
#define KRED  "\x1B[91m"
#define KGRN  "\x1B[92m"
#define KYEL  "\x1B[93m"
#define KBLU  "\x1B[94m"
#define KMAG  "\x1B[95m"
#define KCYN  "\x1B[96m"
#define KWHT  "\x1B[97m"
#define BOLD  "\x1B[1m"
#define UNBOLD  "\x1B[21m"

void delay(unsigned int);
typedef enum {false, true} bool;

/** 
 Takes in an array of bluetooth addresses, which are slaves to
 connect to. Creates a new socket for each connection. Returns a
 connection socket for each connection.
**/
int socket_creator(char arr[], struct sockaddr_l2 loc_addr, struct sockaddr_l2 rem_addr){
	
	int status;
	int connection_socket;
	int bind_status;
	int bytes_read;
	
	struct timeval tv;												//Timeout for socket options for read() to be nonblocking
	tv.tv_sec = 0;
	tv.tv_usec = 100000;	
	
	while(1){
		connection_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);            // Initialize socket
		if (-1 == connection_socket) {
			perror("connection_socket");
		}
	
		bind_status = bind(connection_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));            // Binds socket
		if (-1 == bind_status) {
			perror("bind_status");
		}
		
		setsockopt(connection_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);		// Set a timeout, so read() is nonblocking
		
		str2ba(arr, &rem_addr.l2_bdaddr);													// Converts BT address string to BT address
		
		status = connect(connection_socket, (struct sockaddr *)&rem_addr, sizeof(rem_addr));	// Connect to client
		if (-1 == status) {
			printf(KRED "-----Pi %s failed to connect-----\n" KNRM, arr);
			perror(KRED "status" KNRM);
			close(connection_socket);
		}
		if (0 == status) {
			printf(KGRN "Connection socket value: %d\n" KNRM, connection_socket);
			printf(KGRN "Pi %s connected\n" KNRM, arr);
			break;
		}
	}
	return connection_socket;
}

/**
	This method sets up its local bluetooth adapter and the type of the
	remote bluetooth adapter it will connect to. It then connects to all
	of the hardcoded bluetooth addresses, then forks into two processes
	where one is reading data and one is writing data with all connections.
**/
int main(int argc, char *argv[]) {
    struct sockaddr_l2 loc_addr = {0};												// Local bluetooth address
    struct sockaddr_l2 rem_addr = {0};												// Remote bluetooth address	
    int bytes_read;
    int connections[8];
    char buf[1024] = {0};
    char buf_input[1024] = {0};																// Buffer for reading data	
    bool single_message = false;
    bool single_pi_message = false;
    pid_t childpid;
    
    char arr [8][18] = {
		"B8:27:EB:9B:D4:87", 	// pi1 
		"B8:27:EB:E4:D7:BF", 	// pi2
		"B8:27:EB:4F:D6:56", 	// pi4
		"B8:27:EB:DD:39:F9", 	// pi5
		"B8:27:EB:52:65:92", 	// pi6
		"B8:27:EB:EF:F7:B8", 	// pi7
		"B8:27:EB:F4:3B:BA", 	// pi8
		"B8:27:EB:15:3D:99"};	// pi9			
		
    loc_addr.l2_family = AF_BLUETOOTH; 									// Setup for local (server) BLE adapter
    loc_addr.l2_bdaddr = *BDADDR_ANY;													// Bind socket to the first available bluetooth adapter
    loc_addr.l2_cid = htobs(ATT_CID);                                   				// ATT_CID = 4, For l2cap to use BLE
	loc_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;									// Random, public or 0.		

    rem_addr.l2_family = AF_BLUETOOTH;											// Setup for remote BLE adapter
    rem_addr.l2_cid = htobs(ATT_CID);
    rem_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;   
	
	for (int i = 0; i < 8; i++) {											// 8 is the size of the hard coded array with BT addresses
		printf(BOLD KBLU "Attempting to connect with: " UNBOLD KYEL BOLD "%s\n" UNBOLD KNRM, arr[i]);
		connections[i] = socket_creator(arr[i], loc_addr, rem_addr);
	}
	
	if ((childpid = fork()) == 0) {										// Fork for reading and writing to the clients
		while(1) {
			printf("Type in the BT address or the message to every connection: \n");
			memset(buf_input, 0, sizeof(buf_input));					// Empty the buffer
			fgets(buf_input, sizeof(buf_input), stdin);					// Input
			strtok(buf_input, "\n");

			for (int j = 0; j < 8; j++) {								// Check if message is for a specific client
				if (strcmp(buf_input, arr[j]) == 0) {
					single_message = true;
					printf("Write your message to: %s \n", arr[j]);
					memset(buf_input, 0, sizeof(buf_input));
					fgets(buf_input, sizeof(buf_input), stdin);
					write(connections[j], buf_input, strlen(buf_input)-1);
				}
			}
			
			if (single_message == false) {
				for (int i = 0; i < 8; i++) {									// 8 is the size of the hard coded array with BT addresses
					write(connections[i], buf_input, strlen(buf_input));		// Send a message to all clients
				}
			}
			single_message = false;
		}
	} else {
		
		while(1) {
			for(int i = 0; i < 8; i++){
				memset(buf, 0, sizeof(buf));
				bytes_read = read(connections[i], buf, sizeof(buf));				//Non blocking read from all clients
				if(0 < bytes_read){
					printf(KWHT "%s\n" KNRM,buf);
					strtok(buf, "\n");
					for (int j = 0; j < 8; j++) {			
						if (strcmp(buf, arr[j]) == 0) {								// Check if message is for a specific client
							printf("Writing to: %s \n", arr[j]);
							while(1){
								memset(buf, 0, sizeof(buf));
								bytes_read = read(connections[i], buf, sizeof(buf));
								delay(100);
								if(0 < bytes_read)break;
							}
							
							write(connections[j], buf, strlen(buf));				// Send the message to the specific client
						}
					}
				}
			}
		}
	}
	return 0;
}

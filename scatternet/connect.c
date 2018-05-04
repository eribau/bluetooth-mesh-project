#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <wiringPi.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

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

#define ATT_CID 4 																		// For l2cap socket to use BLE

void delay(unsigned int);
typedef enum {false, true} bool;


/** 
 Takes in an array of bluetooth addresses, which are slaves to
 connect to. Creates a new socket for each connection. Returns a
 connection socket for each connection.
**/
int socket_creator(char *arr, struct sockaddr_l2 loc_addr, struct sockaddr_l2 rem_addr){
	
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
int connect_to_neighbour(char (*array)[18]) {
    struct sockaddr_l2 loc_addr = {0};												// Local bluetooth address
    struct sockaddr_l2 rem_addr = {0};												// Remote bluetooth address	
    int bytes_read;
    int connections[sizeof(array)];
    char buf[1024] = {0};
    char buf_input[1024] = {0};																// Buffer for reading data	
    bool single_message = false;
    bool single_pi_message = false;
    pid_t childpid;
    char temp [1024];			
		
    loc_addr.l2_family = AF_BLUETOOTH; 									// Setup for local (server) BLE adapter
    loc_addr.l2_bdaddr = *BDADDR_ANY;													// Bind socket to the first available bluetooth adapter
    loc_addr.l2_cid = htobs(ATT_CID);                                   				// ATT_CID = 4, For l2cap to use BLE
	loc_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;									// Random, public or 0.		

    rem_addr.l2_family = AF_BLUETOOTH;											// Setup for remote BLE adapter
    rem_addr.l2_cid = htobs(ATT_CID);
    rem_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;   
	
	for (int i = 0; i < sizeof(array) - 1; i++) {											// 8 is the size of the hard coded array with BT addresses
		printf(BOLD KBLU "Attempting to connect with: " UNBOLD KYEL BOLD "%s\n" UNBOLD KNRM, array[i]);
		connections[i] = socket_creator(array[i], loc_addr, rem_addr);
	}
	return 0;
}

int main(int argc, char *argv[]) {

	char arr [3][18] = {
		"B8:27:EB:4F:D6:56", 	// pi4
		"B8:27:EB:DD:39:F9", 	// pi5
		"B8:27:EB:52:65:92", 	// pi6
	};
	
	connect_to_neighbour(arr);

	return 0;
}


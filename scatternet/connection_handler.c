#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <wiringPi.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

//#include "connection_handler.h"	

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


static struct hci_dev_info di;
//static void print_dev_hdr(struct hci_dev_info *di);

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
	remote bluetooth adapter it will connect to. It then connects 
	to all the bluetooth address given as the input
**/
int* connect_to_neighbour(char (*array)[18]) {
    struct sockaddr_l2 loc_addr = {0};												// Local bluetooth address
    struct sockaddr_l2 rem_addr = {0};												// Remote bluetooth address	
    int bytes_read;
    static int connections[sizeof(array)];
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
	return connections;
}

/**
  The accept function does not take in any argument, if it is called it 
  will be blocking and accept the first connection that is sent to it
  **/
int accept_a_neighbour()
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
    //char dest[18] = "B8:27:EB:51:32:99"; //Pi3
    
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
	
	return connection_fd;
}

char* print_dev_hdr(struct hci_dev_info *di)
{
	static int hdr = -1;
	static char addr[18];

	/*if (hdr == di->dev_id)
		return;*/
	hdr = di->dev_id;

	ba2str(&di->bdaddr, addr);

	//printf("%s\n", addr);
	return addr;
}

char* print_own_bd_addr()
{
	int ctl;
	ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	
	ioctl(ctl, HCIGETDEVINFO, (void *) &di);
	
	printf("%s\n", print_dev_hdr(&di));
	
	return print_dev_hdr(&di);
	
}

//TEST MAIN
/*int main(int argc, char *argv[]) {
	
	
	print_own_bd_addr();
	
	
	//print_own_bd_addr();
	
	//print_dev_hdr(&di);
	/*char arr [3][18] = {
		"B8:27:EB:4F:D6:56", 	// pi4
		"B8:27:EB:DD:39:F9", 	// pi5
		"B8:27:EB:52:65:92", 	// pi6
	};
	
	if (strcmp(argv[1], "slave") == 0) {
		accept_a_neighbour();
	}
	if (strcmp(argv[1], "master") == 0) {
		connect_to_neighbour(arr);
	}
	
	//connect_to_neighbour(arr);

	return 0;
}*/

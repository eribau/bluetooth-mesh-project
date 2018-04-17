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
#include "inputprocessing.h"

#define ATT_CID 4																			// ATT_CID = 4, For l2cap socket to use BLE

/**
  The client-side first hardcodes a destination address of the adapter 
  it is connecting to. This method then opens a socket, and then binds 
  it to the first available physical bluetooth adapter on the chip. 
  After, it also sets up configurations for the destination address. 
  It then attemps to connect to the destination address specified. 
  It can then read and write data in the connection with the server. 
  **/
  typedef enum {false, true} bool;
  bool connection_check = true;																	// Temporary variable so we can compile

int main(int argc, char **argv)
{
    struct sockaddr_l2 loc_addr = { 0 }; 													// Local bluetooth address
    struct sockaddr_l2 rem_addr = { 0 };													// Remote bluetooth address
    int connection_socket;
    int status; 
    int bytes_read;
    char buf[1024] = { 0 };
    char dest[18] = "B8:27:EB:9B:D4:87";													// Destination address
    pid_t childpid;
	

    connection_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);				// Allocate a socket
    
    loc_addr.l2_family = AF_BLUETOOTH;														// Set up source address
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_cid = htobs(ATT_CID);                                   					// ATT_CID = 4, For l2cap to use BLE
    loc_addr.l2_bdaddr_type = 0;
    
    bind(connection_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));				// Bind local bluetooth adapter
    
    rem_addr.l2_family = AF_BLUETOOTH;														// Set up destination address
    rem_addr.l2_cid = htobs(ATT_CID);
    rem_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    str2ba( dest, &rem_addr.l2_bdaddr );
   
    status = connect(connection_socket, (struct sockaddr *)&rem_addr, sizeof(rem_addr));	// Connect to server
    
    if ((childpid = fork()) == 0) {
    
    

    if( status == 0 ) {																		// Send a message
        status = write(connection_socket, "hello!", 6);
    }
    
    while(1){
		memset(buf, 0, sizeof(buf));
		bytes_read = read(connection_socket, buf, sizeof(buf));	
		printf("[%s]\n", buf);
		
		
		if(strcmp("togglePi2\n", buf) == 0){
		toggle_led("toggle\n");
		}
	
	
	}
    
    memset(buf, 0, sizeof(buf));
        
		bytes_read = read(connection_socket, buf, sizeof(buf));								// Read a message
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);
		}

    if( status < 0 ) perror("uh oh");

    close(connection_socket);
} else {
	wiringPiSetup();
    pinMode (2, INPUT);													// Sets button as an input
    int prev_button = LOW;												// Last state of the pull-up circuit
    while (1) {
        if(connection_check == false) exit(0);							// If connection to the client was closed, terminate
        if(prev_button == HIGH && digitalRead(2) == LOW) {				// A falling edge
            prev_button = LOW;
            char reply[] = "button pressed\n";
            write(connection_socket, reply, sizeof(reply));				// [Debugging] Send message to client (Maxsize 32)
            printf("Server message\n");									// [Debugging] Send message to server
        }
        else if(prev_button == LOW && digitalRead(2) == HIGH) {			// a rising edge, do nothing
            prev_button = HIGH;
        }
        delay(100);
    }
}
}

// Code based on 
// beej.us/guide/html/multi/advanced.html#blocking

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc , char * *argv)
{
    struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int listener , client, bytes_read;
    fd_set readfds, master;
    int fdmax, sock_flags;
    
    int i; 
    
    unsigned int opt = sizeof(rem_addr);
    
    // clear fd sets
    FD_ZERO(&readfds);
	FD_ZERO(&master);
	
	// allocate a socket
    listener = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	
	// set the connection parameters (who to connect to)
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    //str2ba(dest, &loc_addr.rc_bdaddr);
	
	// put socket in non-blocking mode
    sock_flags = fcntl (listener , F_GETFL , 0 );
    fcntl (listener , F_SETFL , sock_flags | O_NONBLOCK ) ;
	
	bind(listener, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
	
	if(listen(listener, 1) == -1) {
			perror("listen");
			exit(3);
	}
	
	
	FD_SET(listener, &master);			
	fdmax = listener;
	
	while(1){
		readfds = master;
		if(select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
	
		// go trough all connections
		for(i = 0; i <= fdmax; i++) {
			if(FD_ISSET(i, &readfds)) {
				if(i == listener) {
					client = accept(listener, (struct sockaddr*)&rem_addr, &opt);
					
					if(client == -1) {
						perror("accept");
					} else {
						FD_SET(client, &master);
						if(client < fdmax) {
							fdmax = client;
						}
						printf("New connection");
					}
				} else {
					// read data from the client
					bytes_read = read(i, buf, sizeof(buf));
					if( bytes_read > 0 ) {
						printf("received [%s]\n", buf);
						//write(client, "hi", 2);
					} else {
						close(i);
						FD_CLR(i, &master);
					}
				}
			}
		}
	}
		/*
		// initiate connection attempt
		 client = accept (s , (struct sockaddr *)&rem_addr , &opt);
		
		if( 0 != client && errno != EAGAIN ) {
			perror( "connect" ) ;
			return 1;
		}

		// wait for connection to complete or fail
	
		client = select (maxfd + 1, &readfds , &writefds , NULL , NULL );
		
		ba2str( &rem_addr.rc_bdaddr, buf );
		fprintf(stderr, "accepted connection from %s\n", buf);
		memset(buf, 0, sizeof(buf));

		// read data from the client
		bytes_read = read(client, buf, sizeof(buf));
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);
			write(client, "hi", 2);
		}

		if( client < 0 && FD_ISSET( s , &writefds ) ) {
			client = send(s , "hello!" , 6, 0);
		}
		
		if( client < 0 ) perror( "operation failed" );
		
		close(client);
	}
	* */
    
    return 0;
}

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
    struct sockaddr_rc addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = {0};
    int s , client, bytes_read;
    fd_set readfds, writefds;
    int maxfd, sock_flags;
    socklen_t opt = sizeof(rem_addr);
    
	// allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	
	// set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_bdaddr = *BDADDR_ANY;
    addr.rc_channel = (uint8_t) 1;
    //str2ba(dest, &addr.rc_bdaddr);
	
	// put socket in non-blocking mode
    sock_flags = fcntl ( s , F_GETFL , 0 );
    fcntl ( s , F_SETFL , sock_flags | O_NONBLOCK ) ;
	
	bind(s, (struct sockaddr *)&addr, sizeof(addr));
	
	listen(s, 1);
	
	while(1){
	
		// initiate connection attempt
		client = accept (s , (struct sockaddr *)&rem_addr , &opt);
		
		if( 0 != client && errno != EAGAIN ) {
			perror( "connect" ) ;
			return 1;
		}
		
		// wait for connection to complete or fail
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(s, &writefds);
		maxfd = s;
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

		if( client > 0 && FD_ISSET( s , &writefds ) ) {
			client = send(s , "hello!" , 6, 0);
		}
		
		if( client < 0 ) perror( "operation failed" );
		
		close(client);
	}
    
    close (s);
    return 0;
}

//Code from https://people.csail.mit.edu/albert/bluez-intro/x502.html - RFCOMM sockets

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char **argv)
{
    struct sockaddr_rc loc_addr = { 0 }, cli_addr = { 0 };
    char buf[1024] = { 0 };
    int connection_socket, client, bytes_read;
    socklen_t len = sizeof(cli_addr);

    // allocate socket
    connection_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available 
    // local bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = *BDADDR_ANY;
    loc_addr.rc_channel = (uint8_t) 1;
    bind(connection_socket, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put socket into listening mode
    listen(connection_socket, 1);

	while(1){
					
		// accept one connection
		client = accept(connection_socket, (struct sockaddr *)&cli_addr, &len);

		ba2str( &cli_addr.rc_bdaddr, buf );
		fprintf(stderr, "accepted connection from %connection_socket\n", buf);
		memset(buf, 0, sizeof(buf));

		// read data from the client
		bytes_read = read(client, buf, sizeof(buf));
		if( bytes_read > 0 ) {
			printf("received [%connection_socket]\n", buf);
			//Write data to the client
			write(client, "hi", 2);
		}
		
		close(client);
	}

    // close connection
    
    close(connection_socket);
    return 0;
}

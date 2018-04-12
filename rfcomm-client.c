//Code from https://people.csail.mit.edu/albert/bluez-intro/x502.html - RFCOMM sockets

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <stdlib.h>

int main(int argc, char **argv)
{
    struct sockaddr_rc addr = { 0 };
    char buf[1024] = { 0 };
    int connecting_socket, connection_status, bytes_read;
    char dest[18] = "B8:27:EB:9B:D4:87";                      

    // allocate a socket
    connecting_socket = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    // connect to server
    connection_status = connect(connecting_socket, (struct sockaddr *)&addr, sizeof(addr));

    // send a message
    if( connection_status == 0 ) {
        connection_status = write(connecting_socket, "herro!", 6);
    }
    
    // read a message
    /*bytes_read = read(connecting_socket, buf, sizeof(buf));
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);
		}*/

    if( connection_status < 0 ) perror("uh oh");

    close(connecting_socket);
    return 0;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define ATT_CID 4

int main(int argc, char **argv)
{
    struct sockaddr_l2 loc_addr = { 0 }, rem_addr = { 0 };
    int s, status, hci_socket;
    char buf[1024] = { 0 };
    int hci_device_id = 0;
    char *message = "hello!";
    char dest[18] = "B8:27:EB:9B:D4:87";

    // allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    // Set up source address
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_bdaddr = *BDADDR_ANY;
    loc_addr.l2_cid = htobs(ATT_CID);                                   //ATT_CID = 4, For l2cap to use BLE
    loc_addr.l2_bdaddr_type = 0;
    
    //Bind local bluetooth adapter
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
    
    // Set up destination address
    rem_addr.l2_family = AF_BLUETOOTH;
    rem_addr.l2_cid = htobs(ATT_CID);
    rem_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
    str2ba( dest, &rem_addr.l2_bdaddr );
    
    // connect to server
    status = connect(s, (struct sockaddr *)&rem_addr, sizeof(rem_addr));

    // send a message
    if( status == 0 ) {
        status = write(s, "hello!", 6);
    }
    
    // read a message
    bytes_read = read(status, buf, sizeof(buf));
		if( bytes_read > 0 ) {
			printf("received [%s]\n", buf);
		}

    if( status < 0 ) perror("uh oh");

    close(s);
}

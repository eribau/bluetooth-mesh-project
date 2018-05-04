#ifndef CONNECTION_HANDLER_H_
#define CONNECTION_HANDLER_H_

int socket_creator(char *arr, struct sockaddr_l2 loc_addr, struct sockaddr_l2 rem_addr)
int* connect_to_neighbour(char (*array)[18])
int accept_a_neighbour()
char* print_own_bd_addr()
char* print_dev_hdr(struct hci_dev_info *di)

#endif

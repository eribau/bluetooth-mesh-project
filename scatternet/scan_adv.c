#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "ll.h"
#include "structs.h"
#include "nb_data.h"

#define FLAGS_AD_TYPE 0x01
#define FLAGS_LIMITED_MODE_BIT 0x01
#define FLAGS_GENERAL_MODE_BIT 0x02

#define EIR_FLAGS                   0x01  /* flags */
#define EIR_UUID16_SOME             0x02  /* 16-bit UUID, more available */
#define EIR_UUID16_ALL              0x03  /* 16-bit UUID, all listed */
#define EIR_UUID32_SOME             0x04  /* 32-bit UUID, more available */
#define EIR_UUID32_ALL              0x05  /* 32-bit UUID, all listed */
#define EIR_UUID128_SOME            0x06  /* 128-bit UUID, more available */
#define EIR_UUID128_ALL             0x07  /* 128-bit UUID, all listed */
#define EIR_NAME_SHORT              0x08  /* shortened local name */
#define EIR_NAME_COMPLETE           0x09  /* complete local name */
#define EIR_TX_POWER                0x0A  /* transmit power level */
#define EIR_DEVICE_ID               0x10  /* device ID */



// Functions for advertise

int advertise();

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam)
{
	struct hci_request rq;
	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = ocf;
	rq.cparam = cparam;
	rq.clen = clen;
	rq.rparam = status;
	rq.rlen = 1;
	return rq;
}

le_set_advertising_data_cp ble_hci_params_for_set_adv_data(char * name, char * btaddr)
{
	int name_len = strlen(name);

	le_set_advertising_data_cp adv_data_cp;
	memset(&adv_data_cp, 0, sizeof(adv_data_cp));

	adv_data_cp.data[0] = 0x02; // Length.
	adv_data_cp.data[1] = 0x01; // Flags field.
	adv_data_cp.data[2] = 0x01; // LE Limited Discoverable Flag set

	adv_data_cp.data[3] = name_len + 1; // Length.
	adv_data_cp.data[4] = 0x09; // Name field.
	memcpy(adv_data_cp.data + 5, name, name_len);
	memcpy(adv_data_cp.data + 5 + name_len, btaddr, strlen(btaddr));

	adv_data_cp.length = strlen(adv_data_cp.data);

	return adv_data_cp;
}

// Functions for scan from hcitool.c 

static volatile int signal_received = 0;

static void sigint_handler(int sig)
{
	signal_received = sig;
}

static void eir_parse_name(uint8_t *eir, size_t eir_len,
						char *buf, size_t buf_len)
{
	size_t offset;

	offset = 0;
	while (offset < eir_len) {
		uint8_t field_len = eir[0];
		size_t name_len;

		/* Check for the end of EIR */
		if (field_len == 0)
			break;

		if (offset + field_len > eir_len)
			goto failed;

		switch (eir[1]) {
		case EIR_NAME_SHORT:
		case EIR_NAME_COMPLETE:
			name_len = field_len - 1;
			if (name_len > buf_len)
				goto failed;

			memcpy(buf, &eir[2], name_len);
			return;
		}

		offset += field_len + 1;
		eir += field_len + 1;
	}

failed:
	snprintf(buf, buf_len, "(unknown)");
}

static int read_flags(uint8_t *flags, const uint8_t *data, size_t size)
{
	size_t offset;

	if (!flags || !data)
		return -EINVAL;

	offset = 0;
	while (offset < size) {
		uint8_t len = data[offset];
		uint8_t type;

		/* Check if it is the end of the significant part */
		if (len == 0)
			break;

		if (len + offset > size)
			break;

		type = data[offset + 1];

		if (type == FLAGS_AD_TYPE) {
			*flags = data[offset + 2];
			return 0;
		}

		offset += 1 + len;
	}

	return -ENOENT;
}

static int check_report_filter(uint8_t procedure, le_advertising_info *info)
{
	uint8_t flags;

	/* If no discovery procedure is set, all reports are treat as valid */
	if (procedure == 0)
		return 1;

	/* Read flags AD type value from the advertising report if it exists */
	if (read_flags(&flags, info->data, info->length))
		return 0;

	switch (procedure) {
	case 'l': /* Limited Discovery Procedure */
		if (flags & FLAGS_LIMITED_MODE_BIT)
			return 1;
		break;
	case 'g': /* General Discovery Procedure */
		if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
			return 1;
		break;
	default:
		fprintf(stderr, "Unknown discovery procedure\n");
	}

	return 0;
}


struct nb_object* print_advertising_devices(int dd, uint8_t filter_type, struct nb_object *nb_list) {
	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
	//struct nb_object *nb_object = NULL;
	struct hci_filter nf, of;
	struct sigaction sa;
	socklen_t olen;
	int len;
	time_t start = time(0);
	struct timeval tv;												//Timeout for socket options for read() to be nonblocking
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	char arr [100][18];
	char addr_table[1000][18];
	int addr_counter = 0;
	
	
	
	olen = sizeof(of);
	if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
		printf("Could not get socket options\n");
		exit(-1);
	}

	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);

	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
		printf("Could not set socket options\n");
		//return -1;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_handler = sigint_handler;
	sigaction(SIGINT, &sa,
	 NULL);
	setsockopt(dd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);	
	while (1) {
		evt_le_meta_event *meta;
		le_advertising_info *info;
		char addr[18];
		int addr_exists;
	
		
		while ((len = read(dd, buf, sizeof(buf))) < 0) {
			if (errno == EINTR && signal_received == SIGINT) {
				len = 0;
				printf("first goto\n");
				goto done;
			}

			if (errno == EAGAIN || errno == EINTR)
				continue;
				printf("sec goto\n");
			goto done;
		}

		if (time(0) - start >= 1) {
			printf("third goto\n");

			goto done;
		}

		ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
		len -= (1 + HCI_EVENT_HDR_SIZE);

		meta = (void *) ptr;

		if (meta->subevent != 0x02){
			printf("fourth goto\n");
			goto done;
		}
		/* Ignoring multiple reports */
		info = (le_advertising_info *) (meta->data + 1);
		if (check_report_filter(filter_type, info)) {
			char name[30];

			memset(name, 0, sizeof(name));

			ba2str(&info->bdaddr, addr);
			eir_parse_name(info->data, info->length,
							name, sizeof(name) - 1);
			//uint8_t *dat = info->data;
			char rssi;
			//printf("%d\n", rssi);
			
			if (0 == strcmp("Pi", name)) {
				
				char sec_addr[31];
				printf("Pi read in data ");
				for (int i = 7; i < 24; i++) {
					sec_addr[i-7] = info->data[i];
					//printf("%c", sec_addr[i-7]);
					//printf("%d ", rssi); 
				}
				//printf("\n");
				if(sec_addr[2] == ':' && sec_addr[5] == ':'){
					nb_list = ll_new(nb_list);
					strcpy(nb_list->nb_bdaddr, addr);
					strcpy(nb_list->nb_nb_bdaddr, sec_addr);
				} else {
					nb_list = ll_new(nb_list);
					strcpy(nb_list->nb_bdaddr, addr);
				}
				
				
			} else if(0 == strcmp("	De", name)) {
				char sec_addr[31];
				printf("De read in data");
				for (int i = 7; i < 24; i++) {
					sec_addr[i-7] = info->data[i];
					//printf("%c", sec_addr[i-7]);
					//printf("%d ", rssi); 
				}
				//printf("\n");
				if(sec_addr[2] == ':' && sec_addr[5] == ':'){
					nb_list = ll_new(nb_list);
					strcpy(nb_list->nb_bdaddr, addr);
					strcpy(nb_list->nb_nb_bdaddr, sec_addr);
					nb_list->de = 'T';
				} 
			}
					
			printf("%s %s ", addr, name);
			for (int i = 0; i < info->length; i++) {
				rssi = info->data[i];
				printf("%d ", rssi); 
			}
			printf("\n");
			
			addr_counter++;
			if (addr_counter == (sizeof(addr_table)/sizeof(addr_table[0]) - 1)) addr_counter = 0;
		}
	}

done:
	printf("now in done:");
	setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));
	
	ll_foreach(nb_list, it) {
		printf("My_Neighbour: %s", it->nb_bdaddr);
		//printf("GOOD PRINT 2 %s\n", nan->addr_data);
		printf(" Neighbours_Neighbour: %s\n", it->nb_nb_bdaddr);
		
	}
	
	if (len < 0) exit(-1);

	return nb_list;
}


//should be public
/**
*Advertise(char *array), takes a char array of maximum size 24
*And advertises that, (u8bit) advertisement will look like: 2 bytes for name "pi" followed by
*5 bytes for control and flags followed by the 24 byte message.
**/
int advertise(char *array) {
	//------------------------ADVERTISE------------------------		
	int ret, status;

	const int device = hci_open_dev(hci_get_route(NULL));
	if ( device < 0 ) { 
		perror("Failed to open HC device.");
		return 0; 
	}

	// Set BLE advertisement parameters.
	
	le_set_advertising_parameters_cp adv_params_cp;
	memset(&adv_params_cp, 0, sizeof(adv_params_cp));
	adv_params_cp.min_interval = htobs(0x0800);
	adv_params_cp.max_interval = htobs(0x0800);
	adv_params_cp.chan_map = 7;
	
	struct hci_request adv_params_rq = ble_hci_request(
		OCF_LE_SET_ADVERTISING_PARAMETERS,
		LE_SET_ADVERTISING_PARAMETERS_CP_SIZE, &status, &adv_params_cp);
	
	ret = hci_send_req(device, &adv_params_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to set advertisement parameters data.");
		return 0;
	}
	
	// Set BLE advertisement data.
	//struct neighbour *next = ll_next(neighbours);
	
	le_set_advertising_data_cp adv_data_cp = ble_hci_params_for_set_adv_data("De", array);
	
	struct hci_request adv_data_rq = ble_hci_request(
		OCF_LE_SET_ADVERTISING_DATA,
		LE_SET_ADVERTISING_DATA_CP_SIZE, &status, &adv_data_cp);

	ret = hci_send_req(device, &adv_data_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to set advertising data.");
		return 0;
	}

	// Enable advertising.

	le_set_advertise_enable_cp advertise_cp;
	memset(&advertise_cp, 0, sizeof(advertise_cp));
	advertise_cp.enable = 0x01;

	struct hci_request enable_adv_rq = ble_hci_request(
		OCF_LE_SET_ADVERTISE_ENABLE,
		LE_SET_ADVERTISE_ENABLE_CP_SIZE, &status, &advertise_cp);

	ret = hci_send_req(device, &enable_adv_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror("Failed to enable advertising.");
		return 0;
	}

	hci_close_dev(device);
	
	return 0;
}


//should be public
//*
struct nb_object* scan(struct nb_object *nb_list) {
//-----------------------------------------SCAN---------------------------
	
	int err, opt, dd, dev_id;
	uint8_t own_type = LE_PUBLIC_ADDRESS;
	uint8_t scan_type = 0x01;
	uint8_t filter_type = 0;
	uint8_t filter_policy = 0x00;
	uint16_t interval = htobs(0x0010);
	uint16_t window = htobs(0x0010);
	uint8_t filter_dup = 0x01;
	
	
	// Might break comparisons because of the size/actual size
	int address_dec [] = {66, 56, 58, 50, 55, 58, 69, 66, 58, 52, 70, 58, 68, 54, 58, 53, 54};
	char address_ascii [18] = {0};
	for (int i = 0; i < 17; i++) {
				address_ascii[i] = address_dec[i];
				//~ printf("hardcoded %c\n", address_ascii[i]);
			}
	printf("hardcoded %s\n", address_ascii);
	printf("%c\n", address_ascii[17]);
	//~ printf("xd\n");
	
	dev_id = hci_get_route(NULL);

	dd = hci_open_dev(dev_id);
	
	err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
						own_type, filter_policy, 10000);
	if (err < 0) {
		perror("Set scan parameters failed");
		exit(1);
	}

	err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 10000);
	if (err < 0) {
		perror("Enable scan failed");
		exit(1);
	}

	printf("LE Scan ...\n");

	printf("%d\n", &nb_list);
	nb_list = print_advertising_devices(dd, filter_type, nb_list);
	printf("%d\n", &nb_list);
	
	err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 10000);
	if (err < 0) {
		perror("Disable scan failed");
		exit(1);
	}

	hci_close_dev(dd);
	printf("try to return from scan\n");
	return nb_list;
}

void add_to_array(char (*arr)[18], struct nb_object *nb_list, int *counter){
	for(int i = 0; i < *counter; i++){
		if(0 == strcmp(arr[i], nb_list->nb_bdaddr)){
			return;
		}
	}
	strcpy(arr[*counter], nb_list->nb_bdaddr);
	*counter++;
}


//should be public
/**
* Call to this functions returns a pointer to a array of linked list (*linked_list[]);
* Linked list contains node id name and neighbours to that node.
**/
struct nb_object* scan_adv(){
	//strcpy(neighbours->addr_bt, "0"); 
	time_t start = time(0);
	
	char arr[10][18];
	int counter = 0;
	int current = 0;
	
	struct nb_object *nb_list = NULL;

	while (1) {
		
		char neighbour_1 [] = "";
		
		if ((nb_list != NULL)) {
			printf("this is being advertised %s\n", arr[current]);
			advertise(arr[current]);
			current++;
			if(current > counter){
				current = 0;
			}
		} else {
			advertise(neighbour_1);
		}
	
		nb_list = scan(nb_list);
		
		ll_foreach(nb_list, it){
			add_to_array(arr, nb_list, &counter);
		}
		
		if (time(0) - start >= 20) {
			break;
		}

	}
	/*
	ll_foreach(nb_list, it){
		printf("%s\n", it->nb_bdaddr);
		printf("%c\n", it->de);
	}
	*/
	
	struct nb_object *ptr[16];
	
	*ptr = fill_entries(ptr, nb_list);
	
	struct nb_object *rtn = NULL;
	rtn = rtn_nb_ptr(ptr);
	
	printf("%s\n", rtn->nb_bdaddr);
	
	printf("dab on the haters\n");
	ll_foreach(rtn, it){
		printf("dont dab on the haters\n");
		print_nb_nb(ptr, it->nb_bdaddr);
		print_nb_nb(ptr, it->nb_bdaddr);
	}
	
	ll_foreach(rtn, it){
		printf("%s\n", it->nb_bdaddr);
		printf("%s\n", it->nb_nb_bdaddr);
	}
	
	
	return nb_list;
}


int main(int argc, char *argv[]) {	
	//struct nb_object *new = NULL;
	//new = scan(new);
	scan_adv();
	/*
	ll_foreach(new, it){
		printf("%s\n", it->nb_bdaddr);
		printf("%s\n", it->nb_nb_bdaddr);
		printf("%c\n", it->de);
	}
	*/
	return 0;
}


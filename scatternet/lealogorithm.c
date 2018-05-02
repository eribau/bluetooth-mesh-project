#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define MAX_CONNECTION_LIMIT 2
#define BUFFER_SIZE 1024
#define TIMEOUT_SECONDS 20


typedef enum {false, true} bool;

struct t_neighbours {
	char n[10][18];
};

/**
 * scans neighboars for data, returns true if the data tells us that we
 * have a delegated task.
 * returns false if the data tells us that a node wants to connect to us.
**/

char* scan_neighbours(){												//TODO IMPLEMENT scan_neighbours
	char *data;
	return data;
}


bool wait_until_contacted(){											//TODO GET DATA FOR data
	char* data;
	data = scan_neighbours();
	//parse data and return true if delegated task, else false
	if(true) return true;
	else return false;
}

void accept_connection(){

}

struct t_neighbours get_neighbours(){							//TODO ADD CODE FOR GETTING NEIGHBOURS
	time_t start = time(0);
	struct t_neighbours neighbours;
	while(1){
		//ADD CODE FOR GETTING NEIGHBOURS
		if(TIMEOUT_SECONDS < (time(0) - start)) return neighbours;
	}
}

/**
 * Turns on LeAdv
 * */
void advertise_our_id(){
	int device_id;
    int device_descriptor;
    int advertise;
	device_id = hci_get_route(NULL);
    device_descriptor = hci_open_dev(device_id);
	advertise = hci_le_set_advertise_enable(device_descriptor, 1, 10000); //maybe change from 10000
	printf("leadv on %d\n" , advertise);
}


int main(){
	int connectionlimit = MAX_CONNECTION_LIMIT;
	//TODO GIVE VARIABLES APPROPRIATE VALUES AND SIZES
	char buf[BUFFER_SIZE];
	struct t_neighbours neighbours;
	int prey[0];
	int myid;
	bool delegate = false;

	//add neighbours to list until timeout
	advertise_our_id();													//TODO advertise_our_id
	//get neighbours until timeout
	neighbours = get_neighbours();

	int max_id = 5; //get_max_id_neighbour_array(neighbours);				//TODO

	while(1){
		if(max_id > myid){
			delegate = wait_until_contacted();							//Partially implemented
			if(delegate == false){
				accept_connection();									//TODO IMPLEMENT accept_connection
				break; //goto (done) state
			} else {
				
			}
		}

		//count ammount of neighbours (fin)
		/**
		int ammount_of_neighbours = 0;
		ll_foreach(ll_neighbours, id) {
			ammount_of_neighbours++;
		}

		if(ammount_of_neighbours <= connectionlimit){
			ll_foreach(pray, bladrr){
				connect(id or bdaddr);									//TODO IMPLEMENT CONNECT WITH IN
				//connectionlimit--; needed? no
																			
			}
		} else {
			delegate_work(neighbour_max);								//TODO IMPLEMENT delegate_work,
																		//& neighbour_max
			cn = wait_for_cn();											//TODO IMPLEMENT wait_for_cn
			remove_cn(prey);											//TODO IMPLEMENT remove_cn;
		}
		* */
	}
}

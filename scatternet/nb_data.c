/*
This code will take information about neighbours and store it in an array of linked lists.
This code is based on Charles Lehner code ll.c which is free software.
*/
#include <stdio.h>
#include <string.h>
#include "ll.h"

#define MAX_ARR_LENGTH 16

// Global variables
int nmb_arr_entries = 0;
struct nb_object *ptr[16] = { 0 };


/** nb_bdaddr is neighbour bluetooth address, nb_nb is nb's neighbour address **/
struct nb_object {
 	char nb_bdaddr[18];
	char nb_nb_bdaddr[18];
};

/** Adds a LL to an entry in the array **/
void add_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr){
  struct nb_object *nb_object = NULL;
  nb_object = ll_new(nb_object);
  strcpy(nb_object->nb_bdaddr, nb_bdaddr);
  strcpy(nb_object->nb_nb_bdaddr, nb_nb_bdaddr);
  ptr[nmb_arr_entries] = nb_object;
  printf("Neighbour %s is now neigbour with %s\n", nb_bdaddr, nb_nb_bdaddr);
  nmb_arr_entries++;
}

/** Adds new nb_nb. If nb not found it runs add_nb and then adds nb_nb. **/
void add_nb_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr){
	for(int j = 0; j < nmb_arr_entries; j++){
		if(0 == strcmp(ptr[j]->nb_bdaddr, nb_bdaddr)){	
			ll_foreach(ptr[j], it){
				if(0 == strcmp(it->nb_nb_bdaddr, nb_nb_bdaddr)){
					printf("Duplicate\n");
					return;
				}
			}
			ptr[j] = ll_new(ptr[j]);
			strcpy(ptr[j]->nb_bdaddr, nb_bdaddr);
			strcpy(ptr[j]->nb_nb_bdaddr, nb_nb_bdaddr);
			printf("Neighbour %s is now neigbour with %s\n", ptr[j]->nb_bdaddr, nb_nb_bdaddr);
			return;
		} else {
			continue;
		}
	}
	printf("Added %s to the array\n", nb_bdaddr);
	add_nb(ptr, nb_bdaddr, nb_nb_bdaddr);
}

/** Prints all neighbours **/
void print_nb(struct nb_object **ptr){
	for(int j; j < nmb_arr_entries; j++){
		printf("%s\n", ptr[j]->nb_bdaddr);
	}
}

/** Prints all the neigbours of nb_bdaddr **/
void print_nb_nb(struct nb_object **ptr, char *nb_bdaddr){
	for(int j = 0; j < nmb_arr_entries; j++){
		if(0 == strcmp(ptr[j]->nb_bdaddr, nb_bdaddr)){	
			ll_foreach(ptr[j], it){
				printf("%s: %s\n", nb_bdaddr, it->nb_nb_bdaddr);
			}
		}
	}
}


// Remove this later
/** Adds all entries from list_ptr and stores them without duplicates in ptr **/
/*void fill_entries(struct nb_object **ptr, struct nb_object *list_ptr){
  while(ll_pop(list_ptr) != NULL){
      add_nb_nb(ptr, list_ptr->nb_bdaddr, list_ptr->nb_nb_bdaddr);
      ll_pop(list_ptr);
  }
}*/

/** Adds all entries from list_ptr and stores them without duplicates in ptr **/
void fill_entries(struct nb_object **ptr, struct nb_object *list_ptr){
  ll_foreach(list_ptr, it){
    add_nb_nb(ptr, it->nb_bdaddr, it->nb_nb_bdaddr);
  }
}

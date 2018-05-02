#include <stdio.h>
#include <string.h>
#include "ll.h"

#define MAX_ARR_LENGTH 16

int i = 0;

// Nb is neighbour, nb_nb is nb's neighbour
struct nb_object {
 	char nb_bdaddr[18];
	char nb_nb_bdaddr[18];
};

// Adds LL to array element
void add_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr){
  struct nb_object *nb_object = NULL;
  nb_object = ll_new(nb_object);
  strcpy(nb_object->nb_bdaddr, nb_bdaddr);
  strcpy(nb_object->nb_nb_bdaddr, nb_nb_bdaddr);
  ptr[i] = nb_object;
  i++;
}

void add_nb_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr){
	for(int j = 0; j < i; j++){
		if(0 == strcmp(ptr[j]->nb_bdaddr, nb_bdaddr)){	
			ll_foreach(ptr[j], it){
				printf("this will print all nb_nb: %s\n",it->nb_nb_bdaddr);
				if(0 == strcmp(it->nb_nb_bdaddr, nb_nb_bdaddr)){
					printf("duplicate\n");
					return;
				}
			}
			ptr[j] = ll_new(ptr[j]);
			strcpy(ptr[j]->nb_bdaddr, nb_bdaddr);
			strcpy(ptr[j]->nb_nb_bdaddr, nb_nb_bdaddr);
			printf("%s == %s\n", ptr[j]->nb_bdaddr, nb_bdaddr);
			return;
		} else {
			continue;
		}
	}
	printf("adding new nb_bdaddr\n");
	add_nb(ptr, nb_bdaddr, nb_nb_bdaddr);

}

void print_nb(struct nb_object **ptr){
	for(int j; j < i; j++){
		printf("%s\n", ptr[j]->nb_bdaddr);
	}
}

void print_nb_nb(struct nb_object **ptr, char *nb_bdaddr){
	for(int j = 0; j < i; j++){
		if(0 == strcmp(ptr[j]->nb_bdaddr, nb_bdaddr)){	
			ll_foreach(ptr[j], it){
				printf("%s: %s\n", nb_bdaddr, it->nb_nb_bdaddr);
			}
		}
	}
}

int main()
{
	printf("a\n");
	struct nb_object *ptr[16] = { 0 };
	add_nb_nb(ptr, "pi3", "pi1");
	add_nb_nb(ptr, "pi3", "pi3");
	add_nb_nb(ptr, "pi3", "pi4");
	add_nb_nb(ptr, "pi3", "pi4");
	add_nb_nb(ptr, "pi3", "pi5");
	add_nb_nb(ptr, "pi4", "pi5");
	
	//print_nb(ptr);
	print_nb_nb(ptr, "pi3");
   return 0;
}

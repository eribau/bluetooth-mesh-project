#include <stdio.h>
#include <string.h>
#include "ll.h"

int i = 0;

struct thing {
  char bdaddr[18];
};

// Adds LL to array element
void add_nb(struct thing **ptr){

  struct thing *thing = NULL;
  thing = ll_new(thing);
  strcpy(thing->bdaddr, "downiepie");
  ptr[i] = thing;
  i++;
}

int main()
{

  struct thing *ptr[16] = { 0 };

  /* Related to LL */
    struct thing *thing = NULL; /* list */

    thing = ll_new(thing);
    strcpy(thing->bdaddr, "world");
    
    thing = ll_new(thing);
    strcpy(thing->bdaddr, "hejsanxdface");  
    struct thing *thing3= NULL;
    thing3 = ll_new(thing3);
    strcpy(thing3->bdaddr, "downs");
    add_nb(ptr);
    add_nb(ptr);
    
    struct thing *thing2 = NULL; /* list */

    ptr[0] = thing;
    
    ll_foreach(ptr[0], it){
     printf("%s\n",it->bdaddr);
    }
    ll_foreach(ptr[1], it){
     printf("%s\n",it->bdaddr);
    }
    ll_foreach(ptr[2], it){
     printf("%s\n",it->bdaddr);
    }
    
   return 0;
}

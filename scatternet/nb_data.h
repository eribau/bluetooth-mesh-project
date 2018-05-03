#ifndef NB_DATA_H_
#define NB_DATA_H_

void add_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr);
void add_nb_nb(struct nb_object **ptr, char *nb_bdaddr, char *nb_nb_bdaddr);
void print_nb(struct nb_object **ptr);
void print_nb_nb(struct nb_object **ptr, char *nb_bdaddr);
struct nb_object* fill_entries(struct nb_object **ptr, struct nb_object *list_ptr);

#endif

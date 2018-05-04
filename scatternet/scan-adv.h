#ifndef SCAN-ADV_H_  
#define SCAN-ADV_H_

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "ll.h"
#include "structs.h"
#include "nb_data.h"


le_set_advertising_data_cp ble_hci_params_for_set_adv_data(char * name, char * btaddr);

static void sigint_handler(int sig);

static void eir_parse_name(uint8_t *eir, size_t eir_len, char *buf, size_t buf_len);

static int read_flags(uint8_t *flags, const uint8_t *data, size_t size);

static int check_report_filter(uint8_t procedure, le_advertising_info *info);

struct nb_object* print_advertising_devices(int dd, uint8_t filter_type, struct nb_object *nb_object);

int advertise(char *array);

struct nb_object* scan(struct nb_object *nb_object);

void add_to_array(char (*arr)[18], struct nb_object *nb_object, int *counter);

#endif

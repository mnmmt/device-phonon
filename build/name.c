#include <usb_names.h>

#define MANUFACTURER_NAME    {'b','z','z','t','.','s','t','u','d','i','o'}
#define MANUFACTURER_NAME_LEN 11
#define PRODUCT_NAME    {'t','h','u', 'm', 'b', 'e', 'l', 'i', 'n', 'a'}
#define PRODUCT_NAME_LEN 10

struct usb_string_descriptor_struct usb_string_manufacturer_name = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME
};

struct usb_string_descriptor_struct usb_string_product_name = {
        2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME
};

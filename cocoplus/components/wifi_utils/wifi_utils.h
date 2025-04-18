#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

typedef enum {
    WIFI_STATUS_SUCCESS = 0,
    WIFI_STATUS_FAIL = 1
} wifi_status_t;

wifi_status_t wifi_init_sta();

#endif // WIFI_UTILS_H

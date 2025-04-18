#ifndef COCOPLUS_MAIN_H
#define COCOPLUS_MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

extern TaskHandle_t drive_task_handle;
extern TaskHandle_t publish_task_handle;

extern SemaphoreHandle_t distance_mutex;
extern SemaphoreHandle_t controller_mutex;

extern bool should_stop;

#endif // COCOPLUS_MAIN_H

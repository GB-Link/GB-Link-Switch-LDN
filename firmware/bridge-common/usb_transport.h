#pragma once
#include <stddef.h>
#include <stdint.h>
void usb_transport_init(void);
int usb_transport_read(void *buffer, size_t length);
void usb_transport_write(const void *buffer, size_t length);
uint32_t usb_transport_dropped(void);

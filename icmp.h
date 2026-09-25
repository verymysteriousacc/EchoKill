#ifndef ICMP_H
#define ICMP_H

#include <stddef.h>
#include <stdint.h>

int icmp_checksum(const void *data, size_t length);
int icmp_create_socket(void);
int icmp_send_echo(int socket_fd, const char *target, uint16_t id, uint16_t sequence);
int icmp_receive_echo(int socket_fd, uint16_t id, uint16_t sequence);

#endif
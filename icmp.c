#include "icmp.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int icmp_checksum(const void *data, size_t length) {
    const uint16_t *buffer = data;
    uint32_t sum = 0;

    while (length > 1) {
        sum += *buffer++;
        length -= 2;
    }

    if (length == 1)
        sum += *(const uint8_t *)buffer;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;

    return (uint16_t)~sum;
}

int icmp_create_socket(void) {
    return socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
}

int icmp_send_echo(int socket_fd, const char *target, uint16_t id, uint16_t sequence) {
    struct sockaddr_in address;
    struct icmphdr packet;

    memset(&address, 0, sizeof(address));
    memset(&packet, 0, sizeof(packet));

    address.sin_family = AF_INET;

    if (inet_pton(AF_INET, target, &address.sin_addr) != 1)
        return -1;

    packet.type = ICMP_ECHO;
    packet.code = 0;
    packet.un.echo.id = htons(id);
    packet.un.echo.sequence = htons(sequence);
    packet.checksum = icmp_checksum(&packet, sizeof(packet));

    return sendto(
        socket_fd,
        &packet,
        sizeof(packet),
        0,
        (struct sockaddr *)&address,
        sizeof(address)
    );
}

int icmp_receive_echo(int socket_fd, uint16_t id, uint16_t sequence) {
    unsigned char buffer[4096];
    struct sockaddr_in address;
    socklen_t address_length = sizeof(address);

    ssize_t received = recvfrom(
        socket_fd,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&address,
        &address_length
    );

    if (received < 0)
        return -1;

    if ((size_t)received < sizeof(struct ip))
        return -1;

    struct ip *ip_header = (struct ip *)buffer;
    size_t header_length = ip_header->ip_hl * 4;

    if ((size_t)received < header_length + sizeof(struct icmphdr))
        return -1;

    struct icmphdr *icmp_header =
        (struct icmphdr *)(buffer + header_length);

    if (icmp_header->type != ICMP_ECHOREPLY)
        return 0;

    if (ntohs(icmp_header->un.echo.id) != id)
        return 0;

    if (ntohs(icmp_header->un.echo.sequence) != sequence)
        return 0;

    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>

unsigned short checksum(void *data, int len) {
    unsigned short *buf = data;
    unsigned long sum = 0;

    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }

    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += sum >> 16;

    return ~sum;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <ip>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in target;
    memset(&target, 0, sizeof(target));
    target.sin_family = AF_INET;

    if (inet_pton(AF_INET, argv[1], &target.sin_addr) != 1) {
        printf("Invalid IP address\n");
        close(sock);
        return 1;
    }

    struct icmphdr packet;
    memset(&packet, 0, sizeof(packet));

    packet.type = ICMP_ECHO;
    packet.code = 0;
    packet.un.echo.id = getpid() & 0xffff;
    packet.un.echo.sequence = 1;
    packet.checksum = checksum(&packet, sizeof(packet));

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (sendto(sock, &packet, sizeof(packet), 0,
        (struct sockaddr *)&target, sizeof(target)) < 0) {
        perror("sendto");
        close(sock);
        return 1;
    }

    unsigned char buffer[4096];
    struct sockaddr_in reply;
    socklen_t reply_len = sizeof(reply);

    ssize_t received = recvfrom(
        sock,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&reply,
        &reply_len
    );

    if (received < 0) {
        perror("recvfrom");
        close(sock);
        return 1;
    }

    gettimeofday(&end, NULL);

    double rtt =
        (end.tv_sec - start.tv_sec) * 1000.0 +
        (end.tv_usec - start.tv_usec) / 1000.0;

    struct iphdr *ip = (struct iphdr *)buffer;
    struct icmphdr *icmp =
        (struct icmphdr *)(buffer + ip->ihl * 4);

    if (icmp->type == ICMP_ECHOREPLY) {
        printf("Reply from %s: time=%.2f ms ttl=%d\n",
            inet_ntoa(reply.sin_addr),
            rtt,
            ip->ttl
        );
    } else {
        printf("Received ICMP type %d\n", icmp->type);
    }

    close(sock);
    return 0;
}
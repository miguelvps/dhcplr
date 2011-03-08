#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "dhcp.h"


void release_packet(struct dhcp_packet *packet, struct in_addr siaddr,
                    struct in_addr ciaddr, struct ether_addr chaddr) {

    memset(packet, 0, sizeof(*packet));

    packet->op = BOOTREQUEST;
    packet->htype = HTYPE_ETHER;
    packet->hlen = 6;
    packet->xid = rand();
    packet->ciaddr = ciaddr;
    memcpy(packet->chaddr, &chaddr, 6);

    // start options
    memcpy(packet->options, DHCP_OPTIONS_COOKIE, 4);

    packet->options[4] = DHO_DHCP_MESSAGE_TYPE;
    packet->options[5] = 1;
    packet->options[6] = DHCPRELEASE;

    packet->options[7] = DHO_DHCP_SERVER_IDENTIFIER;
    packet->options[8] = 4;
    memcpy(packet->options+9, &siaddr, 4);

    packet->options[13] = DHO_END;
    // end options
}


int main(int argc, const char *argv[]) {

    int s, r;
    struct sockaddr_in sa;
    struct dhcp_packet release;
    struct in_addr siaddr, ciaddr;
    struct ether_addr *chaddr;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s SIADDR CIADDR CHADDR\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    r = inet_pton(AF_INET, argv[1], &siaddr);
    if (!r) {
        fprintf(stderr, "%s: '%s' is not a valid SIADDR\n", argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }

    r = inet_pton(AF_INET, argv[2], &ciaddr);
    if (!r) {
        fprintf(stderr, "%s: '%s' is not a valid CIADDR\n", argv[0], argv[2]);
        exit(EXIT_FAILURE);
    }

    chaddr = ether_aton(argv[3]);
    if (chaddr == NULL) {
        fprintf(stderr, "%s: '%s' is not a valid CHADDR\n", argv[0], argv[3]);
        exit(EXIT_FAILURE);
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (!s) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(DHCP_SERVER_PORT);
    sa.sin_addr = siaddr;

    release_packet(&release, siaddr, ciaddr, *chaddr);

    r = sendto(s, &release, sizeof(release), 0, (const struct sockaddr *)&sa,
               sizeof(sa));
    if (!r) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return 0;
}

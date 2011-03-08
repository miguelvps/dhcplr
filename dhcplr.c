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

    memset(&release, 0, sizeof(release));

    release.op = BOOTREQUEST;
    release.htype = HTYPE_ETHER;
    release.hlen = 6;
    release.xid = rand();
    release.ciaddr = ciaddr;
    memcpy(release.chaddr, (void *)chaddr, 6);

    // start options
    memcpy(release.options, (void *)DHCP_OPTIONS_COOKIE, 4);

    release.options[4] = DHO_DHCP_MESSAGE_TYPE;
    release.options[5] = 1;
    release.options[6] = DHCPRELEASE;

    release.options[7] = DHO_DHCP_SERVER_IDENTIFIER;
    release.options[8] = 4;
    memcpy(release.options+9, (void *)&siaddr.s_addr, 4);

    release.options[13] = DHO_END;
    // end options

    r = sendto(s, &release, sizeof(release), 0, (const struct sockaddr *)&sa,
               sizeof(sa));

    if (!r) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return 0;
}

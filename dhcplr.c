#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "dhcp.h"


int main(int argc, const char *argv[]) {

    int s;
    struct sockaddr_in sa;
    struct dhcp_packet release;
    const char *siaddr, *ciaddr, *chaddr;

    if (argc != 4) {
        printf("Usage: %s SIADDR CIADDR CHADDR\n", argv[0]);
        return 1;
    }

    siaddr = argv[1];
    ciaddr = argv[2];
    chaddr = argv[3];

    srand(time(NULL));

    memset(&release, 0, sizeof(release));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(DHCP_SERVER_PORT);
    sa.sin_addr.s_addr = inet_addr(siaddr);

    s = socket(AF_INET, SOCK_DGRAM, 0);

    release.op = BOOTREQUEST;
    release.htype = HTYPE_ETHER;
    release.hlen = 6;
    release.xid = rand();
    release.ciaddr.s_addr = inet_addr(ciaddr);
    memcpy(release.chaddr, (void *)ether_aton(chaddr), 6);

    // start options
    memcpy(release.options, (void *)DHCP_OPTIONS_COOKIE, 4);

    release.options[4] = DHO_DHCP_MESSAGE_TYPE;
    release.options[5] = 1;
    release.options[6] = DHCPRELEASE;

    release.options[7] = DHO_DHCP_SERVER_IDENTIFIER;
    release.options[8] = 4;
    memcpy(release.options+9, (void *)&sa.sin_addr.s_addr, 4);

    release.options[13] = DHO_END;
    // end options

    sendto(s, &release, sizeof(release), 0, (const struct sockaddr *)&sa,
            sizeof(sa));

    printf("%s\n", strerror(errno));

    return 0;
}

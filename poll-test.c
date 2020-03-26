#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>

#define ATTR_SIZE 100
#define DEFAULT_TIMEOUT 1000000
#define DEFAULT_EVENTS_MASK POLLPRI|POLLERR

int main(int argc, char **argv)
{
    int cnt, notify_fd, rv, opt, timeout, mask, loop;
    char attrData[ATTR_SIZE];
    struct pollfd ufds[1];
    char *sysfs_notify;
    loop = 0;
    timeout = -1;
    mask = -1;

    while ((opt = getopt(argc, argv, "f:t:m:l")) != -1)
    {
        switch (opt)
        {
        case 'f':
            printf("ARG filename: %s\n", optarg);
            sysfs_notify = strdup(optarg);
            break;
        case 't':
            printf("ARG timeout: %s\n", optarg);
            timeout = atoi(optarg);
            break;
        case 'm':
            printf("ARG mask: %s\n", optarg);
            sscanf(optarg, "%x", &mask);
            break;
        case 'l':
            printf("ARG loop mode ON\n");
            loop = 1;
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    if (*sysfs_notify == 0)
    {
        perror("You must give filename in order to poll! Exiting...");
        exit(1);
    }

    if (timeout == -1)
    {
        printf("You did not set timeout... Setting it to %d\n", DEFAULT_TIMEOUT);
        timeout = DEFAULT_TIMEOUT;
    }

    if (mask == -1)
    {
        printf("You did not set mask... Setting it to 0x%x\n", DEFAULT_EVENTS_MASK);
        mask = DEFAULT_EVENTS_MASK;
    }

    printf("Filename: %s\nTimeout: %d\nMask: %x\nLoop: %d\n", sysfs_notify, timeout, mask, loop);

    do
    {
        if ((notify_fd = open(sysfs_notify, O_RDWR)) < 0)
        {
            perror("Unable to open notify");
            exit(1);
        }

        ufds[0].fd = notify_fd;
        ufds[0].events = mask;

        cnt = read(notify_fd, attrData, ATTR_SIZE);
        ufds[0].revents = 0;

        if ((rv = poll(ufds, 1, timeout)) < 0)
            perror("Poll error");
        else if (rv == 0)
            printf("Timeout occurred!\n");
        else
            printf("Triggered\n");

        printf("revents[0]: %08X\n", ufds[0].revents);
        close(notify_fd);
    } while (loop);
}
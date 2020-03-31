#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>

#define DEFAULT_READ_SIZE   (128)
#define DEFAULT_TIMEOUT     (-1)
#define DEFAULT_EVENTS_MASK (POLLPRI|POLLIN|POLLOUT|POLLERR)

int main(int argc, char **argv)
{
	int cnt, notify_fd, rv, opt, timeout, mask, loop;
	char data[DEFAULT_READ_SIZE];
	struct pollfd ufds[1];
	char *filename;
	loop = 0;
	timeout = -1;
	mask = -1;

	while ((opt = getopt(argc, argv, "f:t:m:l")) != -1)
	{
		switch (opt)
		{
		case 'f':
			printf("ARG filename: %s\n", optarg);
			filename = strdup(optarg);
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

	if (filename == NULL)
	{
		perror("You must give filename in order to poll! Exiting...");
		exit(EXIT_FAILURE);
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

	printf("Filename: %s\nTimeout: %d\nMask: %x\nLoop: %d\n", filename, timeout, mask, loop);

	do
	{
		if ((notify_fd = open(filename, O_RDWR)) < 0)
		{
			perror("Unable to open notify");
			exit(EXIT_FAILURE);
		}

		ufds[0].fd = notify_fd;
		ufds[0].events = mask;

		cnt = read(notify_fd, data, DEFAULT_READ_SIZE);
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

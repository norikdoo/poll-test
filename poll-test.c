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

#ifdef DEBUG
#define DBG(...) do { fprintf(stderr, "[DEBUG]: "__VA_ARGS__); } while(0)
#else
#define DBG(...) do { } while (0)
#endif
#define WARN(...) do { fprintf(stdout, " [WARN]: "__VA_ARGS__); } while(0)
#define INFO(...) do { fprintf(stdout, " [INFO]: "__VA_ARGS__); } while(0)
#define ERROR(...) do { fprintf(stderr, "[ERROR]: "__VA_ARGS__); } while(0)

#define DEFAULT_READ_SIZE   (4096)
#define DEFAULT_TIMEOUT     (-1)
#define DEFAULT_EVENTS_MASK (POLLIN|POLLOUT)
#define DEFAULT_FILE_FLAGS  (O_RDWR)

struct poll_event {
	int number;
	char name[11];
};

struct poll_event poll_events[] =
{
	{ POLLIN,     "POLLIN" },
	{ POLLPRI,    "POLLPRI" },
	{ POLLOUT,    "POLLOUT" },
	{ POLLERR,    "POLLERR" },
	{ POLLHUP,    "POLLHUP" },
	{ POLLNVAL,   "POLLNVAL" },
	{ POLLRDNORM, "POLLRDNORM" },
	{ POLLRDBAND, "POLLRDBAND" },
	{ POLLWRNORM, "POLLWRNORM" },
	{ POLLWRBAND, "POLLWRBAND" },
};

struct fopen_flag {
	int number;
	char name[11];
};

struct fopen_flag fopen_flags[] =
{
	{ O_RDONLY,   "O_RDONLY" },
	{ O_WRONLY,   "O_WRONLY" },
	{ O_RDWR,     "O_RDWR" },
	{ O_NONBLOCK, "O_NONBLOCK" },
	{ O_RDONLY,   "ro" },
	{ O_WRONLY,   "wo" },
	{ O_RDWR,     "rw" },
	{ O_NONBLOCK, "nonblock" },
};

int parse_optarg_file_flags(char *str)
{
	char sep[] = " ,;:|";
	char *token;
	int flag = 0;
	int i;

	DBG("%s(): input string = %s\n", __func__, str);

	/* Allow to specify fopen_flag as hex value */
	sscanf(str, "%x", &flag);
	if (flag != 0)
		goto out;

	token = strtok(str, sep);
	while (token != NULL)
	{
		for (i = 0; i < sizeof(fopen_flags) / sizeof(struct fopen_flag); ++i)
		{
			if (!strcmp(token, fopen_flags[i].name)) {
				flag |= fopen_flags[i].number;
				DBG("%s(): 0x%08X -> %s == %s\n", __func__,
					fopen_flags[i].number, fopen_flags[i].name, token);
			}
		}
#if 0
		if (!flag) {
			WARN("Unknown file flag: %s\n", token);
			return -1;
		}
#endif
		token = strtok(NULL, sep);
	}
out:
	DBG("%s(): output file_flag = 0x%08X\n", __func__, flag);
	return flag;
}

int parse_optarg_poll_mask(char *str)
{
	char sep[] = " ,;:|";
	char *token;
	int mask = 0;
	int i;

	DBG("%s(): input string = %s\n", __func__, str);

	/* Allow to specify poll_mask as hex value */
	sscanf(str, "%x", &mask);
	if (mask != 0)
		goto out;

	token = strtok(str, sep);
	while (token != NULL)
	{
		for (i = 0; i < sizeof(poll_events) / sizeof(struct poll_event); ++i)
		{
			if (!strcmp(token, poll_events[i].name)) {
				mask |= poll_events[i].number;
				DBG("%s(): 0x%04X -> %s == %s\n", __func__,
					poll_events[i].number, poll_events[i].name, token);
			}
		}
#if 0
		if (!mask) {
			WARN("Unknown poll mask: %s\n", token);
			return -1;
		}
#endif
		token = strtok(NULL, sep);
	}
out:
	DBG("%s(): output poll_mask = 0x%04X\n", __func__, mask);
	return mask;
}

int parse_poll_revents_to_string(int revents, char *output)
{
	char str[256] = "";
	int i;

	DBG("%s(): input revents = 0x%04X\n", __func__, revents);

	for (i = 0; i < sizeof(poll_events) / sizeof(struct poll_event); ++i)
	{
		if (revents & (poll_events[i].number)) {
			strcat(str, poll_events[i].name);
			DBG("%s(): found 0x%04X -> %s in 0x%04X\n", __func__,
					poll_events[i].number, poll_events[i].name, revents);
			strcat(str, "|");
		}
	}

	/* Remove last '|' delimiter */
	if(str[strlen(str)-1] == '|')
		str[strlen(str)-1] = '\0';

	strcpy(output, str);

	DBG("%s(): output string = %s\n", __func__, str);
	return 0;
}

int main(int argc, char **argv)
{
	char data[DEFAULT_READ_SIZE];
	struct pollfd poll_fds[1];
	char *filename = NULL;
	int fd, opt, i, bytes, ret = -1, cnt = 0;
	int init_read = -1, read_after = -1, write_after = -1;
	int poll_timeout = -1, poll_mask = -1;
	int file_flags = -1;
	int loop_mode = 0;

	while ((opt = getopt(argc, argv, "f:m:o:t:liew")) != -1)
	{
		switch (opt)
		{
		case 'f':
			DBG("ARG: filename: %s\n", optarg);
			filename = strdup(optarg);
			break;
		case 'm':
			DBG("ARG: poll_mask: %s\n", optarg);
			poll_mask = parse_optarg_poll_mask(optarg);
			break;
		case 'o':
			DBG("ARG: file_flags: %s\n", optarg);
			file_flags = parse_optarg_file_flags(optarg);
			break;
		case 't':
			DBG("ARG: poll_timeout: %s\n", optarg);
			poll_timeout = atoi(optarg);
			break;
		case 'l':
			DBG("ARG: loop mode ON\n");
			loop_mode = 1;
			break;
		case 'i':
			DBG("ARG: initial dummy read() ON\n");
			init_read = 1;
			break;
		case 'e':
			DBG("ARG: after poll() dummy read() ON\n");
			read_after = 1;
			break;
		case 'w':
			DBG("ARG: after poll() dummy write() ON\n");
			write_after = 1;
			break;
		case '?':
			ERROR("Unknown option switch -%c\n", optopt);
			errno = EINVAL;
			goto out;
		}
	}

	if (filename == NULL) {
		ERROR("No input file specified, use -f switch\n");
		errno = EINVAL;
		goto out;
	}

	if (file_flags == -1) {
		WARN("Using default open() file flags: 0x%08X\n", DEFAULT_FILE_FLAGS);
		file_flags = DEFAULT_FILE_FLAGS;
	}

	if (poll_mask == -1) {
		parse_poll_revents_to_string(DEFAULT_EVENTS_MASK, data);
		WARN("Using default poll() mask value: 0x%04X -> %s\n",
			DEFAULT_EVENTS_MASK, data);
		poll_mask = DEFAULT_EVENTS_MASK;
	}

	if (poll_timeout < 0) {
		WARN("Using default poll() timeout value: %d %s\n",
			DEFAULT_TIMEOUT, DEFAULT_TIMEOUT < 0 ? "(no timeout)" : "ms");
		poll_timeout = DEFAULT_TIMEOUT;
	}

#ifdef DEBUG
	DBG("Listing all supported poll events:\n");
	for (i = 0; i < sizeof(poll_events) / sizeof(struct poll_event); ++i)
	{
		DBG("0x%04X -> %s\n", poll_events[i].number, poll_events[i].name);
	}
	DBG("Listing all supported fopen flags:\n");
	for (i = 0; i < sizeof(fopen_flags) / sizeof(struct fopen_flag); ++i)
	{
		DBG("0x%08X -> %s\n", fopen_flags[i].number, fopen_flags[i].name);
	}
#endif
	parse_poll_revents_to_string(poll_mask, data);

	INFO("Do %s poll() on %s (mask: 0x%04X -> %s, poll_timeout: %d %s)\n",
		loop_mode ? "loop" : "single", filename, poll_mask, data,
		poll_timeout, poll_timeout < 0 ? "(no timeout)" : "ms");

	/* Open file */
	if ((fd = open(filename, file_flags)) < 0)
	{
		ERROR("Unable to open %s: %s\n", filename, strerror(errno));
		goto out;
	}

	/* Initial dummy read() */
	if (init_read == 1) {
		bytes = read(fd, data, DEFAULT_READ_SIZE);
		if (bytes == -1) {
			ERROR("Inital dummy read() failed: %s\n", strerror(errno));
			goto out;
		}
		INFO("Initial dummy read() returns %d bytes\n", bytes);
	}

	/* Setup & do poll() */
	poll_fds[0].fd = fd;
	poll_fds[0].revents = 0;
	poll_fds[0].events = poll_mask;

	do {
		ret = poll(poll_fds, 1, poll_timeout);
		if (ret == -1) {
			ERROR("(%d) poll() error: %s\n", cnt, strerror(errno));
			goto out;
		} else if (ret == 0) {
			INFO("(%d) poll() timeout of %d msec reached...\n", cnt,
				poll_timeout);
		} else {
			parse_poll_revents_to_string(poll_fds[0].revents, data);
			INFO("(%d) poll() returned revents: 0x%04X -> %s\n", cnt,
				poll_fds[0].revents, data);

			/* We can do dummy read() if enabled */
			if ((read_after != -1) && (poll_fds[0].revents &
				(POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND))) {

				DBG("(%d) Executing dummy read() after poll()\n", cnt);

				/* lseek() is needed before read() for sysfs attributes */
				lseek(fd, 0, SEEK_SET);
				bytes = read(fd, data, DEFAULT_READ_SIZE);
				if (bytes == -1) {
					ERROR("Dummy read() after poll() failed: %s\n",
						strerror(errno));
					goto out;
				}

				INFO("(%d) Dummy read() after poll() returns %d bytes\n",
					cnt, bytes);

				/* Flush revents */
				poll_fds[0].revents = 0;
			}

			/* We can do dummy write() if enabled */
			if ((write_after != -1) && (poll_fds[0].revents &
				(POLLOUT|POLLWRNORM|POLLWRBAND))) {

				DBG("(%d) Executing dummy write() after poll()\n", cnt);

				/* lseek() is needed before write() for sysfs attributes */
				lseek(fd, 0, SEEK_SET);
				bytes = write(fd, data, DEFAULT_READ_SIZE);
				if (bytes == -1) {
					ERROR("Dummy write() after poll() failed: %s\n",
						strerror(errno));
					goto out;
				}

				INFO("(%d) Dummy write() after poll() returns %d bytes\n",
					cnt, bytes);

				/* Flush revents */
				poll_fds[0].revents = 0;
			}

			/* File errors & status */
			if (poll_fds[0].revents & (POLLERR|POLLHUP|POLLNVAL)) {
				WARN("(%d) Filedescriptor reports errors after poll()...\n",
					cnt);
			}
		}
		cnt++;
	} while (loop_mode);

out:
	close(fd);
	DBG("Return value: %d (errno: %s)\n", errno, strerror(errno));
	errno == 0 ? exit(EXIT_SUCCESS) : exit(EXIT_FAILURE);
}

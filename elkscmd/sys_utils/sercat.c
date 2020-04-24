/*
 * sercat.c - serial cat (for testing)
 *
 *	sercat [-v] [serial device] [> file]
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#define DEFAULT_PORT "/dev/ttyS0"
#define BUF_SIZE 4096
#define CTRL_D	04

int verbose = 0;
int fd;
char readbuf[BUF_SIZE];
struct termios org, new;

void sig_handler(int sig)
{
	fprintf(stderr, "Interrupt\n");
	tcsetattr(fd, TCSAFLUSH, &org);
	close(fd);
	exit(1);
}

void copyfile(int ifd, int ofd)
{
	int n;

	while ((n = read(ifd, readbuf, BUF_SIZE)) > 0) {
		if (n == 1 && readbuf[0] == CTRL_D)
			return;
		if (verbose) fprintf(stderr, "%d bytes read\n", n);
		write(ofd, readbuf, n);
	}
	if (n < 0) perror("read");
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "-v")) {
		verbose = 1;
		argc--;
		argv++;
	}
	if (argc > 1) {
		if ((fd = open(argv[1], O_RDONLY|O_EXCL)) < 0) {
			perror(argv[1]);
			return 1;
		}
	} else fd = STDIN_FILENO;

	if (tcgetattr(fd, &org) >= 0) {
		new = org;
		new.c_lflag &= ~(ECHO | ECHOE | ECHONL);
		new.c_lflag &= ~ICANON;
		new.c_cc[VMIN] = 1;				/* min bytes to read if VTIME = 0*/
		new.c_cc[VTIME] = 0;			/* intercharacter timeout if VMIN > 0, timeout if VMIN = 0*/
		//new.c_cflag = B19200 | CS8;
        //new.c_cflag |= CRTSCTS;
		tcsetattr(fd, TCSAFLUSH, &new);
	} else fprintf(stderr, "Can't set termios\n");
	signal(SIGINT, sig_handler);

	copyfile(fd, STDOUT_FILENO);

	tcsetattr(fd, TCSAFLUSH, &org);
	close(fd);
	return 0;
}

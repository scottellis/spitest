/*
 * Simple test of the spidev driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

// or /dev/spidev1.1 for CS 1
char spi_device[] = "/dev/spidev1.0";

void dump_buffer(const char *prompt, unsigned char *buff, int len);
int setup_spi(int fd);
int msleep(int ms);

int main(int argc, char **argv)
{
	int fd, ret, i;
	unsigned char buff[16];

	fd = open(spi_device, O_RDWR);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    if (setup_spi(fd) < 0) {
        close(fd);
        exit(1);
    }

	for (i = 0; i < 10; i++) {		
        // fake a typical transaction, write some command 
		memset(buff, 0xad, sizeof(buff));

		ret = write(fd, buff, sizeof(buff));

		if (ret < 0) {
			perror("write");
            break;
        }
		else {
			dump_buffer("wrote =>", buff, ret);
        }

        // give the other side a chance to prepare a response
		msleep(10);

        // read the response	
		ret = read(fd, buff, sizeof(buff));

		if (ret < 0) {
			perror("read");
            break;
        }
		else {
			dump_buffer("read  =>", buff, ret);
        }
	}

	close(fd);

	return 0;
}

void dump_buffer(const char *prompt, unsigned char *buff, int len)
{
	int i;

	if (prompt && *prompt)
		printf("%s ", prompt);
	else
		printf("=> ");

	for (i = 0; i < len; i++)
		printf("%02x ", buff[i]);

	printf("\n");
}

/*
  The clock comes from dividing a 48 MHz source so the options are
  24000000
  12000000
   6000000
   3000000
   1500000
    750000
    375000
    ...
  I forget how bits are in the divisor register. Might go a little lower.
  The driver will choose the highest available value less then or equal to
  what you ask for.
  This speed can be set in the kernel board file so you don't have to
  change it every time.
*/ 
int setup_spi(int fd)
{
	int ret;
	unsigned int speed = 12000000;

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

	if (ret < 0)
		perror("ioctl(SPI_IOC_WR_MAX_SPEED_HZ)");

	return ret;
}

int msleep(int ms)
{
	struct timespec ts;

	if (ms < 1)
		return -1;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = 1000000 * (ms % 1000);

	return nanosleep(&ts, NULL);
}


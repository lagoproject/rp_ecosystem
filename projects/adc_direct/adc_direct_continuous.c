/*
	 command to compile:
	 gcc -O3 -D_GNU_SOURCE adc-test-server.c -o adc-test-server
	 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define CMA_ALLOC _IOWR('Z', 0, uint32_t)

int interrupted = 0;

void signal_handler(int sig)
{
	interrupted = 1;
}

int main ()
{
	int fd, i;
	volatile void *cfg, *sts;
	volatile int16_t *ram;
	int position, limit, offset;
	volatile uint32_t *rx_addr, *rx_cntr, *rx_nsamp;
	volatile uint16_t *rx_rate;
	volatile uint8_t *rx_rst;
	uint32_t size;
	int16_t value[2];
	int yes = 1;

	if((fd = open("/dev/mem", O_RDWR)) < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40001000);
	sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40002000);

	close(fd);

	if((fd = open("/dev/cma", O_RDWR)) < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	size = 1024*sysconf(_SC_PAGESIZE);

	if(ioctl(fd, CMA_ALLOC, &size) < 0)
	{
		perror("ioctl");
		return EXIT_FAILURE;
	}

	ram = mmap(NULL, 1024*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	rx_rst = (uint8_t *)(cfg + 0);
	rx_addr = (uint32_t *)(cfg + 4);
	rx_nsamp = (uint32_t *)(cfg + 8);

	rx_cntr = (uint32_t *)(sts + 0);

	// set writer address
	*rx_addr = size;

	// set number of samples
	*rx_nsamp = 1024 * 1024 - 1;


	/* enter reset mode */
	*rx_rst &= ~4;
	usleep(100);
	*rx_rst &= ~2;

	signal(SIGINT, signal_handler);

	/* enter normal operating mode */
	*rx_rst |= 4;
	usleep(100);
	*rx_rst |= 2;

	limit = 64*1024;

	while(!interrupted)
	{
		/* read ram writer position */
		position = *rx_cntr;

		/* send 512 kB if ready, otherwise sleep 0.1 ms */
		if((limit > 0 && position > limit) || (limit == 0 && position < 64*1024))
		{
			offset = limit > 0 ? 0 : 512*1024;
			limit = limit > 0 ? 0 : 64*1024;
			// print IN1 and IN2 samples
			for(i = 0; i < 64 * 1024; ++i)
			{
				value[0] = ram[offset + 2 * i + 0];
				value[1] = ram[offset + 2 * i + 1];
				printf("%5d %5d\n", value[0], value[1]);
			}
		}
		else
		{
			usleep(100);
		}
	}

	/* enter reset mode */
	*rx_rst &= ~4;
	usleep(100);
	*rx_rst &= ~2;

	//munmap(cfg, sysconf(_SC_PAGESIZE));
	//munmap(sts, sysconf(_SC_PAGESIZE));
  //munmap(ram, sysconf(_SC_PAGESIZE));

	return EXIT_SUCCESS;
}

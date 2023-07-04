//-------------------------------------------------------------------------
// Dmitry Akimov 29/06/23
//-------------------------------------------------------------------------

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

 
#include "backgroundLayer.h"
#include "imageLayer.h"
#include "key.h"
#include "loadpng.h"

#include "bcm_host.h"

//-------------------------------------------------------------------------

#define NDEBUG
#define RED(string) "\x1b[31m" string "\x1b[0m"


enum {
	FLOOR_H_POS = 2U,
	FLOOR_L_POS = 3U,
	ARROW_BIT_POS = 5U	
};


volatile bool run = true;
const char *program = NULL;
const char *work_mode = NULL;

static void print_usage(void)
{
	fprintf(stderr, RED("Incorrect command line option %s\n"), work_mode);
	fprintf(stderr, RED("Usage: %s "), program);
	fprintf(stderr, RED("[-dynamic or -static]\n"));
	fprintf(stderr, RED("    -dynamic - transparent background for videoplay\n"));
	fprintf(stderr, RED("    -static -  user background image overlays video frames\n"));	
}


	
inline bool is_val_in_range(int x, int x0, int x1)
{
	if (x > x0 && x < x1)
	{
		return true;
	}
	
	return false;
	
}


#define MAX_READ_SIZE 12U


  
struct UartDevice {
	char* filename;
	int rate;

	int fd;
	struct termios *tty;
};



/*
 * Stop the UART device.
 *
 * @param dev points to the UART device to be stopped
 */
void uart_stop(struct UartDevice* dev) {
	free(dev->tty);
}

   
/*
 * Start the UART device.
 *
 * @param dev points to the UART device to be started, must have filename and rate populated
 * @param canonical whether to define some compatibility flags for a canonical interface
 *
 * @return - 0 if the starting procedure succeeded
 *         - negative if the starting procedure failed
 */
int uart_start(struct UartDevice* dev, bool canonical) {
	struct termios *tty;
	int fd;
	int rc;

	fd = open(dev->filename, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		printf("%s: failed to open UART device\r\n", __func__);
		return fd;
	}

	tty = malloc(sizeof(*tty));
	if (!tty) {
		printf("%s: failed to allocate UART TTY instance\r\n", __func__);
		return -ENOMEM;
	}

	memset(tty, 0, sizeof(*tty));

	/*
	 * Set baud-rate.
	 */
	tty->c_cflag |= dev->rate;

    /* Ignore framing and parity errors in input. */
    tty->c_iflag |=  IGNPAR;

    /* Use 8-bit characters. This too may affect standard streams,
     * but any sane C library can deal with 8-bit characters. */
    tty->c_cflag |=  CS8;

    /* Enable receiver. */
    tty->c_cflag |=  CREAD;

    if (canonical) {
        /* Enable canonical mode.
         * This is the most important bit, as it enables line buffering etc. */
        tty->c_lflag |= ICANON;
    } else {
        /* To maintain best compatibility with normal behaviour of terminals,
         * we set TIME=0 and MAX=1 in noncanonical mode. This means that
         * read() will block until at least one byte is available. */
        tty->c_cc[VTIME] = 0;
        tty->c_cc[VMIN] = 1;
    }

	/*
	 * Flush port.
	 */
	tcflush(fd, TCIFLUSH);

	/*
	 * Apply attributes.
	 */
	rc = tcsetattr(fd, TCSANOW, tty);
	if (rc) {
		printf("%s: failed to set attributes\r\n", __func__);
		return rc;
	}

	dev->fd = fd;
	dev->tty = tty;

	return 0;
}  
 
 
 /*
 * Read a string from the UART device.
 *
 * @param dev points to the UART device to be read from
 * @param buf points to the start of buffer to be read into
 * @param buf_len length of the buffer to be read
 *
 * @return - number of bytes read if the read procedure succeeded
 *         - negative if the read procedure failed
 */
int uart_reads(struct UartDevice* dev, char *buf, size_t buf_len) {
	int rc;

	rc = read(dev->fd, buf, buf_len - 1);
	if (rc < 0) {
		printf("%s: failed to read uart data\r\n", __func__);
		return rc;
	}

	buf[rc] = '\0';
	return rc;
}


int main(int argc, char *argv[])
{
    bcm_host_init();

	/* Get command line arguments */
	program = basename(argv[0]);
	work_mode = argv[1];
	
	if(work_mode == NULL)
	{
		print_usage();
		exit(EXIT_FAILURE);
	}
    
    if(strcmp(work_mode, "-static") == 0)
    {
        printf("Static mode\n");
        
    }
    else if (strcmp(work_mode, "-dynamic") == 0)
    {
        printf("Dynamic mode\n");
    }
    else
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

 
	struct UartDevice dev;
	int rc;

	dev.filename = "/dev/ttyAMA0";
	dev.rate = B115200;

	rc = uart_start(&dev, false);
	if (rc) {
		return rc;
	}

	char read_data[MAX_READ_SIZE];
	size_t read_data_len;

	printf("UART DEMO\r\n");


    while (1) {
		read_data_len = uart_reads(&dev, read_data, MAX_READ_SIZE);

		if (read_data_len > 0) {
			printf("%s", read_data);
		}
	}

	uart_stop(&dev);



    return 0;
}


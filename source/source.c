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
#include "uart.h"
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
	
   /*mini UART, TX-14, RX-15 */
	struct UartDevice dev;
	int rc;

	dev.filename = "/dev/ttyAMA0";
	dev.rate = B9600;

	rc = uart_start(&dev, true);
	if (rc) {
		return rc;
	}

	char read_data[10];
	size_t read_data_len;

	printf("UART DEMO\r\n");

	
    while (1)
    {
		read_data_len = uart_reads(&dev, read_data, 10);

		if (read_data_len > 0) {
			printf("%s", read_data);
		}
       
    }



    return 0;
}


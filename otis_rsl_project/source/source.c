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


	
	
	void serial_port_init(struct termios *handle, int *fd)
	{

		*fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
    
		if(*fd == -1)
		{
            fprintf(stderr, RED("Unable to open /dev/ttyAMA0 file \n"));	
            exit(EXIT_FAILURE);
		}
		else printf("/dev/ttyAMA0 opening was successful \n");
    
    
		if(tcgetattr(*fd, handle) < 0)
		{
            fprintf(stderr, RED("Unable to get /dev/ttyAMA0 config \n"));	
            exit(EXIT_FAILURE);
		}
    
		//Serial port setting up
		handle->c_iflag = 0;
		handle->c_oflag = 0;
		handle->c_lflag = 0;
		handle->c_cflag = 0;
		handle->c_cc[VMIN] = 0;
		handle->c_cc[VTIME] = 0;
		handle->c_cflag = B9600 | CS8 | CREAD;
    
		//Apply settings
		tcsetattr(*fd, TCSANOW, handle);
		
		
		close(*fd);
	}
    
    
    
  
 
 
 
int main(int argc, char *argv[])
{
	//struct termios serial;
	//int file_descriptor;
	
	
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

    //serial_port_init(&serial, &file_descriptor);
	
	char uart_rx_buffer[8];
	#if 1
   /*mini UART, TX-14, RX-15 */
	struct termios serial;
    
    int fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
    
    if(fd == -1)
    {
        fprintf(stderr, RED("Unable to open /dev/ttyAMA0 file \n"));	
        exit(EXIT_FAILURE);
    }
    else printf("/dev/ttyAMA0 opening was successful \n");
    
    
    if(tcgetattr(fd, &serial) < 0)
    {
        fprintf(stderr, RED("Unable to get /dev/ttyAMA0 config \n"));	
        exit(EXIT_FAILURE);
    }
    
    //Serial port setting up
    serial.c_iflag = 0;
    serial.c_oflag = 0;
    serial.c_lflag = 0;
    serial.c_cflag = 0;
    serial.c_cc[VMIN] = 0;
    serial.c_cc[VTIME] = 0;
    serial.c_cflag = B9600 | CS8 | CREAD;
    
    //Apply settings
    tcsetattr(fd, TCSANOW, &serial);
    
 #endif  

    while (1)
    {
        int x;	
		/*Receive first char */
		while ((x = read(fd, uart_rx_buffer, 1)) != 1 ) 
		{
			
		}
		
		//if (uart_rx_buffer[0] != '!') 
		{
			//continue;   
		}

		int bytes_read = read(fd, uart_rx_buffer, 9);
		
		printf("Receive:%s \n, %d bytes\n", uart_rx_buffer, bytes_read);
	
       
    }



    return 0;
}


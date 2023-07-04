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
	struct termios serial;
    char uart_rx_buffer[10];
	
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
    serial.c_cflag = B115200 | CS8 | CREAD;
    
    //Apply settings
	if (tcsetattr(fd, TCSANOW, &serial) < 0)
	{
		fprintf(stderr, RED("Unable to set /dev/ttyAMA0 attributes \n"));	
        exit(EXIT_FAILURE);
	}
    char msg[9];

	
    while (1)
    {
       
		int x;	
		tcflush(fd, TCIFLUSH);
		/* Ловим правильный первый байт*/
		while ((x = read(fd, uart_rx_buffer, 1)) != 1 ) {}
	
			
		if (uart_rx_buffer[0] != '!') 
		{
			continue;   
		}
		
		
		/* Чтение остальной части пакета */
		int bytes_read = read(fd, msg, 9);
		 printf("Raw message: %s, %d bytes\n", msg, bytes_read);
		
		/* Проверка корректности пакета*/
		bool is_packet_valid = (bytes_read == 7 && (uart_rx_buffer[0]  == 'm' && uart_rx_buffer[1]  == 'F' && uart_rx_buffer[bytes_read - 2]  == 'E' && uart_rx_buffer[bytes_read - 1]  == 'm'))? true : false;
		
		if(is_packet_valid)
		{
			static int floor_state[2], arrow_state[2];
			
			is_packet_valid = false;
			printf("True message: %s, %d bytes\n", uart_rx_buffer, bytes_read);
			
			int msb = uart_rx_buffer[FLOOR_H_POS] - '0', lsb = uart_rx_buffer[FLOOR_H_POS] - '0';
			
			if(is_val_in_range(msb, 0, 10) && is_val_in_range(lsb, 0, 10))
			{
				floor_state[0] = msb * 10 + lsb;	
				printf("True floor number: %d\n", floor_state[0]);				
			}
			
					
		}
		
	
       
    }



    return 0;
}


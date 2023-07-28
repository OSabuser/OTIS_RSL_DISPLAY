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
//#define DEBUG //Вывод отладочных сообщений в консоль
#define RED(string) "\x1b[31m" string "\x1b[0m"


enum {
	FLOOR_H_POS = 2U,
	FLOOR_L_POS = 3U,
	ARROW_BIT_POS = 5U	
};

typedef struct
{
    uint16_t background; //Alfa layer (0 - transparent)
    int32_t layer;
    uint32_t display_number;
    int32_t pos_X;
    int32_t pos_Y;
}image_object_t;


volatile bool run = true;
const char *program = NULL;
const char *work_mode = NULL;
bool is_static_mode = true;

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
    
static void update_picture_on_layer(IMAGE_LAYER_T *layer, const char *path_to_image)
{
    if(layer != NULL && path_to_image != NULL)
    {
        if (loadPng(&(layer->image), path_to_image) == false)
        {
            /*Ошибка декодирования! */
			fprintf(stderr, RED("unable to load %s\n"), path_to_image);
			exit(EXIT_FAILURE);
        }
    }
    else if (path_to_image == NULL)
    {
        /*Удаляем изображение*/
    }
    
}    
  
 
 
 
int main(int argc, char *argv[])
{
   
	
	
	/*DispmanX structure wrappers*/
    IMAGE_LAYER_T background_layer, right_digit_layer, arrow_layer, left_digit_layer;
	DISPMANX_DISPLAY_HANDLE_T display_1;


    /*Create init image objects*/
    image_object_t   background = 
    {   
        .background = 0x00, 
        .layer = 1, 
        .display_number = 0, 
        .pos_X = 0, 
        .pos_Y = 0
    };  
    image_object_t   right_digit = 
    {
        .background = 0x00, 
        .layer = 2, 
        .display_number = 0, 
        .pos_X = 774, 
        .pos_Y = 112
    };
    image_object_t   left_digit = 
    {
        .background = 0x00, 
        .layer = 2, 
        .display_number = 0, 
        .pos_X = 600, 
        .pos_Y = 112
    };  
    image_object_t   arrow = 
    {
        .background = 0x00, 
        .layer = 2, 
        .display_number = 0, 
        .pos_X = 625, 
        .pos_Y = 450
    };
	
	/* Получение аргументов командной строки */
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
		is_static_mode = true;   
    }
    else if (strcmp(work_mode, "-dynamic") == 0)
    {
        printf("Dynamic mode\n");
		is_static_mode = false;
    }
    else
    {
        print_usage();
        exit(EXIT_FAILURE);
    }
	
	
	bcm_host_init();
	
	/* Загрузка стартовых спрайтов*/
    update_picture_on_layer(&left_digit_layer, "./resources/0.png");
    update_picture_on_layer(&right_digit_layer, "./resources/0.png");
    //update_picture_on_layer(&arrow_layer, "./resources/ARROW_UP.png");
	
	
	int result = 0;

    
    result = vc_dispmanx_display_close(display_1);
    assert(result == 0);
    
    display_1 = vc_dispmanx_display_open(background.display_number);
    assert(display_1 != 0);
    

    DISPMANX_MODEINFO_T info;
    result = vc_dispmanx_display_get_info(display_1, &info);
    assert(result == 0);

    
    
    createResourceImageLayer(&right_digit_layer, right_digit.layer);
    createResourceImageLayer(&left_digit_layer, left_digit.layer);
    createResourceImageLayer(&arrow_layer, arrow.layer);

 

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);
	
	
	if(is_static_mode)
	{
		 update_picture_on_layer(&background_layer, "./resources/BACK.png");
		
		createResourceImageLayer(&background_layer, background.layer);
		addElementImageLayerOffset(&background_layer,
                               background.pos_X,
                               background.pos_Y,
                               display_1,
                               update);		
	}

          
    addElementImageLayerOffset(&right_digit_layer,
                               right_digit.pos_X,
                               right_digit.pos_Y,
                               display_1,
                               update);
     addElementImageLayerOffset(&left_digit_layer,
                               left_digit.pos_X,
                               left_digit.pos_Y,
                               display_1,
                               update);
    /*
	addElementImageLayerOffset(&arrow_layer,
                               arrow.pos_X,
                               arrow.pos_Y,
                               display_1,
                               update);
	*/
    result = vc_dispmanx_update_submit_sync(update);
	


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
    serial.c_cflag = B9600 | CS8 | CREAD;
    
    //Apply settings
	if (tcsetattr(fd, TCSANOW, &serial) < 0)
	{
		fprintf(stderr, RED("Unable to set /dev/ttyAMA0 attributes \n"));	
        exit(EXIT_FAILURE);
	}
    

	bool refresh_MSB = false;
	
    while (1)
    {
       
		int x;	
		
		tcflush(fd, TCIFLUSH); /*Очистка старых данных в буфере*/
		
		while ((x = read(fd, uart_rx_buffer, 1)) != 1 ) {} /* Ловим правильный первый байт*/
	
			
		if (uart_rx_buffer[0] != '!') 
		{
			continue;   
		}
		
		
		/* Чтение остальной части пакета */
		int bytes_read = read(fd, uart_rx_buffer, 9 - 1);
		uart_rx_buffer[bytes_read] = '\0';
#ifndef NDEBUG		
		printf("Raw message: %s, %d bytes\n", uart_rx_buffer, bytes_read);
#endif
		
		/* Проверка корректности пакета*/
		bool is_packet_valid = (bytes_read == 7 && (uart_rx_buffer[0]  == 'm' && uart_rx_buffer[1]  == 'F' && uart_rx_buffer[bytes_read - 1]))? true : false;
		
		if(is_packet_valid)
		{
			static int floor_state[2], arrow_state[2];
			
			is_packet_valid = false;
#ifndef NDEBUG	
			printf("True message: %s, %d bytes\n", uart_rx_buffer, bytes_read);
#endif
			
			int msb = uart_rx_buffer[FLOOR_H_POS] - '0', lsb = uart_rx_buffer[FLOOR_L_POS] - '0';
			int arrow_bits = uart_rx_buffer[ARROW_BIT_POS] - '0';
			
			if(is_val_in_range(msb, 0, 10) && is_val_in_range(lsb, -1, 10))
			{
				/*Обновление номера этажа*/
				floor_state[0] = msb * 10 + lsb;			
				printf("True floor number: %d\n", floor_state[0]);				
			}
			
			if(is_val_in_range(arrow_bits, -1, 4))
			{
				/*Обновление направления движения*/
				arrow_state[0] = arrow_bits; 
				printf("True arrow value: %d\n", arrow_state[0]);	
			}
			
			/*Обновление спрайтов номера этажа*/
			if(floor_state[0] != floor_state[1])
			{
				char pic_name[40];
					
				/* Перерисовывать старший разряд?*/
				if((floor_state[0] > 9 && floor_state[1] < 10) || (floor_state[0] < 10 && floor_state[1] > 9))
				{
					refresh_MSB = true;
				}
				else if((floor_state[0] > 19 && floor_state[1] < 20) || (floor_state[0] < 20 && floor_state[1] > 19))
				{
					refresh_MSB = true;
				}
				else
				{
					refresh_MSB = false;
				}
				
				if(floor_state[0] < 10)
				{
					
					destroyImageLayer(&right_digit_layer);

					if(refresh_MSB)
					{
						destroyImageLayer(&left_digit_layer);
					}
					
					sprintf(pic_name, "./resources/%d.png", floor_state[0] % 10);
					update_picture_on_layer(&right_digit_layer, pic_name);
					createResourceImageLayer(&right_digit_layer, right_digit.layer);
					addElementImageLayerOffset(&right_digit_layer,
									right_digit.pos_X,
									right_digit.pos_Y,
									display_1,
									update);
				}
					else 
					{
					destroyImageLayer(&right_digit_layer);

					if(refresh_MSB)
					{
						if(floor_state[1] > 9)
						{
							destroyImageLayer(&left_digit_layer);
						}
						sprintf(pic_name, "./resources/%d.png", floor_state[0] / 10);
						update_picture_on_layer(&left_digit_layer, pic_name);
						createResourceImageLayer(&left_digit_layer, right_digit.layer);
					}

					sprintf(pic_name, "./resources/%d.png", floor_state[0] % 10);
					update_picture_on_layer(&right_digit_layer, pic_name);
					createResourceImageLayer(&right_digit_layer, right_digit.layer);
					
				}//if(floor_cnt > 9)
				
			}//if(floor_state[0] != floor_state[1])
			
		
			/*Обновление спрайта направления движения*/
			if(arrow_state[0] != arrow_state[1])
			{
				char pic_name[40];
				
				
			}//if(arrow_state[0] != arrow_state[1])
			
			floor_state[1] = floor_state[0];
			arrow_state[1] = arrow_state[0];		
		}
		
	
       
    }
	
	//WHAT THE FUCK ?!!!!!//
    destroyImageLayer(&left_digit_layer);
    destroyImageLayer(&background_layer);
    destroyImageLayer(&right_digit_layer);
    destroyImageLayer(&arrow_layer);

    result = vc_dispmanx_display_close(display_1);
    assert(result == 0);


    fprintf(stderr, RED("UNINTENDED ERROR! \n"));	
    exit(EXIT_FAILURE);
}
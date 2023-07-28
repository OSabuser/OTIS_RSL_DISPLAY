//-------------------------------------------------------------------------
// @2023/24/04 Akimov Dmitry MACH UNIT LLC
//-------------------------------------------------------------------------

#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "backgroundLayer.h"
#include "imageLayer.h"
#include "key.h"
#include "loadpng.h"
#include <string.h>
#include "bcm_host.h"
#include  <wiringPi.h>
#include <errno.h>


#define NDEBUG

const char *program = NULL;

/**
 * \struct
 * @brief Парараметры программного слоя DispmanX 
 * @param background layer
 */
typedef struct LayerObject
{
    uint16_t background;        /**< Прозрачность слоя 0x00 - 0xFF */
    int32_t layer;              /**< Номер программного слоя */
    uint32_t display_number;    /**< Номер дисплея, на котором отрисовывается слой */
    int32_t pos_X;              /**< Координата Х - начала отрисовки */
    int32_t pos_Y;              /**< Координата Y - начала отрисовки */
}layer_object_t;



static void update_picture_on_layer(IMAGE_LAYER_T *layer, const char *path_to_image)
{
    if(layer != NULL && path_to_image != NULL)
    {
        if (loadPng(&(layer->image), path_to_image) == false)
        {
            /*Ошибка декодирования! */
        }
    }
    else if (path_to_image == NULL)
    {
        /*Удаляем изображение*/
    }
    
}

typedef enum {
    ON,
    OFF
} state_t;

#define AUDIO_OUTPUT_PIN 21U ///< Номер порта, отвечающий за вывод звука BCM - 5, wPi - 21 (gpio readall)


/**
 * Calculates the Area of the circle.
 * Formula: Area = PI*r^2
 * @param[in] radius
 * @param[out] area
 */ 
static void set_audio_output_state(state_t state)
{
    if (state == ON)
    {
        digitalWrite(AUDIO_OUTPUT_PIN, HIGH);
    }
    else
    {
        digitalWrite(AUDIO_OUTPUT_PIN, LOW) ;
    }
}

/**
 * Calculates the Area of the circle.
 * Formula: Area = PI*r^2
 * @param[in] radius
 * @param[out] area
 */ 
void BUTTON1_INTERRUPT(void) 
{ 
    printf("B1!!!");
}
/**
 * Calculates the Area of the circle.
 * Formula: Area = PI*r^2
 * @param[in] radius
 * @param[out] area
 */ 
void BUTTON2_INTERRUPT(void) 
{ 
    printf("B2!!!");
}

/**
 * Calculates the Area of the circle.
 * Formula: Area = PI*r^2
 * @param[in] radius
 * @param[out] area
 */ 
int main(int argc, char *argv[])
{
    //---------------------------------------------------------------------
     /*Объекты - слои DispmanX*/
    IMAGE_LAYER_T background_layer, digit_r_layer, arrow_layer, digit_l_layer, reserve_layer;

    /*Настройки слоёв */
    layer_object_t   background = 
    {   
        .background = 0x00, 
        .layer = 2, 
        .display_number = 0, 
        .pos_X = 0, 
        .pos_Y = 0, 
    };

    layer_object_t   reserve = 
    {   
        .background = 0x00, 
        .layer = 1, 
        .display_number = 0, 
        .pos_X = 0, 
        .pos_Y = 0, 
    };

    layer_object_t   digit_l = 
    {
        .background = 0xFF, 
        .layer = 3, 
        .display_number = 0, 
        .pos_X = 790, 
        .pos_Y = 225, 
    };
    layer_object_t   digit_r = 
    {
        .background = 0xFF, 
        .layer = 3, 
        .display_number = 0, 
        .pos_X = 790, 
        .pos_Y = 25, 
    }; 
    layer_object_t   arrow = 
    {
        .background = 0xFF, 
        .layer = 3, 
        .display_number = 0, 
        .pos_X = 785, 
        .pos_Y = 380, 
    };

    /* Инициализация библиотеки WiringPi */
    //wiringPiSetup(); 
   // set_audio_output_state(ON);
   // pinMode (28, INPUT);
   // pinMode (29, INPUT);
   // pullUpDnControl (28, PUD_UP);
   // pullUpDnControl (29, PUD_UP);

   // wiringPiISR (28, INT_EDGE_FALLING, &BUTTON1_INTERRUPT) ;
   // wiringPiISR (29, INT_EDGE_FALLING, &BUTTON2_INTERRUPT) ;


    /* Загрузка стартовых спрайтов*/
    update_picture_on_layer(&reserve_layer, "./resources/B1.png");
    update_picture_on_layer(&background_layer, "./resources/B1.png");
    update_picture_on_layer(&digit_r_layer, "./resources/0.png");
    update_picture_on_layer(&arrow_layer, "./resources/12.png");

    /* Инициализация DispmanX API*/
    bcm_host_init();

    /* Create display instance */
    DISPMANX_DISPLAY_HANDLE_T display = vc_dispmanx_display_open(background.display_number);
    assert(display != 0);

    DISPMANX_MODEINFO_T info;
    int result = vc_dispmanx_display_get_info(display, &info);
    assert(result == 0);

    createResourceImageLayer(&reserve_layer, reserve.layer);
    createResourceImageLayer(&background_layer, background.layer);
    createResourceImageLayer(&digit_r_layer, digit_r.layer);
    createResourceImageLayer(&arrow_layer, arrow.layer);

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
    assert(update != 0);



    addElementImageLayerOffset(&reserve_layer,
                               reserve.pos_X,
                               reserve.pos_Y,
                               display,
                               update);

    addElementImageLayerOffset(&background_layer,
                               background.pos_X,
                               background.pos_Y,
                               display,
                               update);
    
    addElementImageLayerOffset(&digit_r_layer,
                               digit_r.pos_X,
                               digit_r.pos_Y + 75,
                               display,
                               update);
    addElementImageLayerOffset(&arrow_layer,
                               arrow.pos_X,
                               arrow.pos_Y,
                               display,
                               update);

    result = vc_dispmanx_update_submit_sync(update);

    
    const int sleepMilliseconds = 500;
    uint8_t floor_cnt = 0, pic_cnt = 2, floor_state[2] = {0, 0};
    char pic_name[40];
    uint32_t currentTime = 0;
    bool refresh_pic = false, refresh_floor = false, refresh_MSB = false, moving_up = true, change_dir = false;

    usleep(sleepMilliseconds * 2000);

    while (1)
    {
        /* Обновление картинок происходит в определённые временные промежутки (зависят от refresh_pic, refresh_floor, sleepMilliseconds)*/
        refresh_pic = (currentTime % 8 == 0 && currentTime % 3 != 0) ? true : false;
        refresh_floor = (currentTime % 8 != 0 && currentTime % 3 == 0) ? true : false;

        /* Обновление номера этажа*/     
        if(refresh_floor)
        {
            /* Изменение номера текущего этажа */
            if(moving_up)
            {
                floor_cnt = (floor_cnt < 25) ? floor_cnt + 1: 1;
            }

            if(!moving_up)
            {
                floor_cnt = (floor_cnt > 2) ? floor_cnt - 1: 1;
            }


            /* Смена направления движения*/
            if(floor_cnt == 25 && moving_up)
            {
                change_dir = true;
                destroyImageLayer(&arrow_layer);
                moving_up = false;
                update_picture_on_layer(&arrow_layer, "./resources/15.png");
                createResourceImageLayer(&arrow_layer, arrow.layer);
            }

            if(floor_cnt == 1 && !moving_up)
            {
                change_dir = true;
                destroyImageLayer(&arrow_layer);
                moving_up = true;
                update_picture_on_layer(&arrow_layer, "./resources/12.png");
                createResourceImageLayer(&arrow_layer, arrow.layer);
            }

        
            floor_state[0] = floor_cnt;

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


            /*Обновление спрайтов номера этажа*/
            if(floor_cnt < 10)
            {
                destroyImageLayer(&digit_r_layer);

                if(refresh_MSB)
                {
                    destroyImageLayer(&digit_l_layer);
                }
                sprintf(pic_name, "./resources/%d.png", floor_cnt % 10);
                update_picture_on_layer(&digit_r_layer, pic_name);
                createResourceImageLayer(&digit_r_layer, digit_r.layer);
                addElementImageLayerOffset(&digit_r_layer,
                                digit_r.pos_X,
                                digit_r.pos_Y,
                                display,
                                update);
            }
            else 
            {
                destroyImageLayer(&digit_r_layer);

                if(refresh_MSB)
                {
                    if(floor_state[1] > 9)
                    {
                        destroyImageLayer(&digit_l_layer);
                    }
                    sprintf(pic_name, "./resources/%d.png", floor_cnt / 10);
                    update_picture_on_layer(&digit_l_layer, pic_name);
                    createResourceImageLayer(&digit_l_layer, digit_r.layer);
                }

                sprintf(pic_name, "./resources/%d.png", floor_cnt % 10);
                update_picture_on_layer(&digit_r_layer, pic_name);
                createResourceImageLayer(&digit_r_layer, digit_r.layer);
            }//if(floor_cnt > 9)


            floor_state[1] = floor_state[0];

            

        }

        /* Обновление спрайта фоновой картинки*/      
        if(refresh_pic)
        {
            pic_cnt = (pic_cnt < 12) ? pic_cnt + 1: 1;
            destroyImageLayer(&background_layer);
            sprintf(pic_name, "./resources/B%d.png", pic_cnt);
            update_picture_on_layer(&background_layer, pic_name);
            createResourceImageLayer(&background_layer, background.layer);
        }
        
        /* Перерисовка картинок */
        if(refresh_floor || refresh_pic)
        {
            update = vc_dispmanx_update_start(0);

            if(change_dir)
            {
                change_dir = false;
                addElementImageLayerOffset(&arrow_layer,
                                arrow.pos_X,
                                arrow.pos_Y,
                                display,
                                update);
            }

        
            if(refresh_floor)
            {
                if(floor_state[1] < 10)
                {
                    addElementImageLayerOffset(&digit_r_layer,
                                digit_r.pos_X,
                                digit_r.pos_Y + 75,
                                display,
                                update);
                }
                else
                {
                    if(refresh_MSB)
                    {
                        addElementImageLayerOffset(&digit_l_layer,
                                digit_l.pos_X,
                                digit_l.pos_Y,
                                display,
                                update);
                    }
                    if(floor_state[1] == 11 || floor_state[1] == 21)
                    {
                        addElementImageLayerOffset(&digit_r_layer,
                                digit_r.pos_X,
                                digit_r.pos_Y + 25,
                                display,
                                update);
                    }
                    else
                    {
                       addElementImageLayerOffset(&digit_r_layer,
                                digit_r.pos_X,
                                digit_r.pos_Y,
                                display,
                                update); 
                    }
                    
                }

            }//if(refresh_floor)

        
            if(refresh_pic)
            {
                addElementImageLayerOffset(&background_layer,
                                background.pos_X,
                                background.pos_Y,
                                display,
                                update);
            }

            result = vc_dispmanx_update_submit_sync(update);
        }
        

        /* Обновление резервного бэкграунд изображения*/
        if(refresh_pic)
        {
            
            sprintf(pic_name, "./resources/B%d.png", pic_cnt);
            destroyImageLayer(&reserve_layer);
            update_picture_on_layer(&reserve_layer, pic_name);
            createResourceImageLayer(&reserve_layer, reserve.layer);
            update = vc_dispmanx_update_start(0);
            addElementImageLayerOffset(&reserve_layer,
                               reserve.pos_X,
                               reserve.pos_Y,
                               display,
                               update);
            result = vc_dispmanx_update_submit_sync(update);

        }
        

        usleep(sleepMilliseconds * 1000);
        currentTime = (currentTime < UINT32_MAX - 3) ? currentTime + 1 : 0;
    }



    //WHAT THE FUCK ?!!!!!//
    destroyImageLayer(&reserve_layer);
    destroyImageLayer(&background_layer);
    destroyImageLayer(&digit_r_layer);
    destroyImageLayer(&arrow_layer);
    destroyImageLayer(&digit_l_layer);

    result = vc_dispmanx_display_close(display);
    assert(result == 0);

    //---------------------------------------------------------------------

    return 0;
}


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

    //---------------------------------------------------------------------


    // Sleep for 10 milliseconds every run-loop
    const int sleepMilliseconds = 10;
	uint32_t currentTime = 0;
	uint32_t timeout = 0;

    while (run)
    {
        
        //---------------------------------------------------------------------

        usleep(sleepMilliseconds * 1000);

        currentTime += sleepMilliseconds;
        if (timeout != 0 && currentTime >= timeout) {
            run = false;
        }
    }



    return 0;
}


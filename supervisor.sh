#!/usr/bin/env bash

# Akimov D. 2023/11/07 @ MACH UNIT

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White


# Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White


# Path to executables
EXEC_PATH=~/OTIS_RSL_DISPLAY/source
SLEEP_TIME=5

# Work until main process exists (pgrep ret code == 0)
while :
do
	echo -e "${BYellow} Routine:  Check for mounted usb devices... ${White}"
	
	# Check if usb flash drive is present as block device
    if ls /dev/sd*
    then
		# Get name of disk:
		DISK_NAME=$(ls /dev/sd* | head -n1)	
		echo  -e "${BYellow} FOUND: $DISK_NAME ${White}"
		
        # Find usb drive mountpoint path and cd to it
        MOUNT_DIR=$(lsblk -o mountpoint | grep 'media')
        cd $MOUNT_DIR
            
        # Try to find video files with estimated names 01.mp4-99.mp4
        if ls [0-9][1-9].mp4
        then
                echo -e "${BYellow} Found some videos! ${White}"
                
                # Check if output.mp4 already exists and delete if it is
                cd $EXEC_PATH
                [ -e videos/output.mp4 ] && rm videos/output.mp4
                
                # Merge videos into one file
                cd $MOUNT_DIR
                
                 # Delete temp files
                rm *.ts
                
                for video in [0-9][1-9].mp4; do
                    ffmpeg -i $video -c copy -bsf:v h264_mp4toannexb -f mpegts $video.ts
                done

                CONCAT=$(echo $(ls -v *.ts) | sed -e "s/ /|/g")
		
                ffmpeg -i "concat:$CONCAT" -c copy -bsf:a aac_adtstoasc ~/OTIS_RSL_DISPLAY/source/videos/output.mp4

                # Delete temp files
                rm *.ts
               
				echo -e "${BYellow} Dynamic image mode ${White}" 
				
                cd $EXEC_PATH
				
				
				sleep 15
			    # Umount USB drive 
				umount $MOUNT_DIR
				
				# Eject $DISK_NAME
				udisksctl power-off -b $DISK_NAME
				
                
                #pkill rasp_otis
                #omxplayer  --loop --no-osd video/output.mp4 &
                #./rasp_otis -dynamic /dev/ttyAMA0  &
				# Restart supervisor script
                #exec ./supervisor.sh
        else
            echo -e "${BRed} Video files aren't present. Static image mode ${White}"    
        fi    
           
    else 
		echo -e "${BRed} There is no usb devices present! ${White}"
       
    fi
	
	# Sleep for TIMEOUT seconds
    echo -e "${BYellow} Sleep for $SLEEP_TIME seconds ${White}"
    sleep $SLEEP_TIME
	
done


echo  -e "${BRed} OH SHIT! FATAL ERROR! We need to reboot the system!"
#reboot

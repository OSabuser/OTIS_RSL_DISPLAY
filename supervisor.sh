#!/usr/bin/env bash

# Akimov D. 2023/11/07 @ MACH UNIT

# Path to executables
EXEC_PATH=~/OTIS_RSL_DISPLAY/source
SLEEP_TIME=5

# Work until main process exists (pgrep ret code == 0)
while :
do
	echo "Routine:  Check for mounted usb devices..."
	
	# Check if usb flash drive is present as block device
    if ls /dev/sd*
    then
		# Get name of disk:
		DISK_NAME=$(ls /dev/sd* | head -n1)	
		echo "FOUND: $DISK_NAME"
		
        # Find usb drive mountpoint path and cd to it
        MOUNT_DIR=$(lsblk -o mountpoint | grep 'media')
        cd $MOUNT_DIR
            
        # Try to find video files with estimated names 01.mp4-99.mp4
        if ls [0-9][1-9].mp4
        then
                echo "Found some videos!"
                
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
               
			   
				sleep 15
			    # Umount USB drive 
				umount $MOUNT_DIR
				
				# Eject $DISK_NAME
				udisksctl power-off -b $DISK_NAME
				
                echo "Dynamic image mode" 
				
                cd $EXEC_PATH
                #pkill rasp_otis
                #omxplayer  --loop --no-osd video/output.mp4 &
                #./rasp_otis -dynamic /dev/ttyAMA0  &
				# Restart supervisor script
                #exec ./supervisor.sh
        else
            echo "Video files aren't present. Static image mode"    
        fi    
           
    else 
		echo "There is no usb devices present!"
       
    fi
	
	# Sleep for TIMEOUT seconds
    echo "Sleep for $SLEEP_TIME seconds"
    sleep $SLEEP_TIME
	
done


echo "OH SHIT! FATAL ERROR! We need to reboot the system!"
#reboot

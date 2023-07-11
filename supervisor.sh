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


# Директория с исполняемой программой и видео
EXEC_PATH=~/OTIS_RSL_DISPLAY/source

# Таймаут "опроса" о наличие смонтированной флешки
SLEEP_TIME=10

# Работаем, пока существует процесс основной программы (pgrep ret code == 0)
while :
do
	echo -e "${BYellow} Routine:  Check for mounted usb devices... ${White}"
	
	# Проверка наличия блочных устройств (по умолчанию есть только mmcblk, на котором расположены ОС и ФС
    if ls /dev/sd*
    then
		# Имя диска:
		DISK_NAME=$(ls /dev/sd* | head -n1)	
		echo  -e "${BGreen} ---------------->FOUND: $DISK_NAME ${White}"
		
        # Точка монтирования флешки
        MOUNT_DIR=$(lsblk -o mountpoint | grep 'media')
        cd $MOUNT_DIR
            
        # Поиск видеороликов с именами [1-99].mp4 на флешке
        if ls [0-9][1-9].mp4
        then
                echo -e "${BGreen} ---------------->Found some videos! ${White}"
                
                # Если в выходной директории есть ролик, удаляем его
                cd $EXEC_PATH
                [ -e videos/output.mp4 ] && rm videos/output.mp4
                
                
                cd $MOUNT_DIR
                
                # Удаление временных файлов
                rm *.ts
                
				# Склейка видео в один файл
                for video in [0-9][1-9].mp4; do
                    ffmpeg -i $video -c copy -bsf:v h264_mp4toannexb -f mpegts $video.ts
                done

                CONCAT=$(echo $(ls -v *.ts) | sed -e "s/ /|/g")
		
                ffmpeg -i "concat:$CONCAT" -c copy -bsf:a aac_adtstoasc ~/OTIS_RSL_DISPLAY/source/videos/output.mp4

                # Удаление временных файлов
                rm *.ts
               
				echo -e "${BGreen} ---------------->Dynamic image mode ${White}" 
				
                cd $EXEC_PATH
				
				
			    # Размонтирование флешки
				umount $MOUNT_DIR
				
				# Извлечение флешки
				udisksctl power-off -b $DISK_NAME
				
                
                #pkill rasp_otis
                #omxplayer  --loop --no-osd video/output.mp4 &
                #./rasp_otis -dynamic /dev/ttyAMA0  &
				# Restart supervisor script
                #exec ./supervisor.sh
        else
            echo -e "${BRed} ---------------->Video files aren't present. Static image mode ${White}"    
        fi    
           
    else 
		echo -e "${BRed} ---------------->There is no usb devices present! ${White}"
       
    fi
	
	# Таймаут $SLEEP_TIME секунд
    echo -e "${BGreen} ---------------->Sleep for $SLEEP_TIME seconds \r\n\n${White}"
    sleep $SLEEP_TIME
	
done

# Попадаем сюда, если каким-то образом основной процесс был завершён
echo  -e "${BRed} ---------------->OH SHIT! FATAL ERROR! We need to reboot the system!"
#reboot

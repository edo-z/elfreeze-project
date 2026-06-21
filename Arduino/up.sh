#!/bin/bash

# Konfigurasi Board
BOARD="esp32:esp32:esp32s3"
PORT="/dev/ttyACM0"
# PORT="/dev/ttyUSB0"
BAUD="115200"
IP="192.168.2.101"
PW="prototype1234"
PROPS="FlashSize=4M,UploadSpeed=115200,CPUFreq=240"

loading_animation() {
    echo -n "[----|      Please wait     |----]"
    while true; do
        for s in / - \\ \|; do
            printf "\r[$s----|      Please wait     |----$s]"
            sleep .1
        done
    done
}
echo "---------------------------------------"
echo " " 
echo "Building for ESP32-S3 N16R8..."
echo " "
echo "---------------------------------------"

loading_animation &
LOADDER_PID=$!

# Compile dengan optimasi flags
arduino-cli compile --fqbn $BOARD --board-options $PROPS 
kill $LOADDER_PID

if [ $? -eq 0 ]; then
    echo "---------------------------------------"
    echo " "
    echo "[V] Compilation Success. Uploading to $IP..."
    echo " "
    echo "---------------------------------------"
    loading_animation &
    UPLOAD_PID=$!
    # Upload via OTA
    arduino-cli upload -p $IP --fqbn $BOARD --upload-field password=$PW . 

    # Upload
    if [ $? -eq 0 ]; then
        kill $UPLOAD_PID
        echo "---------------------------------------"
        echo " "
        echo "[V] Upload via IP $IP Success!"
        echo " "
        echo "---------------------------------------"
    else
        kill $UPLOAD_PID
        sleep 1s
        echo "---------------------------------------"
        echo " "
        echo "[X] Upload via IP $IP Failed."
        echo " "
        echo "---------------------------------------"
        sleep 1s
        loading_animation &
        SERIAL_PID=$!
        echo "---------------------------------------"
        echo " "
        echo "[$SERIAL_PID] Attemping Serial Upload..."
        echo " "
        echo "---------------------------------------"
        sleep 1s
        kill $SERIAL_PID
        arduino-cli upload -p $PORT --fqbn $BOARD --board-options "$PROPS" .
        
        if [ $? -eq 0 ]; then
        
            echo "---------------------------------------"
            echo " "
            echo -e "[V] Upload Finished! Opening Monitor..."
            echo " "
            echo "---------------------------------------"
            
            LOG_FILE="serial-$(date +%Y%m%d-%H%M%S).log"
            echo "[LOG] Recording to $LOG_FILE"
            arduino-cli monitor -p $PORT --config baudrate=$BAUD | tee "$LOG_FILE"
            
        else        
            echo "---------------------------------------"
            echo " "
            echo   " [X] Upload Failed. Check connection or Permissions."
            echo " "
            echo "---------------------------------------"
        fi
    fi

else
    echo "---------------------------------------"
    echo " "
    echo   "\n [X]Compilation failed. Check your code."
    echo " "
    echo "---------------------------------------"
    
    exit 1
fi
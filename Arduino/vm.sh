#!/bin/bash
FQBN="esp32:esp32:esp32s3"
# COM="/dev/ttyUSB0"
COM="/dev/ttyACM0"

baudrate=115200
IP="192.168.2.101"
PW="prototype1234"

echo "Open Serial Monitor..."
# arduino-cli compile --fqbn $FQBN --verbose

if [ $? -eq 0 ]; then
    # echo "Uploading via OTA to $COM..."
    # echo "Uploading via OTA to $IP..."
    # arduino-cli upload -p $COM --fqbn $FQBN --config baudrate=115200 .
    # arduino-cli upload -p $IP --fqbn $FQBN --upload-field password=$PW .
    arduino-cli monitor -p $COM --config baudrate=115200
else
    echo "Compilation failed. Upload aborted."
fi
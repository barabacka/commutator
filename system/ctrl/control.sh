#!/bin/bash

echo "t - for turn on TV"
echo "m - for turn on MUSIC"
echo "o - for turn OFF everything"
echo "s - for get status"
echo "q - for exit"

stty -F /dev/serial0 115200 cs8 -cstopb -parenb

while read -rsn1 k
do
    case "$k" in
	"t")
	    echo "TV"
	    printf '%c' $'T' > /dev/serial0
	    ;;
	"m")
	    echo "MUSIC"
	    printf '%c' $'M' > /dev/serial0
	    ;;
	"s")
	    echo "Status"
	    printf '%c' $'G' > /dev/serial0
	    ;;
	"q")
	    echo "exit"
	    exit 0
	    ;;
	"o")
	    echo "OFF"
	    printf '%c' $'\x4F' > /dev/serial0
	    ;;
    esac
done



#!/bin/bash

stty -F /dev/serial0 115200 cs8 -cstopb -parenb
while true; do
    cat -v < /dev/serial0
done

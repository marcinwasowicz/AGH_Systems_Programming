#!/bin/bash

(./generate_writers.sh)

while [ "$(ps -aux | grep -c generate_writers)" -gt 1 ]
do
    (< /dev/linked tr -d -c "A" | wc -c)
    (< /dev/linked tr -d -c "E" | wc -c)
    (< /dev/linked tr -d -c "O" | wc -c)
    (sleep 1)
done

(< /dev/linked tr -d -c "A" | wc -c)
(< /dev/linked tr -d -c "E" | wc -c)
(< /dev/linked tr -d -c "O" | wc -c)

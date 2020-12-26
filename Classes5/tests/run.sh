#!/bin/bash

buf_1="$(printf 'A%.0s' {1..10000})"
buf_2="$(printf 'E%.0s' {1..10000})"
buf_3="$(printf 'O%.0s' {1..10000})"

(echo "${buf_1}" > /dev/linked &)
(echo "${buf_2}" > /dev/linked &)
(echo "${buf_3}" > /dev/linked &)

for _ in $(seq 30)
do
    (< /dev/linked tr -d -c "A" | wc -c)
    (< /dev/linked tr -d -c "E" | wc -c)
    (< /dev/linked tr -d -c "O" | wc -c)
    (sleep 1)
done

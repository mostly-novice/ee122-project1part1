#!/bin/sh

gcc -o server -w -lsocket -lnsl server.c;
cp server ../testArea;
cd ../testArea;
./server -t 3000 -u 4000;


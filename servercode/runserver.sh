#!/bin/sh

gcc -o server -w -lsocket -lnsl server.c;
./server -t 3000 -u 4000;


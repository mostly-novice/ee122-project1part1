#!/bin/sh

gcc -o server -w -lsocket -lnsl server.c;
./server -t 3001 -u 4001;


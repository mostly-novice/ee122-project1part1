#!/bin/bash
#Location of test client
inputreader=$1
tclient=$2
username=$3

#Tracker ip address/prt
trackerip=127.0.0.1
trackerprt=10701

#Test client I/O
client1in=client1in.txt
client1out=client1out.txt
client1compare=./comp/beforelogin.txt

echo "============================================"
echo "Testing move command before logging in"

echo -n > $client1in
echo -n > $client1out

#Check client response
(($inputreader $client1in) | ($tclient -s $trackerip -p $trackerprt > $client1out))&
echo "move east" >> $client1in

sleep 2

if diff $client1compare $client1out > /dev/null
then
    echo "Success!!!"
else
    echo "Expected Client Output:"
    cat $client1compare
fi

#Cleanup
pkill -9 -U $username inputreader
pkill -9 -U $username client
rm *.txt

sleep 1

exit 0

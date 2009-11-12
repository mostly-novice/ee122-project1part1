#!/bin/bash
#Location of binaries
inputreader=$1
tclient=$2
ttracker=$3
tfile=$4
tserver=$5
username=$6

#Tracker ip address/prt
trackerip=127.0.0.1
trackerprt=9992

#Tracker I/O
trackerin=trackerin.txt
trackerout=trackerout.txt

#Server 1
server1tcp=5552
server1udp=6662
server1in=server1in.txt

#Server 2
server2tcp=7772
server2udp=8882
server2in=server2in.txt

#Test client I/O
client1in=client1in.txt
client1out=client1out.txt
client1compare=./comp/completelogin.txt

echo "============================================"
echo "Testing successful login"

echo -n > $client1in
echo -n > $client1out
echo -n > $trackerin
echo -n > $trackerout
echo -n > $server1in
echo -n > $server2in

#Start tracker
(($inputreader $trackerin) | ($ttracker -f $tfile -p $trackerprt > $trackerout))& 
#Start first server
(($inputreader $server1in) | ($tserver -t $server1tcp -u $server1udp > /dev/null))&
#Start second server
(($inputreader $server2in) | ($tserver -t $server2tcp -u $server2udp > /dev/null))&

sleep 1

#Check client response
(($inputreader $client1in) | ($tclient -s $trackerip -p $trackerprt > $client1out))&
echo "login bobby" >> $client1in

sleep 2

if diff $client1compare $client1out > /dev/null
then
    echo "Success: Client Output!!!"
else
    echo "Expected Client Output:"
    cat $client1compare
fi

#Check tracker response
if [[ -s $trackerout ]] ; then
echo "Expected no tracker output"
else
echo "Success: Tracker Output!!!"
fi

#Cleanup
pkill -9 -U $username tracker
pkill -9 -U $username server
pkill -9 -U $username server
pkill -9 -U $username client
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
rm *.txt

sleep 1

exit 0

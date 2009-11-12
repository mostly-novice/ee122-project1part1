#!/bin/bash
#Location of binaries
inputreader=$1
tclient=$2
ttracker=$3
tfile=$4
username=$5

#Tracker ip address/prt
trackerip=127.0.0.1
trackerprt=9991

#Tracker I/O
trackerin=trackerin.txt
trackerout=trackerout.txt

#Test client I/O
client1in=client1in.txt
client1out=client1out.txt
client1compare=./comp/unavailserver.txt

echo "============================================"
echo "Testing player location request"

echo -n > $client1in
echo -n > $client1out
echo -n > $trackerin

#Start tracker
(($inputreader $trackerin) | ($ttracker -f $tfile -p $trackerprt >  $trackerout))& 

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
pkill -9 -U $username client
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
rm *.txt

sleep 1

exit 0

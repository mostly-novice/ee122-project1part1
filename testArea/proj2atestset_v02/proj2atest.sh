#!/bin/bash

#Compile input reader
gcc inputreader.c -o inputreader

#Input reader path
tireader=./inputreader

#Set default paths
tclient=./ref_bin/client
tserver=./ref_bin/server
ttracker=./ref_bin/tracker

#Location of tfiles
ttfile1=./ref_bin/tfile1.txt
ttfile2=./ref_bin/tfile2.txt
ttfile3=./ref_bin/tfile3.txt
ttfile4=./ref_bin/tfile4.txt
ttfile5=./ref_bin/tfile5.txt


#Warning
echo -e "Sample project 2a tests. Remember extra tests may be added when grading"

#Request username
echo -ne "Enter username you are logged in with (used for pkill cleanup):"
read username

#Request path
echo -ne "Which portion are you testing? 1 - Client, 2 - Server, 3 - Tracker:"
read tnum
if [ $tnum == 1 ] ; then
    echo -ne "Enter path (complete or relative) of client:"
    read tclient
elif [ $tnum == 2 ] ; then
    echo -ne "Enter path (complete or relative) of server:"
    read tserver
elif [ $tnum == 3 ] ; then
    echo -ne "Enter path (complete or relative) of tracker:"
    read ttracker
else
    echo -e "Invalid option. Exiting."
    exit 0
fi

#Make sure everything is cliean
echo "Making sure everything is clean!"
rm *.txt
pkill -9 -U $username tracker
pkill -9 -U $username server
pkill -9 -U $username server
pkill -9 -U $username client
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader
pkill -9 -U $username inputreader

#Run Tests
./beforelogin.sh $tireader $tclient $username
./unavailtracker.sh $tireader $tclient $username
./unavailserver.sh $tireader $tclient $ttracker $ttfile1 $username
./completelogin.sh $tireader $tclient $ttracker $ttfile2 $tserver $username
./movewest.sh $tireader $tclient $ttracker $ttfile3 $tserver $username
./logout.sh $tireader $tclient $ttracker $ttfile5 $tserver $username
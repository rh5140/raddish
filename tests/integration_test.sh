#!/bin/bash

location=$1

# Start the webserver
if [ "$location" == "local" ];
then
    ../build/bin/webserver ../server_config &
fi

# Run the test
# checks if command returns empty string or not
while [ -n "$2" ];
do
    tmpfile=$(mktemp /tmp/tmpfile.XXXXXXX)
    cat $2 > $tmpfile
    if [ "$location" == "local" ];
    then
        if [[ $2 == *"nc"* ]];
        then
            echo -n "127.0.0.1 8080" >> $tmpfile
        else
            echo -n "127.0.0.1:8080" >> $tmpfile
        fi
    else
        if [[ $2 == *"nc"* ]];
        then
            echo -n "35.230.62.193 80" >> $tmpfile
        else
            echo -n "35.230.62.193:80" >> $tmpfile
        fi
    fi
    if [[ $2 == *"path"* ]];
    then
        echo -n "/echo" >> $tmpfile
    fi

    # ignore comments below, for testing purposes - will remove after
    # cat $tmpfile
    # cat $4
    # bash $tmpfile > $3
    # bash $tmpfile | diff $3 -

    reg=$(<$3)

    if [ ! $( bash $tmpfile | grep -P reg ) ];
    then
        echo SUCCESS
    else
        echo FAILURE
        if [ "$location" == "local" ];
        then
            kill %1
        fi
        rm "$tmpfile"
        exit 1
    fi
    shift
    shift
done

# Stop the webserver
if [ "$location" == "local" ];
then
    kill %1
fi
rm "$tmpfile"
exit 0
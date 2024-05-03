#!/bin/bash

location=$1

# Start the webserver
if [ "$location" == "local" ];
then
    ../build/bin/webserver ../server_config & 
    until curl --silent --head --fail http://localhost:8080; 
    do
        # printf '.'
        sleep 0.1
    done
fi

# Run the test
# checks if command returns empty string or not
while [ -n "$2" ];
do
    # create temp file to customize command based on inputs
    tmpfile=$(mktemp /tmp/tmpfile.XXXXXXX)
    cat $2 > $tmpfile
    if [ "$location" == "local" ];
    then

        if [[ $2 == *"static_file"* ]];
        then
            echo -n "localhost:8080/text/lobster.txt" >> $tmpfile
        else
            echo -n "localhost:8080" >> $tmpfile
        fi
    else
        if [[ $2 == *"static_file"* ]];
        then
            echo -n "35.230.62.193:80/text/lobster.txt" >> $tmpfile
        else
            echo -n "35.230.62.193:80" >> $tmpfile
        fi
    fi
    if [[ $2 == *"path"* ]];
    then
        echo -n "/echo" >> $tmpfile
    fi

    # putting contents of file into local variable
    # for grep purposes
    reg=$(<$3)

    # P for PCRE (multiline), c for count, z for treating the matched text as a sequence of lines 
    if [ "$( bash $tmpfile | grep -Pcz "$reg" - )" != 0 ];
    then
        echo -n SUCCESS
    else
        echo -n FAILURE
        if [ "$location" == "local" ];
        then
            pkill "webserver"
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
    pkill "webserver"
fi
rm "$tmpfile"
exit 0
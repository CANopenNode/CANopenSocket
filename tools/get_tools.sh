#!/bin/bash

wget "https://github.com/robincornelius/libedssharp/raw/gh-pages/build/OpenEDSEditor-latest.zip"

if [ $? -eq 0 ] ; then
    rm -r EDSEditor
    unzip OpenEDSEditor-latest.zip -d EDSEditor
    chmod a+x EDSEditor/*.exe
    rm OpenEDSEditor-latest.zip
    echo "EDSEditor updated successfully."
fi

echo "Press enter to continue..."
read

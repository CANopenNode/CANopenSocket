#!/bin/bash

cd ../CANopenNode

doxygen > /dev/null

cd ..
ln -s CANopenNode/doc/html/index.html

echo "Press enter to continue..."
read

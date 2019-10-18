#!/usr/bin/env bash

cd sort_simple || exit
make clean; make; ./test.sh
cd ../sort_advanced || exit
make clean; make; ./test.sh
cd ../linked_list || exit
make clean; make; ./test.sh
echo "All Ok"

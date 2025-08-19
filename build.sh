#!/bin/bash

cd src || exit 1
make clean && make
cd ..
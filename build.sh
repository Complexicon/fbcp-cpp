#!/bin/bash

FLAG="-O2"

if [[ "$1" == "--debug" ]]; then
    FLAG="-g"
fi

set -o xtrace
clang++ fbcp.cpp -DDISPLAY_ILI9488 $FLAG --std=c++20 -o fbcp -lbcm_host -lpigpio
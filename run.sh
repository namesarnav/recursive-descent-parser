#!/bin/bash

gcc parser.c -o parser
echo "2*3+5" | ./parser
./parser "57*2-(3+4)"
./parser "2**3**2"
./parser "++(3+4) * 5--"

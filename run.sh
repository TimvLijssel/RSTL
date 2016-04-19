#!/bin/bash

rm -rf RSTL/
git clone https://github.com/TimvLijssel/RSTL.git
cd RSTL/
make
cat example.txt | ./parser

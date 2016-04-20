#!/bin/bash

printf "RSTL map wordt verwijderd...\n"
rm -rf RSTL/
printf "Klaar!\n\n"
printf "Clonen van RSTL starten...\n"
git clone https://github.com/TimvLijssel/RSTL.git
printf "Klaar!\n\n"
printf "Maken van RSTL starten...\n"
cd RSTL/
make
printf "Klaar!\n\n\n"

printf "Starten van example.txt in RSTL...\n"
cat example.txt | ./parser
printf "Klaar!\nProgramma eindigt\n"

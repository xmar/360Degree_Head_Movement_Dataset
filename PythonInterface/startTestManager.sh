#!/usr/bin/sh

(cd ../build && make)
source .env/bin/activate
pip install -r requirements.txt
mkdir -p Helpers/build
(cd Helpers/build && cmake ..)
(cd Helpers/build && make)
./TestManager.py

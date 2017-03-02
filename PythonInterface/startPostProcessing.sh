#!/usr/bin/sh

source .env/bin/activate
pip install -r requirements.txt
(cd Helpers/build && cmake ..)
(cd Helpers/build && make)
./PostProcessing.py --withVideo

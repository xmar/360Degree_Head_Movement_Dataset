#!/usr/bin/sh

(cd ../build && make)
source .env/bin/activate
pip install -r requirements.txt
./TestManager.py

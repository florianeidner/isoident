#!/bin/bash

echo "Setting up isoident software."

echo "Copying executable to /bin dir."
cp isoident-armv7 /bin/isoident

echo "Make file executable"
chmod 755 /bin/isoident

echo "Copy startup script to /etc/init.d"
cp S54isoident /etc/init.d/S54isoident

echo "Make script executable"
chmod 755 /etc/init.d/S54isoident

echo "Create isoident folder"
mkdir /media/disk/isodident

echo "Copy datasets to folder"
cp -a datasets /media/disk/datasets

echo "Copy webeditor files to folder"
cp -a webeditor /media/disk/webeditor

echo "Adjust /etc/nginx/nginx.conf"
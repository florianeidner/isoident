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
mkdir /media/disk/isoident

echo "Copy datasets to folder"
cp -a datasets /media/disk/isoident/datasets

echo "Copy webeditor files to folder"
cp -a webeditor /media/disk/isoident/webeditor

echo "########################################################################"
echo "#                                      				     #"
echo "# Please add the following line within http section in the nginx.conf: #"
echo "#               (default: /etc/nginx/nginx.conf)                       #"
echo "#                                      				     #"
echo "#    include /media/disk/isoident/webeditor/nginx-isoident-lua.conf;   #"
echo "#                                      				     #"
echo "########################################################################"

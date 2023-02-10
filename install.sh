#!/bin/sh

cd lunaLora | sudo make
sudo cp lunaLora  /usr/bin/
sudo cp lunaLora.service /etc/systemd/system/
sudo chmod 755 /usr/bin/lunaLora
sudo chmod +x /usr/bin/lunaLora/getIdAndRun.sh
sudo chmod 755 /etc/systemd/system/lunaLora.service
sudo systemctl enable --now lunaLora.service
# RadioAsService

if everything is set up correctly this starts the radio at boot
heavily based on fmtools

make extra hardware buttons working

    sudo cp buttons.conf /etc/triggerhappy/triggers.d/
    sudo cp /lib/systemd/system/triggerhappy.service /etc/systemd/system

change --user nobody to --user pi in

    sudo vi /etc/systemd/system/triggerhappy.service change  

maybe you need to install following packages

    sudo apt-get install -y v4l-utils ir-keytable libasound2-dev
    cd radio
    make
    sudo make install

    sudo systemctl enable radio.service

radio.service looks for .radio in pi's home for start values

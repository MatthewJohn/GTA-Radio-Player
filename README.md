# GTA Radio Player

"Grand Theft Auto radio" style mp3 player

A (very very) basic mp3 player, which keeps a global track position when switching tracks (or "stations").

The goal is to achieve a similar experience to playing the radio in Grand Theft Auto.

## Building

### Install dependencies

    sudo apt-get install --assume-yes qt5-qmake qt5-default qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5
    qmake -makefile -o Makefile
    make

## Running

Place radio station MP3 files in directory current directory, optionally setting title to name of each station.

Run the radio station:

    ./gta-radio-station


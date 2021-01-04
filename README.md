

## Building

### Install dependencies

    sudo apt-get install --assume-yes qt5-qmake qt5-default qtmultimedia5-dev libqt5multimediawidgets5 libqt5multimedia5-plugins libqt5multimedia5
    qmake -makefile -o Makefile
    make

## Running

Place radio station MP3 files in directory current directory, optionally setting title to name of each station.

Run the radio station:

    ./gta-radio-station


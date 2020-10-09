# SoETilesViewer
is a program to view and modify resources of the SNES Game Secret of Evermore.

* Only US/NTSC version
* Only uncompressed ROM supported (unpack ZIP first)
* Use right-click on tiles to import/export
* Make sure to use the same palette when exporting and importing

## How to get the source
* get source: `git clone --recurse-submodules [repo]`
* if cloned without submodules: `git submodule update --init --recursive`
* update source: `git pull --recurse-submodules`
* if unpacked from zip: download and unpack the submodules' zips as well

## How to build with Qt Creator
Install Qt Creator, for example from Qt Offline Installer, open the project and then build it.

## How to build with Debian/Ubuntu
Install `qt5-qmake` and `qt5-default`, then run `qmake SoETilesViewer.pro` and `make -j`.

## Screenshot
![screenshot](../screenshots/v017-blocks.png "Blocks Tab")

#!/bin/bash

echo "#########################################################"
echo "                      Building...                        "
echo "#########################################################"

#1) Install so-commons-library
git clone https://github.com/sisoputnfrba/so-commons-library.git
sudo make --directory=so-commons-library install
sudo rm -rf so-commons-library

#2) Build tp-2020-1c-SOVID
make --directory=delibird-commons
make --directory=broker
make --directory=game-boy
make --directory=game-card
make --directory=team

#3) Add LD_LIBRARY_PATH for delibird-commons
echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/delibird-commons/build' >> ~/.bashrc

echo "#########################################################"
echo "                         Done!                           "
echo "#########################################################"

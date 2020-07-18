#!/bin/sh

cd  ../game-boy

./build/game-boy BROKER CATCH_POKEMON Pikachu 9 3 #19b
./build/game-boy BROKER CATCH_POKEMON Squirtle 9 3 #20b

./build/game-boy BROKER CAUGHT_POKEMON 10 OK
./build/game-boy BROKER CAUGHT_POKEMON 11 FAIL

./build/game-boy BROKER CATCH_POKEMON Bulbasaur 1 7 #21
./build/game-boy BROKER CATCH_POKEMON Charmander 1 7 #22

./build/game-boy BROKER GET_POKEMON Pichu #9
./build/game-boy BROKER GET_POKEMON Raichu #10

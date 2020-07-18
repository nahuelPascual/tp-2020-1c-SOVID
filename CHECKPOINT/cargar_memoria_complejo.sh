#!/bin/sh

cd  ../game-boy

.Debug/game-boy BROKER CATCH_POKEMON Pikachu 9 3 #19b
.Debug/game-boy BROKER CATCH_POKEMON Squirtle 9 3 #20b

.Debug/game-boy BROKER CAUGHT_POKEMON 10 OK
.Debug/game-boy BROKER CAUGHT_POKEMON 11 FAIL

./Debug/game-boy BROKER CATCH_POKEMON Bulbasaur 1 7 #21
./Debug/game-boy BROKER CATCH_POKEMON Charmander 1 7 #22

./Debug/game-boy BROKER GET_POKEMON Pichu #9
./Debug/game-boy BROKER GET_POKEMON Raichu #10

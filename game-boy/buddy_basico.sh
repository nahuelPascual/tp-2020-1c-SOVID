#!/bin/sh
./build/game-boy BROKER CAUGHT_POKEMON 1 OK
./build/game-boy BROKER CAUGHT_POKEMON 2 FAIL

./build/game-boy BROKER NEW_POKEMON Pikachu 2 3 1

./build/game-boy BROKER CATCH_POKEMON Onyx 4 5

./build/game-boy SUSCRIPTOR NEW_POKEMON 10

./build/game-boy BROKER CATCH_POKEMON Charmander 4 5

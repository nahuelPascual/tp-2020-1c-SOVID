#!/bin/sh
./build/game-boy BROKER CAUGHT_POKEMON 1 OK
./build/game-boy BROKER CAUGHT_POKEMON 2 FAIL

./build/game-boy BROKER CATCH_POKEMON Pikachu 2 3
./build/game-boy BROKER CATCH_POKEMON Squirtle 5 2

./build/game-boy BROKER CATCH_POKEMON Onyx 4 5

./build/game-boy SUSCRIPTOR CAUGHT_POKEMON 10

./build/game-boy BROKER CATCH_POKEMON Vaporeon 4 5

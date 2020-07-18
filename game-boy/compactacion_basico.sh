#!/bin/sh
./Debug/game-boy BROKER CAUGHT_POKEMON 1 OK
./Debug/game-boy BROKER CAUGHT_POKEMON 2 FAIL

./Debug/game-boy BROKER CATCH_POKEMON Pikachu 2 3
./Debug/game-boy BROKER CATCH_POKEMON Squirtle 5 2

./Debug/game-boy BROKER CATCH_POKEMON Onyx 4 5

./Debug/game-boy SUSCRIPTOR CAUGHT_POKEMON 10

./Debug/game-boy BROKER CATCH_POKEMON Vaporeon 4 5

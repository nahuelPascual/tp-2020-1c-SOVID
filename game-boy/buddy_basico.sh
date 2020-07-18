#!/bin/sh
./Debug/game-boy BROKER CAUGHT_POKEMON 1 OK
./Debug/game-boy BROKER CAUGHT_POKEMON 2 FAIL

./Debug/game-boy BROKER NEW_POKEMON Pikachu 2 3 1

./Debug/game-boy BROKER CATCH_POKEMON Onyx 4 5

./Debug/game-boy SUSCRIPTOR NEW_POKEMON 10

./Debug/game-boy BROKER CATCH_POKEMON Charmander 4 5

#pragma once
#include <iostream>
#include <random>
#include "general_store.h"

int  init();
int  randomize_0_to_100();
void is_the_player_bad(int nb_tries);
bool compare(int solution, int guess, bool is_finished);
void guess_a_number();
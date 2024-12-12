#include "Probabilities.h"

#include <cassert>

bool chance(RandomNumberGeneration& rng, float chance){
    assert(chance<1 && chance>0 && "chance should be between 0 and 1");
    return RandomFloatExclusive(rng,0,1) <= chance;
}

bool chance(RandomNumberGeneration& rng, double chance){
    assert(chance<1 && chance>0 && "chance should be between 0 and 1");
    return RandomDoubleExclusive(rng,0,1) <= chance;
}

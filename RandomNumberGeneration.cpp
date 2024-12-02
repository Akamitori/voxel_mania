#include "RandomNumberGeneration.h"
#include "math_ops.h"



RandomNumberGeneration CreateRandomEngine(const unsigned int seed) {
    return RandomNumberGeneration(seed);
}

int RandomIntInclusive(RandomNumberGeneration &randomEngine, const int min, const int max) {
    if (randomEngine.min_i != min || randomEngine.max_i != max) {
        randomEngine.d_int = std::uniform_int_distribution<>{min, max};
        randomEngine.min_i = min;
        randomEngine.max_i = max;
    }
    return randomEngine.d_int(randomEngine.engine);
}

float RandomFloatExclusive(RandomNumberGeneration &randomEngine, const float min, const float max) {
    if (!math_ops::is_equal(randomEngine.min_f, min) || !math_ops::is_equal(randomEngine.max_f, max)) {
        randomEngine.d_f = std::uniform_real_distribution{min, max};
        randomEngine.min_f = min;
        randomEngine.max_f = max;
    }
    return randomEngine.d_f(randomEngine.engine);
}

double RandomDoubleExclusive(RandomNumberGeneration &randomEngine, const double min, const double max) {
    if (!math_ops::is_equal(randomEngine.min_d, min) || !math_ops::is_equal(randomEngine.max_d, max)) {
        randomEngine.d_d = std::uniform_real_distribution{min, max};
        randomEngine.min_d = min;
        randomEngine.max_d = max;
    }

    return randomEngine.d_d(randomEngine.engine);
}

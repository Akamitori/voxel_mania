#include "math_ops.h"
#include <random>
export module RandomNumberGeneration;

export class RandomNumberGeneration {
    unsigned int seed{0};
    std::mt19937 engine{0};

    explicit RandomNumberGeneration(const unsigned int seed) : seed(seed), engine(seed) {
    }

    int min_i{};
    int max_i{};
    std::uniform_int_distribution<> d_int;

    float min_f{};
    float max_f{};
    std::uniform_real_distribution<float> d_f;

    double min_d{};
    double max_d{};
    std::uniform_real_distribution<> d_d;

    friend RandomNumberGeneration CreateRandomEngine(unsigned int seed);

    friend int RandomIntInclusive(RandomNumberGeneration &randomEngine, int min, int max);

    friend float RandomFloatExclusive(RandomNumberGeneration &randomEngine, float min, float max);

    friend double RandomDoubleExclusive(RandomNumberGeneration &randomEngine, double min, double max);
};

export RandomNumberGeneration CreateRandomEngine(const unsigned int seed) {
    return RandomNumberGeneration(seed);
}

export int RandomIntInclusive(RandomNumberGeneration &randomEngine, const int min, const int max) {
    if (randomEngine.min_i != min || randomEngine.max_i != max) {
        randomEngine.d_int = std::uniform_int_distribution<>{min, max};
        randomEngine.min_i = min;
        randomEngine.max_i = max;
    }
    return randomEngine.d_int(randomEngine.engine);
}

export float RandomFloatExclusive(RandomNumberGeneration &randomEngine, const float min, const float max) {
    if (!math_ops::is_equal(randomEngine.min_f, min) || !math_ops::is_equal(randomEngine.max_f, max)) {
        randomEngine.d_f = std::uniform_real_distribution{min, max};
        randomEngine.min_f = min;
        randomEngine.max_f = max;
    }
    return randomEngine.d_f(randomEngine.engine);
}

export double RandomDoubleExclusive(RandomNumberGeneration &randomEngine, const double min, const double max) {
    if (!math_ops::is_equal(randomEngine.min_d, min) || !math_ops::is_equal(randomEngine.max_d, max)) {
        randomEngine.d_d = std::uniform_real_distribution{min, max};
        randomEngine.min_d = min;
        randomEngine.max_d = max;
    }

    return randomEngine.d_d(randomEngine.engine);
}

#ifndef RANDOMNUMBERGENERATION_H
#define RANDOMNUMBERGENERATION_H

#include <random>


class RandomNumberGeneration {
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

RandomNumberGeneration CreateRandomEngine(unsigned int seed);

int RandomIntInclusive(RandomNumberGeneration &randomEngine, int min, int max);

float RandomFloatExclusive(RandomNumberGeneration &randomEngine, float min, float max);

double RandomDoubleExclusive(RandomNumberGeneration &randomEngine, double min, double max);

#endif //RANDOMNUMBERGENERATION_H

#ifndef PERLIN_H
#define PERLIN_H
#include <array>

#include "RandomNumberGeneration.h"
#include "Vector2D.h"

struct Perlin {
    std::array<int, 512> permutation{};
    float frequency{};
    int octaves{};
    RandomNumberGeneration rng;
    std::array<Vector2D, 12> GradientVectors = {
        {
            Vector2D{1.0f, 0.0f}, // Right
            Vector2D{0.0f, 1.0f}, // Up
            Vector2D{-1.0f, 0.0f}, // Left
            Vector2D{0.0f, -1.0f}, // Down
            Vector2D{0.7071f, 0.7071f}, // Diagonal: Top-right (√2/2, √2/2)
            Vector2D{-0.7071f, 0.7071f}, // Diagonal: Top-left (-√2/2, √2/2)
            Vector2D{-0.7071f, -0.7071f}, // Diagonal: Bottom-left (-√2/2, -√2/2)
            Vector2D{0.7071f, -0.7071f}, // Diagonal: Bottom-right (√2/2, -√2/2)
            Vector2D{0.8944f, 0.4472f}, // Slightly angled: (1, 0.5) normalized
            Vector2D{-0.8944f, 0.4472f}, // Slightly angled: (-1, 0.5) normalized
            Vector2D{-0.8944f, -0.4472f}, // Slightly angled: (-1, -0.5) normalized
            Vector2D{0.8944f, -0.4472f}, // Slightly angled: (1, -0.5) normalized
        }
    };

    explicit Perlin(unsigned int seed, float frequency) : frequency{frequency}, rng{CreateRandomEngine(seed)} {
        CalculatePermutation(*this);
    }

    explicit Perlin(unsigned int seed, float frequency, int octaves) : frequency{frequency}, octaves(octaves), rng{CreateRandomEngine(seed)} {
        CalculatePermutation(*this);
    }

    friend Perlin CreatePerlin(unsigned int seed, float frequency);
    friend Perlin CreatePerlinFBM(unsigned int seed, float frequency, int octaves);

    friend double Noise2D(const Perlin &p, double x, double y);

    friend void CalculatePermutation(Perlin &p);

    friend double FBMNoise2D(const Perlin &p, double x, double y);
};

Perlin CreatePerlin(unsigned int seed, float frequency);
Perlin CreatePerlinFBM(unsigned int seed, float frequency, int octaves);

double Noise2D(const Perlin &p, double x, double y);
double FBMNoise2D(const Perlin &p, double x, double y);


#ifndef NDEBUG
void CreatePerlinNoiseImage(const Perlin &p, const std::string &filename, int width, int height);
#endif


#endif //PERLIN_H

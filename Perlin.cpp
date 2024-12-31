#include "Perlin.h"
#include <array>
#include <cassert>


#include "math_ops.h"


double fade(double t) {
    return ((6 * t - 15) * t + 10) * t * t * t;
}


Perlin CreatePerlin(const unsigned int seed, const float frequency) {
    return Perlin{seed, frequency};
}

Perlin CreatePerlinFBM(unsigned int seed, float frequency, int octaves) {
    return Perlin{seed, frequency, octaves};
}


double Noise2D(const Perlin &p, float x, float y) {
    x *= p.frequency;
    y *= p.frequency;

    const int x_int_part = static_cast<int>(std::floor(x));
    const int y_int_part = static_cast<int>(std::floor(y));

    const int X = x_int_part & 255;
    const int Y = y_int_part & 255;

    const float xf = x - static_cast<float>(x_int_part);
    const float yf = y - static_cast<float>(y_int_part);

    const Vector2D topRight{xf - 1.0f, yf - 1.0f};
    const Vector2D topLeft{xf, yf - 1.0f};
    const Vector2D bottomRight{xf - 1.0f, yf};
    const Vector2D bottomLeft{xf, yf};

    const int valueTopRight = p.permutation[p.permutation[X + 1] + Y + 1];
    const int valueTopLeft = p.permutation[p.permutation[X] + Y + 1];
    const int valueBottomRight = p.permutation[p.permutation[X + 1] + Y];
    const int valueBottomLeft = p.permutation[p.permutation[X] + Y];

    const double dotTopRight = Dot(&topRight, &p.GradientVectors[valueTopRight % p.GradientVectors.size()]);
    const double dotTopLeft = Dot(&topLeft, &p.GradientVectors[valueTopLeft % p.GradientVectors.size()]);
    const double dotBottomRight = Dot(&bottomRight, &p.GradientVectors[valueBottomRight % p.GradientVectors.size()]);
    const double dotBottomLeft = Dot(&bottomLeft, &p.GradientVectors[valueBottomLeft % p.GradientVectors.size()]);

    const double u = fade(xf);
    const double v = fade(yf);

    return math_ops::lerp(u,
                          math_ops::lerp(v, dotBottomLeft, dotTopLeft),
                          math_ops::lerp(v, dotBottomRight, dotTopRight)
    );
}

double FBMNoise2D(const Perlin &p, float x, float y) {
    double result = 0;
    double amplitude = 1.0;

    // TODO revisit this is we need thread safety as well and copying turns out to be too expensive
    // this is safe since since the mutation is reverted by the end of the call
    auto &p_mut=const_cast<Perlin&>(p);
    float original_frequency = p_mut.frequency;
    
    assert(("Octaves should be higher than 0",p_mut.octaves>0));

    for (int octaves = 0; octaves < p_mut.octaves; octaves++) {
        const double n = amplitude * Noise2D(p_mut, x, y);
        result += n;

        amplitude *= 0.5;
        p_mut.frequency *= 2.0;
    }

    p_mut.frequency = original_frequency;
    return result;
}

void CalculatePermutation(Perlin &p) {
    std::array<int, 256> permutation{};

    for (int i = 0; i < permutation.size(); i++) {
        permutation[i] = i;
    }

    for (int i = permutation.size() - 1; i > 0; i--) {
        int random_i = RandomIntInclusive(p.rng, 0, i);
        int temp = permutation[random_i];
        permutation[random_i] = permutation[i];
        permutation[i] = temp;
    }

    for (int i = 0; i < permutation.size(); i++) {
        p.permutation[256 + i] = p.permutation[i] = permutation[i];
    }
}


#ifndef NDEBUG

#include <fstream>
#include <iostream>
void Write_noise_to_file(std::ofstream& file, double noise) {
    int intensity = static_cast<int>((noise + 1.0f) * 127.5f); // Map [-1, 1] to [0, 255]
    intensity = intensity < 0 ? 0 : (intensity > 255 ? 255 : intensity);

    file << intensity << " " << intensity << " " << intensity << " ";
}

// Generate a grayscale image from Perlin noise and save it as a PPM file
void CreatePerlinNoiseImage(const Perlin &p, const std::string &filename, int width, int height) {
    std::ofstream file(filename + ".ppm", std::ios::binary);
    std::ofstream file2(filename + "_2.ppm", std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Write PPM header for a grayscale image
    file << "P3\n"; // P2 format for grayscale
    file << width << " " << height << "\n"; // Image dimensions
    file << "255\n"; // Max grayscale value

    file2 << "P3\n"; // P2 format for grayscale
    file2 << width << " " << height << "\n"; // Image dimensions
    file2 << "255\n"; // Max grayscale value
    
    // Generate Perlin noise and write pixel values
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            // Map noise value to RGB
            Write_noise_to_file(file, Noise2D(p, i, j));
            Write_noise_to_file(file2, FBMNoise2D(const_cast<Perlin&>(p),i,j));
        }
        file << "\n";
        file2 << "\n";
    }


    file.close();
    file2.close();
}

#endif
#include "Perlin.h"
#include <cassert>
#include <cstring>


#include "math_ops.h"


double fade(const double t) {
    return ((6 * t - 15) * t + 10) * t * t * t;
}


Perlin CreatePerlin(const unsigned int seed, const float frequency) {
    return Perlin{seed, frequency};
}

Perlin CreatePerlinFBM(const unsigned int seed, const float frequency, const int octaves) {
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

    const double dotTopRight = dot(&topRight, &p.GradientVectors[valueTopRight % Perlin::Gradient_Vector_Size]);
    const double dotTopLeft = dot(&topLeft, &p.GradientVectors[valueTopLeft % Perlin::Gradient_Vector_Size]);
    const double dotBottomRight = dot(&bottomRight,
                                      &p.GradientVectors[valueBottomRight % Perlin::Gradient_Vector_Size]);
    const double dotBottomLeft = dot(&bottomLeft, &p.GradientVectors[valueBottomLeft % Perlin::Gradient_Vector_Size]);

    const double u = fade(xf);
    const double v = fade(yf);

    return math_ops::lerp(u,
                          math_ops::lerp(v, dotBottomLeft, dotTopLeft),
                          math_ops::lerp(v, dotBottomRight, dotTopRight)
    );
}

double FBMNoise2D(const Perlin &p, const float x, const float y) {
    double result = 0;
    double amplitude = 1.0;

    // TODO revisit this is we need thread safety as well and copying turns out to be too expensive
    // this is safe since since the mutation is reverted by the end of the call
    auto &p_mut = const_cast<Perlin &>(p);
    const float original_frequency = p_mut.frequency;

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
    constexpr int permutation_half_size = Perlin::Permutation_Size / 2;
    constexpr int last_element_index = permutation_half_size - 1;

    int permutation[permutation_half_size]{};


    for (int i = 0; i < permutation_half_size; i++) {
        permutation[i] = i;
    }

    for (int i = last_element_index; i > 0; i--) {
        const int random_i = RandomIntInclusive(p.rng, 0, i);
        const int temp = permutation[random_i];
        permutation[random_i] = permutation[i];
        permutation[i] = temp;
    }

    for (int i = 0; i < permutation_half_size; i++) {
        p.permutation[permutation_half_size + i] = p.permutation[i] = permutation[i];
    }
}


#ifndef NDEBUG

int Write_noise_to_file(FILE *fp, const double noise) {
    int intensity = static_cast<int>((noise + 1.0f) * 127.5f); // Map [-1, 1] to [0, 255]
    intensity = intensity < 0 ? 0 : intensity > 255 ? 255 : intensity;

    if (fprintf(fp, "%d %d %d ", intensity, intensity, intensity) < 0) {
        return 0;
    }


    return 1;
}

// Generate a grayscale image from Perlin noise and save it as a PPM file
void CreatePerlinNoiseImage(const Perlin &p, const char *filename, const int width, const int height) {
    const char *ext1 = ".ppm";
    const char *ext2 = "_2.ppm";

    char file_name_1[1024];
    strcpy(file_name_1, filename);
    strcat(file_name_1, ext1);

    char file_name_2[1024];
    strcpy(file_name_2, filename);
    strcat(file_name_2, ext2);

    FILE *fp1 = fopen(file_name_1, "wb+");

    if (fp1 == nullptr) {
        fprintf(stderr, "Cannot create/truncate file: %s, %s", file_name_1, strerror(errno));
        return;
    }

    FILE *fp2 = fopen(file_name_2, "wb+");
    if (fp2 == nullptr) {
        fprintf(stderr, "Cannot create/truncate file: %s, %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_1;
        return;
    }

    if (fprintf(fp1, "P3\n") < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    if (fprintf(fp1, "%d %d\n", width, height) < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    if (fprintf(fp1, "255\n") < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    if (fprintf(fp2, "P3\n") < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    if (fprintf(fp2, "%d %d\n", width, height) < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    if (fprintf(fp2, "255\n") < 0) {
        fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
        goto CLOSE_FILE_2;
    }

    // Generate Perlin noise and write pixel values
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            // Map noise value to RGB
            if (!Write_noise_to_file(fp1, Noise2D(p, i, j))) {
                fprintf(stderr, "Error during writing noise at %s: %s", file_name_1, strerror(errno));
                goto CLOSE_FILE_2;
            }

            if (!Write_noise_to_file(fp2, FBMNoise2D(p, i, j))) {
                fprintf(stderr, "Error during writing noise at %s: %s", file_name_2, strerror(errno));
                goto CLOSE_FILE_1;
            }
        }

        if (fprintf(fp1, "\n") < 0) {
            fprintf(stderr, "Error during writing at %s: %s", file_name_1, strerror(errno));
            goto CLOSE_FILE_2;
        }

        if (fprintf(fp2, "\n") < 0) {
            fprintf(stderr, "Error during writing at %s: %s", file_name_2, strerror(errno));
            goto CLOSE_FILE_2;
        }
    }

CLOSE_FILE_2:
    if (fclose(fp2) == EOF) {
        fprintf(stderr, "Failed to close %s", file_name_2);
    };
CLOSE_FILE_1:
    if (fclose(fp1) == EOF) {
        fprintf(stderr, "Failed to close %s", file_name_1);
    };
}

#endif

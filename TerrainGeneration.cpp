#include "TerrainGeneration.h"
#include "Perlin.h"
#include "Probabilities.h"

#include "data/tile.h"
#include <cassert>

const float FlatPercentageCutOff = 0.15f;
const float SmallPercentageCutOff = 0.25f;
const float MediumPercentageCutOff = 0.35f;
const float LargePercentageCutOff = 0.45f;
const float SingularCliffUnit = 0.05f;
const float max_height=10.0f;
const int cliff_height=1;


void GenerateTerrain(Perlin &rng, int width, int height, std::vector<glm::mat4> &out_transforms,
                     std::vector<glm::vec3> &out_colors) {
    assert((out_transforms.empty() && out_colors.empty() && "Vectors provided are not empty"));


    std::vector<float> height_map;
    height_map.reserve(width * height);
    //float *height_map = new float[width * height];


    // generate a height map;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float value = FBMNoise2D(rng, x, y) + 1;
            value /= 2;
            height_map.push_back(value);
        }
    }

    float cliff_cutoff = LargePercentageCutOff;
    std::vector<std::pair<tile, float> > tiles = rolled_tiles();
    // map the height to tile data
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int index = y * width + x;
            float cliffNoise = height_map[index];

            

            int cliff_units=0;
            if (cliffNoise <= cliff_cutoff) {
                float cliff_height = (cliff_cutoff - cliffNoise) / SingularCliffUnit;
                cliff_units=static_cast<int>(std::clamp(cliff_height,1.f,max_height-1));
            }

            int current_height = 0;
            for (int i = 0; i < cliff_units*cliff_height; i++) {
                glm::mat4 transform(1);
                transform[3] = {x, y, i, 1};
                out_transforms.push_back(transform);
                out_colors.emplace_back(rock.color[0], rock.color[1], rock.color[2]);
                current_height++;
            }

            glm::mat4 transform(1);
            transform[3] = {x, y, current_height, 1};
            out_transforms.push_back(transform);
            auto picked_tile = pick_a_weighted_item(rng.rng, tiles);
            out_colors.emplace_back(picked_tile.color[0], picked_tile.color[1], picked_tile.color[2]);
        }
    }

    //delete[] height_map;
}

#ifndef TERRAINGENERATION_H
#define TERRAINGENERATION_H


#include <vector>
#include <glm/glm.hpp>
struct Perlin;

void GenerateTerrain(Perlin& rng, int width, int height, std::vector<glm::mat4>& out_transforms ,std::vector<glm::vec3>& out_colors);

#endif //TERRAINGENERATION_H

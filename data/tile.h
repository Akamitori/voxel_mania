#ifndef TILE_H
#define TILE_H

#include <vector>
#include <utility>

struct tile {
    float color[3]{};
    float weight;
};

//#define DEBUG_COLORSaaa

#ifdef DEBUG_COLORS
static const tile dirt{{0, 0, 1}, 10};
static const tile grass{{0, 1, 0}, 20};
static const tile rock{{1, 0, 0}, 5};
#endif

#ifndef DEBUG_COLORS
static const tile dirt{ {139/255.f,69/255.f,19/255.f},   10};
static const tile grass{{80/255.f, 200/255.f, 120/255.f},  20};
static const tile rock{{64/255.f,64/255.f,64/255.f},    5};
#endif

static tile roll_able_tiles[2]{
    dirt, grass
};

std::vector<std::pair<tile, float> > rolled_tiles() {
    std::vector<std::pair<tile, float> > tiles{};
    for (tile &t: roll_able_tiles) {
        tiles.emplace_back(t, t.weight);
    }
    return tiles;
}


#endif //TILE_H

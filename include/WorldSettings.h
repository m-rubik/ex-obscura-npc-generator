#pragma once
#include <string>

namespace exob {

struct WorldSettings {
    std::string settlement = "small_town";
    bool wartime = false;
    bool prohibition = false;
    bool coastal = false;
    bool miningTown = false;
    bool collegeTown = false;
    int year = 1926;
    int seed = 0;
};

} // namespace exob

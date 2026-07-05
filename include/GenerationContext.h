#pragma once
#include <random>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "npc/NPC.h"
#include "WorldSettings.h"

namespace exob {

struct GenerationContext {
    std::mt19937 rng;
    WorldSettings world;
    NPC npc;
    nlohmann::json dataRoot;
    std::vector<std::string> generationLog;

    GenerationContext(int seed = 0) : rng(seed ? seed : std::random_device{}()) {}
};

} // namespace exob

#pragma once
#include <string>
#include <vector>

namespace exob {

struct ClothingItem {
    std::string name;
    std::string slot;
    std::string location;
    std::string notes;
};

struct NPC {
    std::string name;
    std::string occupation;
    int age = 0;
    std::string gender;
    std::string race;
    std::string subrace;
    int sanityPoints = 0;
    std::vector<ClothingItem> clothing;
    std::string clothingStyle;
    std::string personality;
    std::string secret;
};

} // namespace exob

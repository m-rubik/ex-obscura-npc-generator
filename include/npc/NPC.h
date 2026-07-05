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
    std::vector<ClothingItem> clothing;
    std::string clothingStyle;
};

} // namespace exob

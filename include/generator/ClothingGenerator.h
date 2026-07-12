#pragma once
#include "GenerationContext.h"
#include "rules/ProbabilityMap.h"
#include <nlohmann/json.hpp>


using json = nlohmann::json;

namespace exob {

class ClothingGenerator {
public:
    void generate(GenerationContext& ctx);
protected:
    void seedHeadItems(GenerationContext& ctx);
    void seedBodyItems(GenerationContext& ctx);
    void seedAccessoryItems(GenerationContext& ctx);
    void seedDetailColors(GenerationContext& ctx);
    void seedDetailJewelryMaterials(GenerationContext& ctx);
    void seedDetailWearStates(GenerationContext& ctx);
    void pickHeadItem(GenerationContext& ctx);
    void pickBodyItem(GenerationContext& ctx);
    void pickAccessoryItem(GenerationContext& ctx);
    std::string pickDetailColor(GenerationContext& ctx);
    std::string pickDetailJewelryMaterial(GenerationContext& ctx);
    std::string pickDetailWearState(GenerationContext& ctx);

    const json *mClothingArray = nullptr;
    const json *mClothingDetailsArray = nullptr;
    ProbabilityMap mClothingHeadMap;
    ProbabilityMap mClothingBodyMap;
    ProbabilityMap mClothingAccessoryMap;
    ProbabilityMap mDetailColorMap;
    ProbabilityMap mDetailJewelryMaterialMap;
    ProbabilityMap mDetailWearStateMap;
    std::string mHeadItem;
    std::string mBodyItem;
    std::string mAccessoryItem;
    std::string mHeadDetailColor;
    std::string mHeadDetailWearState;
    std::string mBodyDetailColor;
    std::string mBodyDetailWearState;
    std::string mAccessoryDetailJewelryMaterial;
};

} // namespace exob

#include "rules/ProbabilityMap.h"
#include "generator/ClothingGenerator.h"
#include <nlohmann/json.hpp>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

using json = nlohmann::json;

namespace exob {

void ClothingGenerator::seedHeadItems(GenerationContext& ctx) {
    ProbabilityMap clothingHeadMap;
    mClothingHeadMap = clothingHeadMap;
    for (const auto &entry : (*mClothingArray)["head"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mClothingHeadMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::seedBodyItems(GenerationContext& ctx) {
    ProbabilityMap clothingBodyMap;
    mClothingBodyMap = clothingBodyMap;
    for (const auto &entry : (*mClothingArray)["body"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mClothingBodyMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::seedAccessoryItems(GenerationContext& ctx) {
    ProbabilityMap clothingAccessoryMap;
    mClothingAccessoryMap = clothingAccessoryMap;
    for (const auto &entry : (*mClothingArray)["accessory"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mClothingAccessoryMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::seedDetailColors(GenerationContext& ctx) {
    ProbabilityMap colorMap;
    mDetailColorMap = colorMap;
    for (const auto &entry : (*mClothingDetailsArray)["base_neutrals"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailColorMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
    for (const auto &entry : (*mClothingDetailsArray)["muted_tones"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailColorMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
    for (const auto &entry : (*mClothingDetailsArray)["rich_colors"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailColorMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
    const auto& coreColorArray =
    (ctx.npc.gender == "male")
        ? ctx.dataRoot["clothing"]["details"]["menswear_core"]
        : ctx.dataRoot["clothing"]["details"]["womenswear_core"];
    for (const auto &entry : coreColorArray) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailColorMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::seedDetailJewelryMaterials(GenerationContext& ctx) {
    ProbabilityMap jewelryMap;
    mDetailJewelryMaterialMap = jewelryMap;
    for (const auto &entry : (*mClothingDetailsArray)["jewelry_materials"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailJewelryMaterialMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::seedDetailWearStates(GenerationContext& ctx) {
    ProbabilityMap wearStateMap;
    mDetailWearStateMap = wearStateMap;
    for (const auto &entry : (*mClothingDetailsArray)["wear_states"]) {
        if (entry.contains("name") && entry.contains("weight")) {
            mDetailWearStateMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
        }
    }
}

void ClothingGenerator::pickHeadItem(GenerationContext& ctx) {
    if (!mClothingHeadMap.weights().empty()) {
        mHeadItem = mClothingHeadMap.pick(ctx.rng);
    }
}

void ClothingGenerator::pickBodyItem(GenerationContext& ctx) {
    if (!mClothingBodyMap.weights().empty()) {
        mBodyItem = mClothingBodyMap.pick(ctx.rng);
    }
}

void ClothingGenerator::pickAccessoryItem(GenerationContext& ctx) {
    if (!mClothingAccessoryMap.weights().empty()) {
        mAccessoryItem = mClothingAccessoryMap.pick(ctx.rng);
    }
}

std::string ClothingGenerator::pickDetailColor(GenerationContext& ctx) {
    if (!mDetailColorMap.weights().empty()) {
        return mDetailColorMap.pick(ctx.rng);
    }
    return "";
}

std::string ClothingGenerator::pickDetailJewelryMaterial(GenerationContext& ctx) {
    if (!mDetailJewelryMaterialMap.weights().empty()) {
        return mDetailJewelryMaterialMap.pick(ctx.rng);
    }
    return "";
}

std::string ClothingGenerator::pickDetailWearState(GenerationContext& ctx) {
    if (!mDetailWearStateMap.weights().empty()) {
        return mDetailWearStateMap.pick(ctx.rng);
    }
    return "";
}

void ClothingGenerator::generate(GenerationContext& ctx) {
    mClothingArray = (ctx.npc.gender == "male")
        ? &ctx.dataRoot["clothing"]["men"]
        : &ctx.dataRoot["clothing"]["women"];
    mClothingDetailsArray = &ctx.dataRoot["clothing"]["details"];

    seedHeadItems(ctx);
    seedBodyItems(ctx);
    seedAccessoryItems(ctx);

    pickHeadItem(ctx);
    pickBodyItem(ctx);
    pickAccessoryItem(ctx);

    seedDetailColors(ctx);
    seedDetailJewelryMaterials(ctx);
    seedDetailWearStates(ctx);

    std::string mHeadDetailColor = pickDetailColor(ctx);
    std::string mHeadDetailWearState = pickDetailWearState(ctx);
    std::string mBodyDetailColor = pickDetailColor(ctx);
    std::string mBodyDetailWearState = pickDetailWearState(ctx);
    std::string mAccessoryDetailJewelryMaterial = pickDetailJewelryMaterial(ctx);

    ctx.npc.clothing.clear();

    ctx.npc.clothing.push_back({mHeadItem, "Head", mHeadDetailColor, mHeadDetailWearState});
    ctx.npc.clothing.push_back({mBodyItem, "Body", mBodyDetailColor, mBodyDetailWearState});
    ctx.npc.clothing.push_back({mAccessoryItem, "Accessory", mAccessoryDetailJewelryMaterial, ""});

    ctx.npc.clothingStyle = mHeadDetailWearState + " " + 
                            mHeadDetailColor + " " + 
                            mHeadItem + ", " +
                            mBodyDetailWearState + " " + 
                            mBodyDetailColor + " " + 
                            mBodyItem + ", and " +
                            mAccessoryDetailJewelryMaterial + " accented " +
                            mAccessoryItem;

    ctx.generationLog.push_back(std::string("Clothing: ") + ctx.npc.clothingStyle);
}

} // namespace exob
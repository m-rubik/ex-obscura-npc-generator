#include "generator/NPCGenerator.h"
#include "generator/IdentityGenerator.h"
#include "generator/ClothingGenerator.h"
#include "rules/ProbabilityMap.h"
#include <nlohmann/json.hpp>
#include <random>
#include <vector>
#include <algorithm>
#include <iostream>

using json = nlohmann::json;

namespace exob {

void NPCGenerator::generateRace(GenerationContext& ctx) {
    // Race generation
    if (ctx.dataRoot.contains("races")) {
        const auto &races = ctx.dataRoot["races"];
        ProbabilityMap raceMap;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> subraceWeights;

        for (auto it = races.begin(); it != races.end(); ++it) {
            const std::string raceName = it.key();
            const auto &entry = it.value();
            if (entry.contains("weight") && entry["weight"].is_number_integer()) {
                raceMap.add(raceName, entry["weight"].get<int>());
            }
            if (entry.contains("subraces") && entry["subraces"].is_object()) {
                std::unordered_map<std::string, int> subs;
                for (auto subIt = entry["subraces"].begin(); subIt != entry["subraces"].end(); ++subIt) {
                    subs[subIt.key()] = subIt.value().get<int>();
                }
                subraceWeights[raceName] = subs;
            }
        }

        if (!raceMap.weights().empty()) {
            std::string chosenRace = raceMap.pick(ctx.rng);
            ctx.npc.race = chosenRace;
            if (subraceWeights.count(chosenRace)) {
                ProbabilityMap subraceMap;
                for (const auto &sub : subraceWeights[chosenRace]) {
                    subraceMap.add(sub.first, sub.second);
                }
                if (!subraceMap.weights().empty()) {
                    ctx.npc.subrace = subraceMap.pick(ctx.rng);

                    if (ctx.npc.subrace != "Standard") {
                        ctx.npc.race_full_str = ctx.npc.race + " (" + ctx.npc.subrace + ")";
                    }
                    else
                    {
                        ctx.npc.race_full_str = ctx.npc.race;   
                    }
                }
            }
            ctx.generationLog.push_back(std::string("Race: ") + ctx.npc.race + (ctx.npc.subrace.empty() ? "" : (" (" + ctx.npc.subrace + ")")));
        }
    }
}

void NPCGenerator::generateAge(GenerationContext& ctx) {
    // Age
    const auto& ageData = ctx.dataRoot["age"];

    ProbabilityMap ageMap;
    for (const auto &entry : ageData["age"]) {
        if (entry.contains("value") && entry.contains("weight")) {
            ageMap.add(std::to_string(entry["value"].get<int>()), entry["weight"].get<int>());
        }
    }

    ctx.npc.age = std::stoi(ageMap.pick(ctx.rng));

    // Age Categories: need to define this somewhere better
    // teenager: 16-17
    // college: 18-22
    // young_adult: 23-30
    // adult: 31-50
    // middle_aged: 51-65
    // senior: 66-80
    if (ctx.npc.age >= 16 && ctx.npc.age <= 17) {
        ctx.npc.ageCategory = "teenager";
    } else if (ctx.npc.age >= 18 && ctx.npc.age <= 22) {
        ctx.npc.ageCategory = "college";
    } else if (ctx.npc.age >= 23 && ctx.npc.age <= 30) {
        ctx.npc.ageCategory = "young_adult";
    } else if (ctx.npc.age >= 31 && ctx.npc.age <= 50) {
        ctx.npc.ageCategory = "adult";
    } else if (ctx.npc.age >= 51 && ctx.npc.age <= 65) {
        ctx.npc.ageCategory = "middle_aged";
    } else if (ctx.npc.age >= 66 && ctx.npc.age <= 80) {
        ctx.npc.ageCategory = "senior";
    }

    ctx.generationLog.push_back(std::string("Age: ") + std::to_string(ctx.npc.age));

}

void NPCGenerator::generateOccupation(GenerationContext& ctx) {
    // Occupation - select a main category first, then roll a job from the category's subtable
    if (ctx.dataRoot.contains("occupations") && ctx.dataRoot["occupations"].contains("categories")) {
        ProbabilityMap categoryMap;
        std::unordered_map<std::string, std::string> tableByCategory;
        for (auto &cat : ctx.dataRoot["occupations"]["categories"]) {
            std::string name = cat.value("name", "");
            int weight = cat.value("weight", 1);
            std::string table = cat.value("table", "");
            
            // Load age modifiers to occupations category
            double modifier = 1;
            const auto& ageCategories = ctx.dataRoot["modifiers"]["age"]["occupations_categories"][ctx.npc.ageCategory];
            if (ageCategories.contains(name) &&
                ageCategories[name].contains("modifier"))
            {
                modifier = ageCategories[name]["modifier"].get<double>();
            }
            weight *= modifier;

            if (!name.empty() && !table.empty()) {
                categoryMap.add(name, weight);
                tableByCategory[name] = table;
            }
        }

        if (!categoryMap.weights().empty()) {
            std::string chosenCategory = categoryMap.pick(ctx.rng);
            std::string tableName = tableByCategory[chosenCategory];
            ctx.generationLog.push_back(std::string("Occupation category: ") + chosenCategory);

            if (ctx.dataRoot["occupations"].contains("tables") && ctx.dataRoot["occupations"]["tables"].contains(tableName)) {
                auto &tableJson = ctx.dataRoot["occupations"]["tables"][tableName];
                ProbabilityMap jobMap;
                if (tableJson.contains("jobs") && tableJson["jobs"].is_array()) {
                    for (auto &job : tableJson["jobs"]) {
                        std::string name = job.value("name", "");
                        int weight = job.value("weight", 1);

                        // Apply age modifier to job weight
                        double modifier = 1;
                        const auto& ageCategories = ctx.dataRoot["modifiers"]["age"]["occupations"][ctx.npc.ageCategory];
                        if (ageCategories.contains(chosenCategory) &&
                            ageCategories[name].contains("modifier"))
                        {
                            modifier = ageCategories[name]["modifier"].get<double>();
                        }
                        weight *= modifier;

                        if (!name.empty()) {
                            jobMap.add(name, weight);
                        }
                    }
                }
                if (!jobMap.weights().empty()) {
                    ctx.npc.occupation = jobMap.pick(ctx.rng);
                    ctx.generationLog.push_back(std::string("Occupation: ") + ctx.npc.occupation);
                } else {
                    ctx.generationLog.push_back("Occupation subtable loaded but jobs were empty");
                }
            } else {
                ctx.generationLog.push_back(std::string("Occupation table missing: ") + tableName);
            }
        }
    }
    // fallback for old flat occupations list
    else {
        ProbabilityMap pm;
        if (ctx.dataRoot.contains("occupations") && ctx.dataRoot["occupations"].is_array()) {
            for (auto &o : ctx.dataRoot["occupations"]) {
                std::string name = o.value("name", "");
                int weight = o.value("weight", 1);
                if (!name.empty()) {
                    pm.add(name, weight);
                }
            }
        }
        if (!pm.weights().empty()) {
            ctx.npc.occupation = pm.pick(ctx.rng);
            ctx.generationLog.push_back(std::string("Occupation: ") + ctx.npc.occupation);
        }
    }
}

void NPCGenerator::generateClothing(GenerationContext& ctx) {
    ClothingGenerator clothingGenerator = ClothingGenerator();
    clothingGenerator.generate(ctx);
}

void NPCGenerator::generateSanity(GenerationContext& ctx) {
    // Sanity points: 55 + 2d10
    std::uniform_int_distribution<int> d10(1, 10);
    ctx.npc.sanityPoints = 55 + d10(ctx.rng) + d10(ctx.rng);
    ctx.generationLog.push_back(std::string("Sanity Points: ") + std::to_string(ctx.npc.sanityPoints));
}

void NPCGenerator::generatePersonality(GenerationContext& ctx) {
    // Personality generation
    if (ctx.dataRoot.contains("personalities")) {
        const auto &pers = ctx.dataRoot["personalities"];
        
        // Map trait keys to personality struct fields
        if (pers.contains("core_traits") && pers["core_traits"].is_array() && !pers["core_traits"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["core_traits"].size() - 1);
            ctx.npc.personality.coreTrait = pers["core_traits"][traitDist(ctx.rng)].get<std::string>();
        }
        
        if (pers.contains("social_traits") && pers["social_traits"].is_array() && !pers["social_traits"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["social_traits"].size() - 1);
            ctx.npc.personality.socialBehavior = pers["social_traits"][traitDist(ctx.rng)].get<std::string>();
        }
        
        if (pers.contains("emotional_traits") && pers["emotional_traits"].is_array() && !pers["emotional_traits"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["emotional_traits"].size() - 1);
            ctx.npc.personality.emotionalRegulation = pers["emotional_traits"][traitDist(ctx.rng)].get<std::string>();
        }
        
        if (pers.contains("mental_traits") && pers["mental_traits"].is_array() && !pers["mental_traits"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["mental_traits"].size() - 1);
            ctx.npc.personality.cognitiveTendencies = pers["mental_traits"][traitDist(ctx.rng)].get<std::string>();
        }
        
        if (pers.contains("behavioral_traits") && pers["behavioral_traits"].is_array() && !pers["behavioral_traits"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["behavioral_traits"].size() - 1);
            ctx.npc.personality.behavioralNote = pers["behavioral_traits"][traitDist(ctx.rng)].get<std::string>();
        }
        
        if (pers.contains("stress_reactions") && pers["stress_reactions"].is_array() && !pers["stress_reactions"].empty()) {
            std::uniform_int_distribution<size_t> traitDist(0, pers["stress_reactions"].size() - 1);
            ctx.npc.personality.stressReaction = pers["stress_reactions"][traitDist(ctx.rng)].get<std::string>();
        }
        
        ctx.generationLog.push_back(std::string("Personality: Core=") + ctx.npc.personality.coreTrait + 
                                    " Social=" + ctx.npc.personality.socialBehavior + 
                                    " Emotional=" + ctx.npc.personality.emotionalRegulation);

        if (pers.contains("occult_sensitivity") && pers["occult_sensitivity"].is_array() && !pers["occult_sensitivity"].empty()) {
            ProbabilityMap occultSensitivityMap;
            for (const auto &entry : pers["occult_sensitivity"]) {
                if (entry.contains("name") && entry.contains("weight")) {
                    occultSensitivityMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
                }
            }
            if (!occultSensitivityMap.weights().empty()) {
                ctx.npc.occultSensitivity = occultSensitivityMap.pick(ctx.rng);
                ctx.generationLog.push_back(std::string("Occult Sensitivity: ") + ctx.npc.occultSensitivity);
            }
        }
    }
}

void NPCGenerator::generateSecret(GenerationContext& ctx) {
    if (ctx.dataRoot.contains("secrets")) {
        const auto &secrets = ctx.dataRoot["secrets"];
        std::vector<std::string> secretPools;
        for (const auto &key : {"harmless_secrets", "social_secrets", "criminal_secrets", "occult_secrets", "identity_secrets"}) {
            if (secrets.contains(key) && secrets[key].is_array()) {
                for (const auto &secret : secrets[key]) {
                    secretPools.push_back(secret.get<std::string>());
                }
            }
        }
        if (!secretPools.empty()) {
            std::uniform_int_distribution<size_t> secretDist(0, secretPools.size() - 1);
            ctx.npc.secret = secretPools[secretDist(ctx.rng)];
            ctx.generationLog.push_back(std::string("Secret: ") + ctx.npc.secret);
        }
    }
}

void NPCGenerator::generateWealth(GenerationContext& ctx) {
    if (ctx.dataRoot.contains("wealth") && ctx.dataRoot["wealth"].contains("level")) {
        ProbabilityMap wealthMap;
        for (const auto &entry : ctx.dataRoot["wealth"]["level"]) {
            if (entry.contains("name") && entry.contains("weight")) {
                wealthMap.add(entry["name"].get<std::string>(), entry["weight"].get<int>());
            }
        }
        if (!wealthMap.weights().empty()) {
            std::string chosenWealth = wealthMap.pick(ctx.rng);
            ctx.npc.wealth = chosenWealth;
            ctx.generationLog.push_back(std::string("Wealth Level: ") + chosenWealth);
        }
    }
}

NPC NPCGenerator::generate(GenerationContext& ctx) {
    /*
    IDEAS:
    - Friends
    - Family
    - Enemies
    - (other relationships?)
    - Bio (health, illness, condition, etc)
    - Stats?
    - Notable physical things like tattoos
    - Items of note (pocketwatch, weapon, etc)

    TODO:
    - I have the main template down for Age modifying Occupation, but generating
    the full mapping is an exhaustive process, that would be best suited for AI.
    */
    
    // First, we generate the base of their identiy.
    // Gender
    // MODIFIED BY:
    // MODIFIES: Name, Clothing
    // 
    // Name
    // MODIFIED BY: Gender
    // MODIFIES:
    IdentityGenerator idg;
    idg.generate(ctx);

    generateRace(ctx);

    // Age
    // MODIFIED BY: 
    // MODIFIES: Occupation, Wealth
    generateAge(ctx);
    
    // Occupation
    // MODIFIED BY: Age
    // MODIFIES: Wealth
    generateOccupation(ctx);

    // Wealth
    // MODIFIED BY: Occupation, Age
    generateWealth(ctx);

    // Clothing
    // MODIFIED BY: Gender, Wealth
    // MODIFIES:
    generateClothing(ctx);

    generatePersonality(ctx);
    generateSecret(ctx);

    // Sanity
    // MODIFIED BY:
    // MODIFIES: 
    generateSanity(ctx);

    return ctx.npc;
}

} // namespace exob

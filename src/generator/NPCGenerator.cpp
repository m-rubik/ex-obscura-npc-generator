#include "generator/NPCGenerator.h"
#include "generator/IdentityGenerator.h"
#include "rules/ProbabilityMap.h"
#include <nlohmann/json.hpp>
#include <random>

namespace exob {

NPC NPCGenerator::generate(GenerationContext& ctx) {
    // Identity
    IdentityGenerator idg;
    idg.generate(ctx);

    // Age
    std::uniform_int_distribution<int> ageDist(18, 70);
    ctx.npc.age = ageDist(ctx.rng);
    ctx.generationLog.push_back(std::string("Age: ") + std::to_string(ctx.npc.age));

    // Occupation - select a main category first, then roll a job from the category's subtable
    if (ctx.dataRoot.contains("occupations") && ctx.dataRoot["occupations"].contains("categories")) {
        ProbabilityMap categoryMap;
        std::unordered_map<std::string, std::string> tableByCategory;
        for (auto &cat : ctx.dataRoot["occupations"]["categories"]) {
            std::string name = cat.value("name", "");
            int weight = cat.value("weight", 1);
            std::string table = cat.value("table", "");
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

    return ctx.npc;
}

} // namespace exob

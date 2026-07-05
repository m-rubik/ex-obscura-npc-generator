#include "generator/IdentityGenerator.h"
#include <random>
#include <nlohmann/json.hpp>

namespace exob {

void IdentityGenerator::generate(GenerationContext& ctx) {
    // choose gender based on simple coin flip
    std::uniform_int_distribution<int> coin(0,1);
    bool male = coin(ctx.rng) == 0;

    const auto& names = ctx.dataRoot["names"];
    const auto& firstList = male ? names["male_first"] : names["female_first"];
    const auto& surnames = names["surnames"];

    if (!firstList.is_array() || !surnames.is_array()) return;

    std::uniform_int_distribution<size_t> fd(0, firstList.size()-1);
    std::uniform_int_distribution<size_t> sd(0, surnames.size()-1);
    std::string first = firstList[fd(ctx.rng)];
    std::string sur = surnames[sd(ctx.rng)];
    ctx.npc.name = first + " " + sur;
    ctx.generationLog.push_back("Identity: chosen name " + ctx.npc.name);
}

} // namespace exob

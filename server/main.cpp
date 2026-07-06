// Minimal single-threaded HTTP server (Linux) to handle two endpoints:
// GET /npc/random and POST /npc with optional JSON body {"seed":...,"world":{...}}

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include "GenerationContext.h"
#include "generator/NPCGenerator.h"

using json = nlohmann::json;
using namespace exob;

static json loadData(const std::string& path) {
    std::ifstream in(path);
    if (!in) return json{};
    json j; in >> j; return j;
}

static std::string resolveOccupationTable(const std::string& base) {
    namespace fs = std::filesystem;
    fs::path root("data/occupations");
    fs::path candidate = root / base;
    if (fs::exists(candidate)) return candidate.string();
    fs::path alt = root / (base + "s");
    if (fs::exists(alt)) return alt.string();
    fs::path altJson = root / (base + ".json");
    if (fs::exists(altJson)) return altJson.string();
    fs::path altJsonPlural = root / (base + "s.json");
    if (fs::exists(altJsonPlural)) return altJsonPlural.string();
    return {};
}

static void send_json_response(int client_fd, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: application/json\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
    send(client_fd, resp.c_str(), resp.size(), 0);
}

static void send_html_response(int client_fd, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html; charset=utf-8\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
    send(client_fd, resp.c_str(), resp.size(), 0);
}

int main(int argc, char** argv) {
    GenerationContext ctx;
    ctx.dataRoot["names"]["male_first"] = loadData("data/names/male_first.json");
    ctx.dataRoot["names"]["female_first"] = loadData("data/names/female_first.json");
    ctx.dataRoot["names"]["surnames"] = loadData("data/names/surnames.json");
    ctx.dataRoot["clothing"]["details"] = loadData("data/clothing/details.json");
    ctx.dataRoot["clothing"]["items"] = json::array();
    ctx.dataRoot["personalities"] = loadData("data/personalities.json");
    ctx.dataRoot["secrets"] = loadData("data/secrets.json");
    ctx.dataRoot["races"] = loadData("data/races.json");

    auto men = loadData("data/clothing/men.json");
    auto women = loadData("data/clothing/women.json");
    if (men.contains("men") && men["men"].is_array()) {
        for (const auto &item : men["men"]) {
            ctx.dataRoot["clothing"]["items"].push_back(item);
        }
    }
    if (women.contains("women") && women["women"].is_array()) {
        for (const auto &item : women["women"]) {
            ctx.dataRoot["clothing"]["items"].push_back(item);
        }
    }

    ctx.dataRoot["occupations"] = loadData("data/occupations/occupations.json");
    if (ctx.dataRoot["occupations"].contains("categories")) {
        for (auto &category : ctx.dataRoot["occupations"]["categories"]) {
            std::string table = category.value("table", "");
            if (!table.empty()) {
                std::string path = resolveOccupationTable(table);
                if (!path.empty()) {
                    ctx.dataRoot["occupations"]["tables"][table] = loadData(path);
                }
            }
        }
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("socket"); return 1; }
    int opt = 1; setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 10) < 0) { perror("listen"); return 1; }

    std::cout << "Server listening on http://0.0.0.0:8080" << std::endl;

    while (true) {
        int client = accept(server_fd, nullptr, nullptr);
        if (client < 0) continue;
        // read request
        char buf[8192];
        ssize_t r = recv(client, buf, sizeof(buf)-1, 0);
        if (r <= 0) { close(client); continue; }
        buf[r] = '\0';
        std::string req(buf);
        std::istringstream rs(req);
        std::string method, path, ver;
        rs >> method >> path >> ver;

        if (method == "GET" && path == "/") {
            std::ifstream htmlFile("frontend/index.html");
            std::ostringstream htmlStream;
            htmlStream << htmlFile.rdbuf();
            send_html_response(client, htmlStream.str());
        } else if (method == "GET" && path == "/npc/random") {
            int seed = ctx.world.seed ? ctx.world.seed : std::random_device{}();
            GenerationContext localCtx(seed);
            localCtx.dataRoot = ctx.dataRoot;
            NPCGenerator gen; NPC npc = gen.generate(localCtx);
            json out = { {"name", npc.name}, {"occupation", npc.occupation}, {"age", npc.age}, {"gender", npc.gender}, {"race", npc.race}, {"subrace", npc.subrace}, {"sanity_points", npc.sanityPoints}, {"clothing_style", npc.clothingStyle}, {"personality", npc.personality}, {"secret", npc.secret}, {"log", localCtx.generationLog} };
            send_json_response(client, out.dump(2));
        } else if (method == "POST" && path == "/npc") {
            // find body (after \r\n\r\n)
            auto pos = req.find("\r\n\r\n");
            std::string body = (pos != std::string::npos) ? req.substr(pos+4) : std::string();
            try {
                auto j = json::parse(body.empty() ? "{}" : body);
                GenerationContext localCtx(j.value("seed", 0));
                if (j.contains("world")) {
                    auto w = j["world"];
                    localCtx.world.year = w.value("year", localCtx.world.year);
                    localCtx.world.coastal = w.value("coastal", localCtx.world.coastal);
                }
                localCtx.dataRoot = ctx.dataRoot;
                NPCGenerator gen; NPC npc = gen.generate(localCtx);
                json out = { {"name", npc.name}, {"occupation", npc.occupation}, {"age", npc.age}, {"gender", npc.gender}, {"race", npc.race}, {"subrace", npc.subrace}, {"sanity_points", npc.sanityPoints}, {"clothing_style", npc.clothingStyle}, {"personality", npc.personality}, {"secret", npc.secret}, {"log", localCtx.generationLog} };
                send_json_response(client, out.dump(2));
            } catch (...) {
                std::string err = "{\"error\":\"invalid json\"}";
                send_json_response(client, err);
            }
        } else if (method == "GET" && path == "/world") {
            json worldOut = json::object();
            json npcsArray = json::array();
            std::vector<std::string> npcIds;
            
            // Generate 5 NPCs for the world
            for (int i = 0; i < 5; ++i) {
                int seed = ctx.world.seed ? ctx.world.seed + i : std::random_device{}() + i;
                GenerationContext localCtx(seed);
                localCtx.dataRoot = ctx.dataRoot;
                NPCGenerator gen;
                NPC npc = gen.generate(localCtx);
                
                std::string npcId = "npc-" + std::to_string(i);
                npcIds.push_back(npcId);
                
                json npcJson = {
                    {"id", npcId},
                    {"name", npc.name},
                    {"occupation", npc.occupation},
                    {"age", npc.age},
                    {"gender", npc.gender},
                    {"race", npc.race},
                    {"subrace", npc.subrace},
                    {"sanity_points", npc.sanityPoints},
                    {"clothing_style", npc.clothingStyle},
                    {"personality", npc.personality},
                    {"secret", npc.secret}
                };
                npcsArray.push_back(npcJson);
            }
            
            // Generate some relationships between NPCs
            json relArray = json::array();
            std::vector<std::string> relTypes = {"employer", "friend", "family", "member_of", "owes_money"};
            std::mt19937 rng(ctx.world.seed ? ctx.world.seed : std::random_device{}());
            std::uniform_int_distribution<size_t> npcDist(0, npcIds.size() - 1);
            std::uniform_int_distribution<size_t> relDist(0, relTypes.size() - 1);
            
            for (int i = 0; i < 4; ++i) {
                size_t from = npcDist(rng);
                size_t to = npcDist(rng);
                if (from != to) {
                    std::string relType = relTypes[relDist(rng)];
                    json rel = {
                        {"from", npcIds[from]},
                        {"to", npcIds[to]},
                        {"type", relType},
                        {"label", relType}
                    };
                    relArray.push_back(rel);
                }
            }
            
            worldOut["npcs"] = npcsArray;
            worldOut["relationships"] = relArray;
            send_json_response(client, worldOut.dump(2));
        } else {
            std::string notfound = "{\"error\":\"not found\"}";
            send_json_response(client, notfound);
        }
        close(client);
    }

    close(server_fd);
    return 0;
}

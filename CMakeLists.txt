cmake_minimum_required(VERSION 3.14)
project(ex_obscura_npc)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

## Removed cpp-httplib FetchContent — using a minimal socket server in server/main.cpp

add_executable(server
  server/main.cpp
  src/generator/NPCGenerator.cpp
  src/generator/ClothingGenerator.cpp
  src/generator/IdentityGenerator.cpp
)

target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)

enable_testing()
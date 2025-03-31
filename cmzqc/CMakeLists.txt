cmake_minimum_required(VERSION 3.10)
project(mzqc_reader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find nlohmann_json package
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Add schema file to resources
configure_file(${CMAKE_SOURCE_DIR}/mzqc_schema.json ${CMAKE_BINARY_DIR}/mzqc_schema.json COPYONLY)

# Add the mzqc_reader executable
add_executable(mzqc_reader mzqc_reader.cpp mzqc.cpp)
target_link_libraries(mzqc_reader PRIVATE nlohmann_json::nlohmann_json)

# Add the example executable
add_executable(example example.cpp mzqc.cpp)
target_link_libraries(example PRIVATE nlohmann_json::nlohmann_json)

# Installation
install(TARGETS mzqc_reader DESTINATION bin)
install(FILES ${CMAKE_SOURCE_DIR}/mzqc_schema.json DESTINATION bin)

cmake_minimum_required(VERSION 2.8)

project(fractal_service_native)

find_package(PNG REQUIRED)

file(GLOB SOURCE_FILES "src/*.cpp" "src/*.h")
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

add_definitions(${PNG_DEFINITIONS})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC} ${PNG_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB} ${PNG_LIBRARIES})

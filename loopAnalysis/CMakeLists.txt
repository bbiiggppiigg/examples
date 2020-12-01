cmake_minimum_required(VERSION 3.4.0)
project(loopAnalysis LANGUAGES CXX)

# The main code coverage tool
add_executable(analyze_loop loopAnalysis.C)
target_include_directories(analyze_loop PRIVATE ${DYNINST_INCLUDE_DIR})
target_link_libraries(analyze_loop dyninstAPI symtabAPI parseAPI instructionAPI)


# Some builds of gcc have PIE enabled by default. Dyninst doesn't handle them
# correctly, so disable this
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(testcc PRIVATE "-fno-pie")
  target_link_options(testcc PRIVATE "-no-pie")
endif()

# A dummy target to allow building this example by itself
add_custom_target(loopAnalysis DEPENDS analze_loop)

cmake_minimum_required(VERSION 3.4.0)
project(registerUsage LANGUAGES CXX)

# The main code coverage tool
add_executable(count_regs registerUsage.C)
target_include_directories(count_regs PRIVATE ${DYNINST_INCLUDE_DIR})
target_link_libraries(count_regs common dyninstAPI symtabAPI parseAPI instructionAPI)


# A dummy target to allow building this example by itself
add_custom_target(registerUsage DEPENDS genCFG)

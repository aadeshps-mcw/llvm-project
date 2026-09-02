cmake_minimum_required(VERSION 3.20)
project(aadesh-dialect CXX C)

set(CMAKE_CXX_STANDARD 17)

find_package(MLIR REQUIRED CONFIG)

message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)
include(HandleLLVMOptions)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${MLIR_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

link_directories(${LLVM_BUILD_BINARY_DIR}/lib)
add_definitions(${LLVM_DEFINITIONS})

list(APPEND MLIR_TABLEGEN_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/include")

set(LLVM_TARGET_DEFINITIONS ${CMAKE_CURRENT_SOURCE_DIR}/include/Aadesh/AadeshOps.td)

mlir_tablegen(include/Aadesh/AadeshDialect.h.inc -gen-dialect-decls -dialect=aadesh)
mlir_tablegen(include/Aadesh/AadeshDialect.cpp.inc -gen-dialect-defs -dialect=aadesh)
mlir_tablegen(include/Aadesh/AadeshOps.h.inc -gen-op-decls)
mlir_tablegen(include/Aadesh/AadeshOps.cpp.inc -gen-op-defs)
add_public_tablegen_target(AadeshOpsIncGen)

add_subdirectory(lib/Aadesh)
add_subdirectory(tools)
add_subdirectory(test)

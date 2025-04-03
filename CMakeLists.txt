cmake_minimum_required(VERSION 3.11.0)
project(CXLMemSim VERSION 0.1.0)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif ()
execute_process(COMMAND uname -r OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
set(LINUX_SOURCE /lib/modules/${arch}/build/)

set(CMAKE_CXX_STANDARD 20)

# Forcibly link to pthread library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_subdirectory(lib/bpftime)
add_subdirectory(microbench)
#add_subdirectory(workloads)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# Handle cxxopts - either find it externally or fetch/build it
find_package(cxxopts QUIET)
if(NOT cxxopts_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        cxxopts
        GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
        GIT_TAG v3.1.1
    )
    FetchContent_MakeAvailable(cxxopts)
endif()

#find_package(spdlog REQUIRED)
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

execute_process(COMMAND uname -r OUTPUT_VARIABLE arch OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic -pthread")

add_executable(CXLMemSim ${SOURCE_FILES} src/main.cc)

include_directories(CXLMemSim include ${runtime_SOURCE_DIR}/include)
if(TARGET cxxopts::cxxopts)
    target_link_libraries(CXLMemSim cxxopts::cxxopts)
else()
    target_link_libraries(CXLMemSim cxxopts)
endif()
target_link_libraries(CXLMemSim bpftime_vm bpftime-object bpftime_base_attach_impl bpftime-agent ${CMAKE_THREAD_LIBS_INIT})

add_executable(CXLMemSimRoB ${SOURCE_FILES} src/rob.cc)

include_directories(CXLMemSimRoB include ${runtime_SOURCE_DIR}/include)
if(TARGET cxxopts::cxxopts)
    target_link_libraries(CXLMemSimRoB cxxopts::cxxopts)
else()
    target_link_libraries(CXLMemSimRoB cxxopts)
endif()
target_link_libraries(CXLMemSimRoB bpftime_vm bpftime-object bpftime_base_attach_impl bpftime-agent ${CMAKE_THREAD_LIBS_INIT})

function(bpf prefix)
    add_custom_target(${prefix}_bpf ALL
            COMMAND clang --target=bpf -nostdinc -S -I/usr/include/linux -I${CMAKE_SOURCE_DIR}/include -I${LINUX_SOURCE}/arch/x86/include -I/usr/include -I${LINUX_SOURCE}/arch/x86/include/uapi -I${LINUX_SOURCE}/arch/x86/include/generated -I${LINUX_SOURCE}/arch/x86/include/generated/uapi -I${LINUX_SOURCE}/include -I${LINUX_SOURCE}/include/uapi -I${LINUX_SOURCE}/include/generated/uapi -I${LINUX_SOURCE}/tools/testing/selftests/bpf -include ${LINUX_SOURCE}/include/linux/kconfig.h -D__KERNEL__ -D__ASM_SYSREG_H -D__BPF_TRACING__ -D__TARGET_ARCH_x86 -Wno-implicit-function-declaration -O3 -emit-llvm -g -c ${CMAKE_SOURCE_DIR}/src/${prefix}.bpf.c -o ${CMAKE_BINARY_DIR}/${prefix}.ll
            COMMAND llc -march=bpf -filetype=obj -o ${CMAKE_BINARY_DIR}/${prefix}.o ${CMAKE_BINARY_DIR}/${prefix}.ll
            )
    add_dependencies(CXLMemSim ${prefix}_bpf)
endfunction()

bpf(cxlmemsim)

file(GLOB SRC_BENCHMARKS "src/benchmarks/*.cpp")
file(GLOB SRC_APP "src/app/*.cpp")

file(GLOB INC_HASHES "src/hashes/*")
file(GLOB INC_MAPS "src/maps/*")

# external code #####################################################################

set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory("external/abseil__abseil-cpp")

# pthread, required by absl see https://github.com/abseil/abseil-cpp/issues/183#issuecomment-458453010
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# build stuff #####################################################################

# create targets for all map - hash pairs

foreach(HASH_DIR ${INC_HASHES})
    get_filename_component(HASH_NAME ${HASH_DIR} NAME_WE)
    add_custom_target("${HASH_NAME}_all") # aggregate of map targets
endforeach(HASH_DIR ${INC_HASHES})

foreach(MAP_DIR ${INC_MAPS})
    get_filename_component(MAP_NAME ${MAP_DIR} NAME_WE)
    add_custom_target("${MAP_NAME}_all") # aggregate of hash targets
    foreach(HASH_DIR ${INC_HASHES})
        # executable name: mapname_hashname
        get_filename_component(HASH_NAME ${HASH_DIR} NAME_WE)
        set(EXECUTABLE_NAME "map_bench_${MAP_NAME}__${HASH_NAME}")

        # collect .cpp files in current hash and map directory
        file(GLOB SRC_MAP_DIR "${MAP_DIR}/*.cpp")
        file(GLOB SRC_HASH_DIR "${HASH_DIR}/*.cpp")

        llvm_test_run()
        llvm_test_executable(${EXECUTABLE_NAME} ${SRC_APP} ${SRC_BENCHMARKS} ${SRC_MAP_DIR} ${SRC_HASH_DIR})
        
        target_include_directories(${EXECUTABLE_NAME} PRIVATE "src/app" "external" ${MAP_DIR} ${HASH_DIR} ${FOLLY_DIR})
        target_link_libraries(${EXECUTABLE_NAME} benchmark)

        if (EXISTS "${MAP_DIR}/dependencies.cmake")
            include("${MAP_DIR}/dependencies.cmake")
        endif ()

        if (EXISTS "${HASH_DIR}/dependencies.cmake")
            include("${HASH_DIR}/dependencies.cmake")
        endif ()

        add_dependencies("${HASH_NAME}_all" ${EXECUTABLE_NAME})
        add_dependencies("${MAP_NAME}_all" ${EXECUTABLE_NAME})
    endforeach(HASH_DIR ${INC_HASHES})
endforeach(MAP_DIR ${INC_MAPS})

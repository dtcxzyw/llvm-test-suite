add_cxx_compiler_flag(-std=c++17)

llvm_test_run()
llvm_test_executable(RandomGeneratorBenchmarks rng.cpp isaac.c)

target_include_directories(RandomGeneratorBenchmarks PRIVATE pcg-cpp/include)
target_link_libraries(RandomGeneratorBenchmarks benchmark)

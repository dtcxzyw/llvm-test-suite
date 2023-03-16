#include <app/randomseed.h>

#include <random>

uint64_t randomseed() {
    std::mt19937_64 rd;
    return std::uniform_int_distribution<uint64_t>{}(rd);
}

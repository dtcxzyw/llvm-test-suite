include(FetchContent)
FetchContent_Declare(silesia_corpus
  URL https://sun.aei.polsl.pl//~sdeor/corpus/silesia.zip
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
  URL_HASH SHA256=0626E25F45C0FFB5DC801F13B7C82A3B75743BA07E3A71835A41E3D9F63C77AF
)
FetchContent_MakeAvailable(silesia_corpus)

set(CORPUS_FILE ${CMAKE_CURRENT_BINARY_DIR}/silesia.tar)
add_custom_command(OUTPUT ${CORPUS_FILE}
COMMAND tar -cf ${CORPUS_FILE} ${silesia_corpus_SOURCE_DIR}
)
add_custom_target(silesia_corpus_tar DEPENDS ${CORPUS_FILE})
set(lzbench_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE LZBENCH_SRC_ALL ${lzbench_SOURCE_DIR}/*.cpp ${lzbench_SOURCE_DIR}/*.c ${lzbench_SOURCE_DIR}/*.cc ${lzbench_SOURCE_DIR}/*.S)
set(LZBENCH_SRC "")
foreach(SRC ${LZBENCH_SRC_ALL})
  if(
  (NOT SRC MATCHES "nvcomp")
  AND (NOT SRC MATCHES "csc")
  AND (NOT SRC MATCHES "GLZA[.]c")
  AND (NOT ((SRC MATCHES "lizard") AND (NOT SRC MATCHES "lizard_compress[.]c") AND (NOT SRC MATCHES "lizard_decompress[.]c")))
  AND (NOT SRC MATCHES "tornado")
  AND (NOT ((SRC MATCHES "lzlib") AND (NOT SRC MATCHES "lzlib[.]c")))
  AND (NOT SRC MATCHES "nakamichi")
  AND (NOT SRC MATCHES "lzsse")
  AND (NOT ((SRC MATCHES "xz/common") AND (NOT SRC MATCHES "xz/common/(alone_decoder|alone_encoder|common)[.]c")))
  AND (NOT ((SRC MATCHES "xz/check") AND (NOT SRC MATCHES "xz/check/crc32_table[.]c")))
  AND (NOT ((SRC MATCHES "xz") AND (SRC MATCHES "tablegen[.]c")))

  AND (NOT SRC MATCHES "bzip2[.]c")
  AND (NOT SRC MATCHES "bzip2recover[.]c")
  AND (NOT SRC MATCHES "dlltest[.]c")
  AND (NOT SRC MATCHES "mk251[.]c")
  AND (NOT SRC MATCHES "spewG[.]c")
  AND (NOT SRC MATCHES "unzcrash[.]c")
  AND (NOT SRC MATCHES "libcsc/decomp[.]cpp")
  AND (NOT SRC MATCHES "lz4/xxhash[.]c")
  AND (NOT SRC MATCHES "gipfeli/gipfeli_test[.]cc")
  )
    list(APPEND LZBENCH_SRC ${SRC})
  endif()
endforeach()

set(RUN_OPTIONS ${CORPUS_FILE})
llvm_multisource(lzbench ${LZBENCH_SRC})
target_include_directories(lzbench PRIVATE 
${lzbench_SOURCE_DIR} 
${lzbench_SOURCE_DIR}/brotli/include 
${lzbench_SOURCE_DIR}/libcsc 
${lzbench_SOURCE_DIR}/libdeflate 
${lzbench_SOURCE_DIR}/xpack/common 
${lzbench_SOURCE_DIR}/xz 
${lzbench_SOURCE_DIR}/xz/api 
${lzbench_SOURCE_DIR}/xz/check
${lzbench_SOURCE_DIR}/xz/common 
${lzbench_SOURCE_DIR}/xz/lz 
${lzbench_SOURCE_DIR}/xz/lzma 
${lzbench_SOURCE_DIR}/xz/rangecoder 
${lzbench_SOURCE_DIR}/zstd/lib 
${lzbench_SOURCE_DIR}/zstd/lib/common
)
target_compile_options(lzbench PRIVATE -w -fomit-frame-pointer -fstrict-aliasing -ffast-math -Wno-int-conversion -fms-extensions)
target_compile_definitions(lzbench PRIVATE HAVE_CONFIG_H FL2_SINGLETHREAD)
target_compile_definitions(lzbench PRIVATE BENCH_REMOVE_LZSSE BENCH_REMOVE_CSC BENCH_REMOVE_TORNADO)
target_compile_definitions(lzbench PRIVATE MS_x64_CALL=0)
target_link_options(lzbench PRIVATE -Wno-stringop-overflow -Wno-lto-type-mismatch)
add_dependencies(lzbench silesia_corpus_tar)

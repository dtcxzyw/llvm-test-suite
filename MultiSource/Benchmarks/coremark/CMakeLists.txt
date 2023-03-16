set(NO_REFERENCE_OUTPUT true)
list(APPEND CFLAGS -DSEED_METHOD=SEED_VOLATILE -DPERFORMANCE_RUN=1 -DITERATIONS=100000 -I ${CMAKE_CURRENT_LIST_DIR} -I ${CMAKE_CURRENT_LIST_DIR}/posix -DCOMPILER_FLAGS="" -DCOMPILER_VERSION="")

llvm_multisource(coremark
  core_list_join.c
  core_main.c
  core_matrix.c
  core_state.c
  core_util.c
  posix/core_portme.c
)

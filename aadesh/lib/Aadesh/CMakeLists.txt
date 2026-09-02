add_mlir_dialect_library(MLIRAadesh
  AadeshDialect.cpp
  AadeshOps.cpp

  DEPENDS
  AadeshOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSideEffectInterfaces
)

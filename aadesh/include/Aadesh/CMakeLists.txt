set(LLVM_TARGET_DEFINITIONS AadeshOps.td)

mlir_tablegen(AadeshDialect.h.inc -gen-dialect-decls -dialect=aadesh)
mlir_tablegen(AadeshDialect.cpp.inc -gen-dialect-defs -dialect=aadesh)
mlir_tablegen(AadeshOps.h.inc -gen-op-decls)
mlir_tablegen(AadeshOps.cpp.inc -gen-op-defs)

add_public_tablegen_target(AadeshOpsIncGen)

#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Aadesh/AadeshDialect.h"
#include "Aadesh/AadeshOps.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  mlir::registerAllDialects(registry);
  mlir::registerAllPasses();

  registry.insert<mlir::aadesh::AadeshDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Aadesh modular optimizer\n", registry));
}

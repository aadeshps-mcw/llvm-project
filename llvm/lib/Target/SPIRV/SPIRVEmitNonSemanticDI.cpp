#include "MCTargetDesc/SPIRVBaseInfo.h"
#include "MCTargetDesc/SPIRVMCTargetDesc.h"
#include "SPIRV.h"
#include "SPIRVGlobalRegistry.h"
#include "SPIRVRegisterInfo.h"
#include "SPIRVTargetMachine.h"
#include "SPIRVUtils.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugProgramInstruction.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Path.h"

#define DEBUG_TYPE "spirv-nonsemantic-debug-info"

using namespace llvm;

namespace {
struct SPIRVCodeGenContext {
  MachineIRBuilder &MIRBuilder;
  MachineRegisterInfo &MRI;
  SPIRVGlobalRegistry *GR;
  const SPIRVType *VoidTy;
  const SPIRVType *I32Ty;
  const SPIRVInstrInfo *TII;
  const SPIRVRegisterInfo *TRI;
  const RegisterBankInfo *RBI;
  MachineFunction &MF;
  const Register &I32ZeroReg;
  SPIRVTargetMachine *TM;

  // Constructor to initialize all members
  SPIRVCodeGenContext(MachineIRBuilder &Builder, MachineRegisterInfo &RegInfo,
                      SPIRVGlobalRegistry *Registry, const SPIRVType *VTy,
                      const SPIRVType *I32Ty, const SPIRVInstrInfo *TI,
                      const SPIRVRegisterInfo *TR, const RegisterBankInfo *RB,
                      MachineFunction &Function, const Register &ZeroReg,
                      SPIRVTargetMachine *TargetMachine)
      : MIRBuilder(Builder), MRI(RegInfo), GR(Registry), VoidTy(VTy),
        I32Ty(I32Ty), TII(TI), TRI(TR), RBI(RB), MF(Function),
        I32ZeroReg(ZeroReg), TM(TargetMachine) {}
};
struct SPIRVEmitNonSemanticDI : public MachineFunctionPass {
  static char ID;
  SPIRVTargetMachine *TM;
  SPIRVEmitNonSemanticDI(SPIRVTargetMachine *TM = nullptr)
      : MachineFunctionPass(ID), TM(TM) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool IsGlobalDIEmitted = false;
  bool emitGlobalDI(MachineFunction &MF);
  Register EmitOpString(StringRef, SPIRVCodeGenContext &Ctx);

  Register EmitDIInstruction(SPIRV::NonSemanticExtInst::NonSemanticExtInst Inst,
                             ArrayRef<Register> Operands,
                             SPIRVCodeGenContext &Ctx);

  void emitDebugBuildIdentifier(StringRef BuildIdentifier,
                                SPIRVCodeGenContext &Ctx);

  void emitDebugStoragePath(StringRef BuildStoragePath,
                            SPIRVCodeGenContext &Ctx);

  void emitDebugLineInstructions(SPIRVCodeGenContext &Ctx,
                                 Register DebugSourceResIdReg);
  void emitDebugLinePerInstruction(MachineInstr &MI, SPIRVCodeGenContext &Ctx,
                                   Register DebugSourceResIdReg);

  Register emitDebugGlobalVariable(const DIGlobalVariableExpression *GVE,
                                   SPIRVCodeGenContext &Ctx,
                                   const Register &DebugSourceResIdReg,
                                   const Register &DebugCompUnitResIdReg);

  void emitDebugBasicTypes(
      const SmallPtrSetImpl<DIBasicType *> &BasicTypes,
      SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SPIRVCodeGenContext &Ctx);

  void emitDebugTypeInheritance(
      const SmallPtrSetImpl<DIDerivedType *> &InheritedTypes,
      SPIRVCodeGenContext &Ctx);

  void emitDebugQualifiedTypes(
      const SmallPtrSetImpl<DIDerivedType *> &QualifiedDerivedTypes,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SPIRVCodeGenContext &Ctx);

  void emitDebugTypedefs(
      const SmallPtrSetImpl<DIDerivedType *> &TypedefTypes,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg);

  void emitDebugImportedEntities(
      const SmallVectorImpl<const DIImportedEntity *> &ImportedEntities,
      SPIRVCodeGenContext &Ctx);

  void emitDebugArrayTypes(
      const SmallPtrSetImpl<DICompositeType *> &ArrayTypes,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SPIRVCodeGenContext &Ctx);

  void emitDebugVectorTypes(DICompositeType *ArrayTy, Register BaseTypeReg,
                            SPIRVCodeGenContext &Ctx);

  void emitDebugPointerTypes(
      const SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SPIRVCodeGenContext &Ctx);

  void emitDebugMacroDefs(const DICompileUnit *CU, SPIRVCodeGenContext &Ctx);

  void emitDebugMacroUndef(const DIMacro *MacroUndef, StringRef FileName,
                           SPIRVCodeGenContext &Ctx,
                           const DenseMap<StringRef, Register> &MacroDefRegs);

  uint32_t mapDwarfTagToTypeQualifier(unsigned Tag);
  Register findEmittedBasicTypeReg(
      const DIType *BaseType,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void
  extractTypeMetadata(DIType *Ty, SmallPtrSetImpl<DIBasicType *> &BasicTypes,
                      SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
                      SmallPtrSetImpl<DIDerivedType *> &QualifiedDerivedTypes,
                      SmallPtrSetImpl<DIDerivedType *> &TypedefTypes,
                      SmallPtrSetImpl<DICompositeType *> &ArrayTypes,
                      SmallPtrSetImpl<const DICompositeType *> &CompositeTypes,
                      SmallPtrSetImpl<DIDerivedType *> &InheritedTypes);

  void emitTemplateDebugInstructions(
      const DICompositeType *CompTy, SPIRVCodeGenContext &Ctx,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      const Register DebugSourceResIdReg);

  void emitDebugTypeComposite(
      const DICompositeType *CompTy, SPIRVCodeGenContext &Ctx,
      const Register &SourceReg, const Register &CUReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitDebugTypeMember(
      const DIDerivedType *Member, SPIRVCodeGenContext &Ctx,
      const Register &CompositeReg, const Register &SourceReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SmallVectorImpl<Register> &MemberRegs);

  void emitDebugTypeEnum(
      const DICompositeType *EnumTy, SPIRVCodeGenContext &Ctx,
      const Register &SourceReg, const Register &CUReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitSingleCompilationUnit(StringRef FilePath, int64_t SourceLanguage,
                                 SPIRVCodeGenContext &Ctx,
                                 Register DebugInfoVersionReg,
                                 Register DwarfVersionReg);
};
} // anonymous namespace

INITIALIZE_PASS(SPIRVEmitNonSemanticDI, DEBUG_TYPE,
                "SPIRV NonSemantic.Shader.DebugInfo.100 emitter", false, false)

char SPIRVEmitNonSemanticDI::ID = 0;

MachineFunctionPass *
llvm::createSPIRVEmitNonSemanticDIPass(SPIRVTargetMachine *TM) {
  return new SPIRVEmitNonSemanticDI(TM);
}

enum BaseTypeAttributeEncoding {
  Unspecified = 0,
  Address = 1,
  Boolean = 2,
  Float = 3,
  Signed = 4,
  SignedChar = 5,
  Unsigned = 6,
  UnsignedChar = 7
};

enum SourceLanguage {
  Unknown = 0,
  ESSL = 1,
  GLSL = 2,
  OpenCL_C = 3,
  OpenCL_CPP = 4,
  HLSL = 5,
  CPP_for_OpenCL = 6,
  SYCL = 7,
  HERO_C = 8,
  NZSL = 9,
  WGSL = 10,
  Slang = 11,
  Zig = 12
};

bool SPIRVEmitNonSemanticDI::emitGlobalDI(MachineFunction &MF) {
  // If this MachineFunction doesn't have any BB repeat procedure
  // for the next
  if (MF.begin() == MF.end()) {
    IsGlobalDIEmitted = false;
    return false;
  }

  LLVMContext *Context;
  SmallVector<SmallString<128>> FilePaths;
  SmallVector<int64_t> LLVMSourceLanguages;
  int64_t DwarfVersion = 0;
  int64_t DebugInfoVersion = 0;
  SmallPtrSet<DIBasicType *, 12> BasicTypes;
  SmallPtrSet<DIDerivedType *, 12> PointerDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> QualifiedDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> TypedefTypes;
  SmallPtrSet<DIDerivedType *, 12> InheritedTypes;
  SmallVector<const DIImportedEntity *, 5> ImportedEntities;
  SmallPtrSet<DICompositeType *, 12> ArrayTypes;
  SmallString<128> BuildIdentifier;
  SmallString<128> BuildStoragePath;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypesWithTemplates;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypes;
  Register DebugSourceResIdReg;
  Register DebugCompUnitResIdReg;

  {
    const MachineModuleInfo &MMI =
        getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
    const Module *M = MMI.getModule();
    Context = &M->getContext();
    const NamedMDNode *DbgCu = M->getNamedMetadata("llvm.dbg.cu");
    if (!DbgCu)
      return false;
    for (const auto *Op : DbgCu->operands()) {
      if (const auto *CompileUnit = dyn_cast<DICompileUnit>(Op)) {
        if (CompileUnit->getDWOId())
          BuildIdentifier = std::to_string(CompileUnit->getDWOId());
        if (!CompileUnit->getSplitDebugFilename().empty())
          BuildStoragePath = CompileUnit->getSplitDebugFilename();

        for (auto *GVE : CompileUnit->getGlobalVariables()) {
          if (auto *DIGV = dyn_cast<DIGlobalVariable>(GVE->getVariable())) {
            extractTypeMetadata(DIGV->getType(), BasicTypes,
                                PointerDerivedTypes, QualifiedDerivedTypes,
                                TypedefTypes, ArrayTypes, CompositeTypes,
                                InheritedTypes);
          }
        }
        for (const auto *IE : CompileUnit->getImportedEntities()) {
          if (const auto *Imported = dyn_cast<DIImportedEntity>(IE)) {
            ImportedEntities.push_back(Imported);
          }
        }

        DIFile *File = CompileUnit->getFile();
        FilePaths.emplace_back();
        sys::path::append(FilePaths.back(), File->getDirectory(),
                          File->getFilename());
        LLVMSourceLanguages.push_back(CompileUnit->getSourceLanguage());
      }
    }
    const NamedMDNode *ModuleFlags = M->getNamedMetadata("llvm.module.flags");
    assert(ModuleFlags && "Expected llvm.module.flags metadata to be present");
    for (const auto *Op : ModuleFlags->operands()) {
      const MDOperand &MaybeStrOp = Op->getOperand(1);
      if (MaybeStrOp.equalsStr("Dwarf Version"))
        DwarfVersion =
            cast<ConstantInt>(
                cast<ConstantAsMetadata>(Op->getOperand(2))->getValue())
                ->getSExtValue();
      else if (MaybeStrOp.equalsStr("Debug Info Version"))
        DebugInfoVersion =
            cast<ConstantInt>(
                cast<ConstantAsMetadata>(Op->getOperand(2))->getValue())
                ->getSExtValue();
    }

    for (auto &F : *M) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          for (DbgVariableRecord &DVR : filterDbgVars(I.getDbgRecordRange())) {
            DILocalVariable *LocalVariable = DVR.getVariable();
            extractTypeMetadata(LocalVariable->getType(), BasicTypes,
                                PointerDerivedTypes, QualifiedDerivedTypes,
                                TypedefTypes, ArrayTypes, CompositeTypes,
                                InheritedTypes);

            //       if (auto *ArrayCT =
            //               dyn_cast<DICompositeType>(LocalVariable->getType()))
            //               {

            //         if (ArrayCT->getTag() == dwarf::DW_TAG_structure_type ||
            //             ArrayCT->getTag() == dwarf::DW_TAG_class_type ||
            //             ArrayCT->getTag() == dwarf::DW_TAG_union_type ||
            //             ArrayCT->getTag() == dwarf::DW_TAG_enumeration_type)
            //             {
            //           llvm::errs()
            //               << "Extracting composite type: " <<
            //               ArrayCT->getName() << "\n";
            //           CompositeTypes.insert(ArrayCT);
            //             for (Metadata *Element : ArrayCT->getElements()) {
            //               if (auto *Member =
            //               dyn_cast<DIDerivedType>(Element)) {
            //                 extractTypeMetadata(Member, BasicTypes,
            //                                     PointerDerivedTypes,
            //                                     QualifiedDerivedTypes,
            //                                     TypedefTypes, ArrayTypes,
            //                                     CompositeTypes,
            //                                     InheritedTypes);
            //               }
            //             }
            //         }
            //         if (ArrayCT->getTag() == dwarf::DW_TAG_array_type) {
            //           ArrayTypes.insert(ArrayCT); // Already handled
            //         }
            //       }
            //       if (auto *BasicType =
            //               dyn_cast<DIBasicType>(LocalVariable->getType())) {
            //         BasicTypes.insert(BasicType);
            //       } else if (auto *DerivedType =
            //                      dyn_cast<DIDerivedType>(LocalVariable->getType()))
            //                      {
            //         DILocalVariable *LocalVariable = DVR.getVariable();
            //         llvm::errs() << "Extracting derived type: "
            //                      << LocalVariable->getName() << "\n";
            //         extractTypeMetadata(LocalVariable->getType(), BasicTypes,
            //                             PointerDerivedTypes,
            //                             QualifiedDerivedTypes, TypedefTypes,
            //                             ArrayTypes, CompositeTypes,
            //                             InheritedTypes);
            //         if (DerivedType->getTag() == dwarf::DW_TAG_pointer_type)
            //         {
            //           PointerDerivedTypes.insert(DerivedType);
            //           // DIBasicType can be unreachable from DbgRecord and
            //           only
            //           // pointed on from other DI types
            //           // DerivedType->getBaseType is null when pointer
            //           // is representing a void type
            //           if (auto *BT = dyn_cast_or_null<DIBasicType>(
            //                   DerivedType->getBaseType()))
            //             BasicTypes.insert(BT);
            //         }
            //       }
          }
        }
      }
    }
  }
  // NonSemantic.Shader.DebugInfo.100 global DI instruction emitting
  {
    // Required LLVM variables for emitting logic
    const SPIRVInstrInfo *TII = TM->getSubtargetImpl()->getInstrInfo();
    const SPIRVRegisterInfo *TRI = TM->getSubtargetImpl()->getRegisterInfo();
    const RegisterBankInfo *RBI = TM->getSubtargetImpl()->getRegBankInfo();
    SPIRVGlobalRegistry *GR = TM->getSubtargetImpl()->getSPIRVGlobalRegistry();
    MachineRegisterInfo &MRI = MF.getRegInfo();
    MachineBasicBlock &MBB = *MF.begin();

    // To correct placement of a OpLabel instruction during SPIRVAsmPrinter
    // emission all new instructions needs to be placed after OpFunction
    // and before first terminator
    MachineIRBuilder MIRBuilder(MBB, MBB.getFirstTerminator());

    const SPIRVType *VoidTy =
        GR->getOrCreateSPIRVType(Type::getVoidTy(*Context), MIRBuilder,
                                 SPIRV::AccessQualifier::ReadWrite, false);

    const SPIRVType *I32Ty =
        GR->getOrCreateSPIRVType(Type::getInt32Ty(*Context), MIRBuilder,
                                 SPIRV::AccessQualifier::ReadWrite, false);

    const Register DwarfVersionReg =
        GR->buildConstantInt(DwarfVersion, MIRBuilder, I32Ty, false);

    const Register DebugInfoVersionReg =
        GR->buildConstantInt(DebugInfoVersion, MIRBuilder, I32Ty, false);

    const Register I32ZeroReg =
        GR->buildConstantInt(0, MIRBuilder, I32Ty, false, false);

    SPIRVCodeGenContext Ctx(MIRBuilder, MRI, GR, VoidTy, I32Ty, TII, TRI, RBI,
                            MF, I32ZeroReg, TM);

    const DICompileUnit *CU = MF.getFunction().getSubprogram()->getUnit();
    emitDebugLineInstructions(Ctx, DebugSourceResIdReg);
    emitDebugMacroDefs(CU, Ctx);

    for (unsigned Idx = 0; Idx < LLVMSourceLanguages.size(); ++Idx) {
      emitSingleCompilationUnit(FilePaths[Idx], LLVMSourceLanguages[Idx], Ctx,
                                DebugInfoVersionReg, DwarfVersionReg);
    }
    emitDebugBuildIdentifier(BuildIdentifier, Ctx);
    emitDebugStoragePath(BuildStoragePath, Ctx);

    // First: Collect composite types from retained types
    for (const auto &Node : CU->getRetainedTypes()) {
      if (const auto *CT = dyn_cast<DICompositeType>(Node)) {
        if (CT->getTag() == dwarf::DW_TAG_structure_type ||
            CT->getTag() == dwarf::DW_TAG_class_type ||
            CT->getTag() == dwarf::DW_TAG_union_type) {
          CompositeTypes.insert(CT);
        }
      }
    }

    // Next: Also collect composite types from global variables (important!)
    for (DIGlobalVariableExpression *GVE : CU->getGlobalVariables()) {
      if (const auto *GV = GVE->getVariable()) {
        DIType *Ty = GV->getType();

        extractTypeMetadata(Ty, BasicTypes, PointerDerivedTypes,
                            QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                            CompositeTypes, InheritedTypes);

        if (auto *CT = dyn_cast<DICompositeType>(Ty)) {
          if (!CT->getTemplateParams().empty())
            CompositeTypesWithTemplates.insert(CT);
        }
      }
    }

    for (auto *GVE : CU->getGlobalVariables()) {
      emitDebugGlobalVariable(GVE, Ctx, DebugSourceResIdReg,
                              DebugCompUnitResIdReg);
    }

    // We aren't extracting any DebugInfoFlags now so we
    // emitting zero to use as <id>Flags argument for DebugBasicType

    // We need to store pairs because further instructions reference
    // the DIBasicTypes and size will be always small so there isn't
    // need for any kind of map
    SmallVector<std::pair<const DIBasicType *const, const Register>, 12>
        BasicTypeRegPairs;
    emitDebugBasicTypes(BasicTypes, BasicTypeRegPairs, Ctx);

    // Emit all composite types now
    for (auto *CT : CompositeTypes) {

      extractTypeMetadata(const_cast<DICompositeType *>(CT), BasicTypes,
                          PointerDerivedTypes, QualifiedDerivedTypes,
                          TypedefTypes, ArrayTypes, CompositeTypes,
                          InheritedTypes);
      emitDebugTypeComposite(CT, Ctx, DebugSourceResIdReg,
                             DebugCompUnitResIdReg, BasicTypeRegPairs);
    }

    emitDebugQualifiedTypes(QualifiedDerivedTypes, BasicTypeRegPairs, Ctx);
    emitDebugTypedefs(TypedefTypes, BasicTypeRegPairs, Ctx,
                      DebugSourceResIdReg);
    emitDebugImportedEntities(ImportedEntities, Ctx);
    emitDebugArrayTypes(ArrayTypes, BasicTypeRegPairs, Ctx);

    for (const DICompositeType *CompTy : CompositeTypesWithTemplates) {
      // Ensure we extract all types used in template parameters
      for (const auto *MD : CompTy->getTemplateParams()) {
        if (const auto *TTP = dyn_cast<DITemplateTypeParameter>(MD)) {
          extractTypeMetadata(TTP->getType(), BasicTypes, PointerDerivedTypes,
                              QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                              CompositeTypes, InheritedTypes);
        } else if (const auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
          extractTypeMetadata(TVP->getType(), BasicTypes, PointerDerivedTypes,
                              QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                              CompositeTypes, InheritedTypes);
        }
      }

      llvm::errs() << "Emitting template instructions for: "
                   << CompTy->getName() << "\n";

      emitTemplateDebugInstructions(CompTy, Ctx, BasicTypeRegPairs,
                                    DebugSourceResIdReg);
    }
    emitDebugTypeInheritance(InheritedTypes, Ctx);

    emitDebugPointerTypes(PointerDerivedTypes, BasicTypeRegPairs, Ctx);
  }
  return true;
}

bool SPIRVEmitNonSemanticDI::runOnMachineFunction(MachineFunction &MF) {
  bool Res = false;
  // emitGlobalDI needs to be executed only once to avoid
  // emitting duplicates
  if (!IsGlobalDIEmitted) {
    IsGlobalDIEmitted = true;
    Res = emitGlobalDI(MF);
  }
  return Res;
}

uint32_t SPIRVEmitNonSemanticDI::mapDwarfTagToTypeQualifier(unsigned Tag) {
  switch (Tag) {
  case dwarf::DW_TAG_const_type:
    return 0; // ConstType
  case dwarf::DW_TAG_volatile_type:
    return 1; // VolatileType
  case dwarf::DW_TAG_restrict_type:
    return 2; // RestrictType
  case dwarf::DW_TAG_atomic_type:
    return 3; // AtomicType
  default:
    llvm_unreachable("Unknown DWARF tag for DebugTypeQualifier");
  }
}

void SPIRVEmitNonSemanticDI::extractTypeMetadata(
    DIType *Ty, SmallPtrSetImpl<DIBasicType *> &BasicTypes,
    SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
    SmallPtrSetImpl<DIDerivedType *> &QualifiedDerivedTypes,
    SmallPtrSetImpl<DIDerivedType *> &TypedefTypes,
    SmallPtrSetImpl<DICompositeType *> &ArrayTypes,
    SmallPtrSetImpl<const DICompositeType *> &CompositeTypes,
    SmallPtrSetImpl<DIDerivedType *> &InheritedTypes) {
  if (!Ty)
    return;
  if (auto *CT = dyn_cast<DICompositeType>(Ty)) {
    if (CT->getTag() == dwarf::DW_TAG_array_type)
      ArrayTypes.insert(CT);
    else if (CT->getTag() == dwarf::DW_TAG_structure_type ||
             CT->getTag() == dwarf::DW_TAG_class_type ||
             CT->getTag() == dwarf::DW_TAG_union_type ||
             CT->getTag() == dwarf::DW_TAG_enumeration_type) {
      llvm::errs() << "Extracting composite type: " << CT->getName() << "\n";
      CompositeTypes.insert(CT);
      if (const auto *CT = dyn_cast<DICompositeType>(Ty)) {
        for (Metadata *Element : CT->getElements()) {
          if (auto *Member = dyn_cast<DIDerivedType>(Element)) {
            extractTypeMetadata(Member->getBaseType(), BasicTypes,
                                PointerDerivedTypes, QualifiedDerivedTypes,
                                TypedefTypes, ArrayTypes, CompositeTypes,
                                InheritedTypes);
            extractTypeMetadata(Member, BasicTypes, PointerDerivedTypes,
                                QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                                CompositeTypes, InheritedTypes);
          }
        }
      }
    }

    for (Metadata *El : CT->getElements()) {
      if (auto *SR = dyn_cast_or_null<DISubrange>(El)) {
        auto CountValUnion = SR->getCount();

        // Check if the count is a DIVariable
        if (auto *CountVar = CountValUnion.dyn_cast<DIVariable *>()) {
          extractTypeMetadata(CountVar->getType(), BasicTypes,
                              PointerDerivedTypes, QualifiedDerivedTypes,
                              TypedefTypes, ArrayTypes, CompositeTypes,
                              InheritedTypes);
        }

        // Optionally handle ConstantInt* too, if you want to extract integer
        // type
        if (auto *CountCI = CountValUnion.dyn_cast<ConstantInt *>()) {
          // Could insert the int type manually
          if (auto *IntTy = dyn_cast_or_null<DIBasicType>(CT->getBaseType()))
            BasicTypes.insert(IntTy);
        }
      }
    }

    extractTypeMetadata(CT->getBaseType(), BasicTypes, PointerDerivedTypes,
                        QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                        CompositeTypes, InheritedTypes);
  }

  if (auto *BT = dyn_cast<DIBasicType>(Ty)) {
    BasicTypes.insert(BT);
  } else if (auto *DT = dyn_cast<DIDerivedType>(Ty)) {
    switch (DT->getTag()) {
    case dwarf::DW_TAG_pointer_type:
      PointerDerivedTypes.insert(DT);
      break;
    case dwarf::DW_TAG_const_type:
    case dwarf::DW_TAG_volatile_type:
    case dwarf::DW_TAG_restrict_type:
    case dwarf::DW_TAG_atomic_type:
      QualifiedDerivedTypes.insert(DT);
      break;
    case dwarf::DW_TAG_typedef:
      TypedefTypes.insert(DT);
    case dwarf::DW_TAG_member:
      BasicTypes.insert(dyn_cast<DIBasicType>(DT->getBaseType()));
      llvm::errs() << "Extracting member type: " << DT->getName() << "\n";
      break;
    case dwarf::DW_TAG_inheritance:
      llvm::errs() << "Extracting inherited type: " << DT->getName() << "\n";
      InheritedTypes.insert(DT);
      break;
    }
    extractTypeMetadata(DT->getBaseType(), BasicTypes, PointerDerivedTypes,
                        QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                        CompositeTypes, InheritedTypes);
  }
}

Register SPIRVEmitNonSemanticDI::findEmittedBasicTypeReg(
    const DIType *BaseType,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {
  const DIType *Ty = BaseType;

  while (Ty && !isa<DIBasicType>(Ty)) {
    if (auto *Derived = dyn_cast<DIDerivedType>(Ty))
      Ty = Derived->getBaseType();
    else
      return Register();
  }

  if (const auto *BT = dyn_cast<DIBasicType>(Ty)) {
    StringRef Name = BT->getName();
    uint64_t Size = BT->getSizeInBits();

    for (const auto &[DefinedBT, Reg] : BasicTypeRegPairs) {
      if (DefinedBT->getName() == Name && DefinedBT->getSizeInBits() == Size)
        return Reg;
    }
  }

  return Register();
}

void SPIRVEmitNonSemanticDI::emitDebugBuildIdentifier(
    StringRef BuildIdentifier, SPIRVCodeGenContext &Ctx) {
  if (!BuildIdentifier.empty()) {
    const Register BuildIdStrReg = EmitOpString(BuildIdentifier, Ctx);
    const Register FlagsReg =
        Ctx.GR->buildConstantInt(1, Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register DebugBuildIdReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugBuildIdentifier,
                          {BuildIdStrReg, FlagsReg}, Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugStoragePath(StringRef BuildStoragePath,
                                                  SPIRVCodeGenContext &Ctx) {
  if (!BuildStoragePath.empty()) {
    const Register PathStrReg = EmitOpString(BuildStoragePath, Ctx);
    const Register DebugStoragePathReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugStoragePath, {PathStrReg}, Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugBasicTypes(
    const SmallPtrSetImpl<DIBasicType *> &BasicTypes,
    SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SPIRVCodeGenContext &Ctx) {
  // We need to store pairs because further instructions reference
  // the DIBasicTypes and size will be always small so there isn't
  // need for any kind of map
  for (auto *BasicType : BasicTypes) {
    if (!BasicType)
      continue;
    llvm::errs() << "Collected BasicType: " << BasicType->getName() << "\n";
    const Register BasicTypeStrReg = EmitOpString(BasicType->getName(), Ctx);

    const Register ConstIntBitwidthReg = Ctx.GR->buildConstantInt(
        BasicType->getSizeInBits(), Ctx.MIRBuilder, Ctx.I32Ty, false);

    uint64_t AttributeEncoding = BaseTypeAttributeEncoding::Unspecified;
    switch (BasicType->getEncoding()) {
    case dwarf::DW_ATE_signed:
      AttributeEncoding = BaseTypeAttributeEncoding::Signed;
      break;
    case dwarf::DW_ATE_unsigned:
      AttributeEncoding = BaseTypeAttributeEncoding::Unsigned;
      break;
    case dwarf::DW_ATE_unsigned_char:
      AttributeEncoding = BaseTypeAttributeEncoding::UnsignedChar;
      break;
    case dwarf::DW_ATE_signed_char:
      AttributeEncoding = BaseTypeAttributeEncoding::SignedChar;
      break;
    case dwarf::DW_ATE_float:
      AttributeEncoding = BaseTypeAttributeEncoding::Float;
      break;
    case dwarf::DW_ATE_boolean:
      AttributeEncoding = BaseTypeAttributeEncoding::Boolean;
      break;
    case dwarf::DW_ATE_address:
      AttributeEncoding = BaseTypeAttributeEncoding::Address;
    }

    const Register AttributeEncodingReg = Ctx.GR->buildConstantInt(
        AttributeEncoding, Ctx.MIRBuilder, Ctx.I32Ty, false);

    const Register BasicTypeReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeBasic,
                          {BasicTypeStrReg, ConstIntBitwidthReg,
                           AttributeEncodingReg, Ctx.I32ZeroReg},
                          Ctx);
    Ctx.GR->addDebugValue(BasicType, BasicTypeReg);
    BasicTypeRegPairs.emplace_back(BasicType, BasicTypeReg);
    llvm::errs() << "Emitted BasicType: " << BasicType->getName()
                 << " with Reg: " << BasicTypeReg << "\n";
  }
}

void SPIRVEmitNonSemanticDI::emitDebugQualifiedTypes(
    const SmallPtrSetImpl<DIDerivedType *> &QualifiedDerivedTypes,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SPIRVCodeGenContext &Ctx) {
  if (!QualifiedDerivedTypes.empty()) {
    for (const auto *QualifiedDT : QualifiedDerivedTypes) {
      Register BaseTypeReg = findEmittedBasicTypeReg(QualifiedDT->getBaseType(),
                                                     BasicTypeRegPairs);
      if (!BaseTypeReg)
        continue;

      const uint32_t QualifierValue =
          mapDwarfTagToTypeQualifier(QualifiedDT->getTag());
      const Register QualifierConstReg = Ctx.GR->buildConstantInt(
          QualifierValue, Ctx.MIRBuilder, Ctx.I32Ty, false);

      [[maybe_unused]]
      const Register DebugQualifiedTypeReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeQualifier,
                            {BaseTypeReg, QualifierConstReg}, Ctx);
    }
  }
}

void SPIRVEmitNonSemanticDI::emitDebugTypedefs(
    const SmallPtrSetImpl<DIDerivedType *> &TypedefTypes,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg) {
  for (const auto *TypedefDT : TypedefTypes) {
    Register BaseTypeReg =
        findEmittedBasicTypeReg(TypedefDT->getBaseType(), BasicTypeRegPairs);
    if (!BaseTypeReg)
      continue;

    // Emit name string for typedef
    const Register TypedefNameReg = EmitOpString(TypedefDT->getName(), Ctx);

    // Emit source file string
    DIFile *File = TypedefDT->getFile();
    const Register FilePathStrReg =
        EmitOpString((File ? File->getFilename() : "<unknown>"), Ctx);

    // Emit DebugSource
    const Register DebugSourceReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, Ctx);

    // Emit line and column constants
    const Register LineReg = Ctx.GR->buildConstantInt(
        TypedefDT->getLine(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register ColumnReg = Ctx.GR->buildConstantInt(
        /* fallback */ 0, Ctx.MIRBuilder, Ctx.I32Ty, false);

    // Emit scope (for now: just use DebugInfoNone)
    const Register ScopeReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

    [[maybe_unused]]
    const Register DebugTypedefReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypedef,
                          {TypedefNameReg, BaseTypeReg, DebugSourceReg, LineReg,
                           ColumnReg, ScopeReg},
                          Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugImportedEntities(
    const SmallVectorImpl<const DIImportedEntity *> &ImportedEntities,
    SPIRVCodeGenContext &Ctx) {
  for (const auto *Imported : ImportedEntities) {
    // Skip if no name or entity
    if (!Imported->getEntity())
      continue;

    const Register NameStrReg = EmitOpString(Imported->getName(), Ctx);

    // Tag (DW_TAG_imported_module or DW_TAG_imported_declaration, etc.)
    const Register TagReg = Ctx.GR->buildConstantInt(
        Imported->getTag(), Ctx.MIRBuilder, Ctx.I32Ty, false);

    // Source file
    const Register FilePathStrReg = EmitOpString(
        Imported->getFile() ? Imported->getFile()->getFilename() : "<unknown>",
        Ctx);
    const Register DebugSourceReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, Ctx);

    // Entity being imported – this could be a namespace or declaration
    // For now: emit DebugInfoNone as a placeholder
    const Register EntityReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

    // Line and column
    const Register LineReg = Ctx.GR->buildConstantInt(
        Imported->getLine(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register ColumnReg = Ctx.GR->buildConstantInt(
        0, Ctx.MIRBuilder, Ctx.I32Ty, false); // Column info often unavailable

    // Parent scope – again use DebugInfoNone for now
    const Register ScopeReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

    // Emit the actual DebugImportedEntity instruction
    [[maybe_unused]]
    const Register DebugImportedEntityReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugImportedEntity,
                          {NameStrReg, TagReg, DebugSourceReg, EntityReg,
                           LineReg, ColumnReg, ScopeReg},
                          Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugArrayTypes(
    const SmallPtrSetImpl<DICompositeType *> &ArrayTypes,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SPIRVCodeGenContext &Ctx) {
  for (auto *ArrayTy : ArrayTypes) {
    DIType *ElementType = ArrayTy->getBaseType();
    Register BaseTypeReg =
        findEmittedBasicTypeReg(ElementType, BasicTypeRegPairs);
    if (!BaseTypeReg)
      continue;
    DINodeArray Subranges = ArrayTy->getElements();
    if (ArrayTy->isVector()) {
      assert(Subranges.size() == 1 && "Only 1D vectors supported!");
      emitDebugVectorTypes(ArrayTy, BaseTypeReg, Ctx);
      continue;
    }

    SmallVector<Register, 4> ComponentCountRegs;
    for (Metadata *M : Subranges) {
      if (auto *SR = dyn_cast<DISubrange>(M)) {
        auto CountValUnion = SR->getCount();
        if (auto *CountCI = CountValUnion.dyn_cast<ConstantInt *>()) {
          uint64_t CountVal = CountCI->getZExtValue();
          Register ConstCountReg = Ctx.GR->buildConstantInt(
              CountVal, Ctx.MIRBuilder, Ctx.I32Ty, false);
          ComponentCountRegs.push_back(ConstCountReg);
        } else {
          // Runtime-sized or unknown count — emit 0 constant
          Register ConstZero =
              Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
          ComponentCountRegs.push_back(ConstZero);
        }
      }
    }

    SmallVector<Register, 6> Ops;
    Ops.push_back(BaseTypeReg);
    llvm::append_range(Ops, ComponentCountRegs);

    [[maybe_unused]]
    Register DebugArrayTypeReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeArray, Ops, Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugVectorTypes(DICompositeType *ArrayTy,
                                                  Register BaseTypeReg,
                                                  SPIRVCodeGenContext &Ctx) {
  DINodeArray Subranges = ArrayTy->getElements();

  Register ComponentCountReg;
  if (auto *SR = dyn_cast<DISubrange>(Subranges[0])) {
    auto CountValUnion = SR->getCount();
    if (auto *CountCI = CountValUnion.dyn_cast<ConstantInt *>()) {
      uint64_t CountVal = CountCI->getZExtValue();
      ComponentCountReg =
          Ctx.GR->buildConstantInt(CountVal, Ctx.MIRBuilder, Ctx.I32Ty, false);
    } else {
      ComponentCountReg = Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty,
                                                   false); // fallback
    }
  }

  SmallVector<Register, 4> Ops;
  Ops.push_back(BaseTypeReg);
  Ops.push_back(ComponentCountReg);

  [[maybe_unused]]
  Register DebugVectorTypeReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeVector, Ops, Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugPointerTypes(
    const SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SPIRVCodeGenContext &Ctx) {
  if (PointerDerivedTypes.size()) {
    for (const auto *PointerDerivedType : PointerDerivedTypes) {

      assert(PointerDerivedType->getDWARFAddressSpace().has_value());
      const Register StorageClassReg = Ctx.GR->buildConstantInt(
          addressSpaceToStorageClass(
              PointerDerivedType->getDWARFAddressSpace().value(),
              *Ctx.TM->getSubtargetImpl()),
          Ctx.MIRBuilder, Ctx.I32Ty, false);

      // If the Pointer is representing a void type it's getBaseType
      // is a nullptr
      const auto *MaybeNestedBasicType =
          dyn_cast_or_null<DIBasicType>(PointerDerivedType->getBaseType());
      if (MaybeNestedBasicType) {
        for (const auto &BasicTypeRegPair : BasicTypeRegPairs) {
          const auto &[DefinedBasicType, BasicTypeReg] = BasicTypeRegPair;
          if (DefinedBasicType == MaybeNestedBasicType) {
            [[maybe_unused]]
            const Register DebugPointerTypeReg = EmitDIInstruction(
                SPIRV::NonSemanticExtInst::DebugTypePointer,
                {BasicTypeReg, StorageClassReg, Ctx.I32ZeroReg}, Ctx);
          }
        }
      } else {
        const Register DebugInfoNoneReg = EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
        [[maybe_unused]]
        const Register DebugPointerTypeReg = EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugTypePointer,
            {DebugInfoNoneReg, StorageClassReg, Ctx.I32ZeroReg}, Ctx);
      }
    }
  }
}

void SPIRVEmitNonSemanticDI::emitDebugMacroDefs(const DICompileUnit *CU,
                                                SPIRVCodeGenContext &Ctx) {

  DenseMap<StringRef, Register> MacroDefRegs;
  if (!CU || !CU->getMacros())
    return;
  const StringRef FileName =
      CU->getFile() ? CU->getFile()->getFilename() : "<unknown>";

  std::function<void(const MDNode *)> WalkMacroTree;
  WalkMacroTree = [&](const MDNode *Node) {
    if (const auto *Macro = dyn_cast<DIMacro>(Node)) {
      if (Macro->getMacinfoType() == dwarf::DW_MACINFO_define) {
        // Optionally skip macros without line numbers
        if (Macro->getLine() == 0)
          return;

        const StringRef Name = Macro->getName();
        const StringRef Value = Macro->getValue();
        const unsigned Line = Macro->getLine();

        const Register SourceStrReg = EmitOpString(FileName, Ctx);
        const Register LineConstReg =
            Ctx.GR->buildConstantInt(Line, Ctx.MIRBuilder, Ctx.I32Ty, false);
        const Register NameStrReg = EmitOpString(Name, Ctx);
        const Register ValueStrReg = EmitOpString(Value, Ctx);

        [[maybe_unused]] const Register DebugMacroDefReg = EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugMacroDef,
            {SourceStrReg, LineConstReg, NameStrReg, ValueStrReg}, Ctx);
        MacroDefRegs[Macro->getName()] = DebugMacroDefReg;
      } else if (Macro->getMacinfoType() == dwarf::DW_MACINFO_undef) {
        emitDebugMacroUndef(Macro, FileName, Ctx, MacroDefRegs);
      }
    } else if (const auto *MacroFile = dyn_cast<DIMacroFile>(Node)) {
      for (const auto &Child : MacroFile->getElements())
        WalkMacroTree(Child);
    }
  };

  for (const auto &MacroNode : CU->getMacros()->operands()) {
    if (const auto *MD = dyn_cast<MDNode>(MacroNode.get()))
      WalkMacroTree(MD);
  }
}
void SPIRVEmitNonSemanticDI::emitDebugMacroUndef(
    const DIMacro *MacroUndef, StringRef FileName, SPIRVCodeGenContext &Ctx,
    const DenseMap<StringRef, Register> &MacroDefRegs) {

  const StringRef Name = MacroUndef->getName();
  const unsigned Line = MacroUndef->getLine();

  // We need the macro def for this name
  auto It = MacroDefRegs.find(Name);
  if (It == MacroDefRegs.end())
    return; // No previous DebugMacroDef emitted — skip

  Register MacroDefReg = It->second;

  Register SourceStrReg = EmitOpString(FileName, Ctx);
  Register LineConstReg =
      Ctx.GR->buildConstantInt(Line, Ctx.MIRBuilder, Ctx.I32Ty, false);

  [[maybe_unused]] Register MacroUndefReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugMacroUndef,
                        {SourceStrReg, LineConstReg, MacroDefReg}, Ctx);
}

Register SPIRVEmitNonSemanticDI::EmitOpString(StringRef SR,
                                              SPIRVCodeGenContext &Ctx) {
  const Register StrReg = Ctx.MRI.createVirtualRegister(&SPIRV::IDRegClass);
  Ctx.MRI.setType(StrReg, LLT::scalar(32));
  MachineInstrBuilder MIB = Ctx.MIRBuilder.buildInstr(SPIRV::OpString);
  MIB.addDef(StrReg);
  addStringImm(SR, MIB);
  return StrReg;
}
Register SPIRVEmitNonSemanticDI::EmitDIInstruction(
    SPIRV::NonSemanticExtInst::NonSemanticExtInst Inst,
    ArrayRef<Register> Operands, SPIRVCodeGenContext &Ctx) {
  const Register InstReg = Ctx.MRI.createVirtualRegister(&SPIRV::IDRegClass);
  Ctx.MRI.setType(InstReg, LLT::scalar(32));
  MachineInstrBuilder MIB =
      Ctx.MIRBuilder.buildInstr(SPIRV::OpExtInst)
          .addDef(InstReg)
          .addUse(Ctx.GR->getSPIRVTypeID(Ctx.VoidTy))
          .addImm(static_cast<int64_t>(
              SPIRV::InstructionSet::NonSemantic_Shader_DebugInfo_100))
          .addImm(Inst);
  for (auto Reg : Operands)
    MIB.addUse(Reg);
  MIB.constrainAllUses(*Ctx.TII, *Ctx.TRI, *Ctx.RBI);
  Ctx.GR->assignSPIRVTypeToVReg(Ctx.VoidTy, InstReg, Ctx.MF);
  return InstReg;
}
void SPIRVEmitNonSemanticDI::emitTemplateDebugInstructions(
    const DICompositeType *CompTy, SPIRVCodeGenContext &Ctx,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    const Register DebugSourceResIdReg) {

  const DINodeArray TemplateParams = CompTy->getTemplateParams();
  if (TemplateParams.empty())
    return;

  Register LineReg = Ctx.GR->buildConstantInt(CompTy->getLine(), Ctx.MIRBuilder,
                                              Ctx.I32Ty, false);
  Register ColumnReg = Ctx.GR->buildConstantInt(
      0, Ctx.MIRBuilder, Ctx.I32Ty, false); // No column info in DICompositeType

  SmallVector<Register, 4> ParamRegs;

  for (const auto *MD : TemplateParams) {
    if (auto *TTP = dyn_cast<DITemplateTypeParameter>(MD)) {
      Register NameStr = EmitOpString(TTP->getName(), Ctx);
      Register TypeReg =
          findEmittedBasicTypeReg(TTP->getType(), BasicTypeRegPairs);

      if (!TypeReg) {
        llvm::errs() << "Cannot emit DebugTypeTemplateParameter: type not "
                        "found for param "
                     << TTP->getName() << "\n";
        continue;
      }

      // Use DebugInfoNone for type parameters
      Register NoneReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

      ParamRegs.push_back(EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
          {NameStr, TypeReg, NoneReg, DebugSourceResIdReg, LineReg, ColumnReg},
          Ctx));

    } else if (auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
      Register NameStr = EmitOpString(TVP->getName(), Ctx);
      Register TypeReg =
          findEmittedBasicTypeReg(TVP->getType(), BasicTypeRegPairs);
      if (!TypeReg)
        continue;

      // TODO: Extract actual constant value. For now fallback to 0.
      Register ValueReg =
          Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);

      ParamRegs.push_back(EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
          {NameStr, TypeReg, ValueReg, DebugSourceResIdReg, LineReg, ColumnReg},
          Ctx));
    }
  }

  Register CompositeReg = Ctx.GR->getDebugValue(CompTy);
  if (!CompositeReg.isValid()) {
    llvm::errs() << "Missing DebugTypeComposite for templated type: "
                 << CompTy->getName() << "\n";
    return;
  }

  // Insert CompositeReg as first operand
  ParamRegs.insert(ParamRegs.begin(), CompositeReg);

  EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeTemplate, ParamRegs,
                    Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugTypeComposite(
    const DICompositeType *CompTy, SPIRVCodeGenContext &Ctx,
    const Register &SourceReg, const Register &CUReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  if (!CompTy)
    return;

  unsigned TagValue = CompTy->getTag();

  if (TagValue == dwarf::DW_TAG_enumeration_type) {
    emitDebugTypeEnum(CompTy, Ctx, SourceReg, CUReg, BasicTypeRegPairs);
    return;
  }

  Register NameStr = EmitOpString(CompTy->getName(), Ctx);
  Register LinkageNameStr = EmitOpString(CompTy->getIdentifier(), Ctx);
  Register Tag = Ctx.GR->buildConstantInt(CompTy->getTag(), Ctx.MIRBuilder,
                                          Ctx.I32Ty, false);
  Register Line = Ctx.GR->buildConstantInt(CompTy->getLine(), Ctx.MIRBuilder,
                                           Ctx.I32Ty, false);
  Register Column = Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty,
                                             false); // Not available
  Register SizeReg = Ctx.GR->buildConstantInt(CompTy->getSizeInBits(),
                                              Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register FlagsReg = Ctx.GR->buildConstantInt(
      CompTy->getFlags(), Ctx.MIRBuilder, Ctx.I32Ty, false);

  Register Res = Ctx.MRI.createVirtualRegister(&SPIRV::IDRegClass);
  Ctx.MRI.setType(Res, LLT::scalar(32));

  SmallVector<Register, 4> MemberRegs;

  for (Metadata *El : CompTy->getElements()) {
    if (auto *DTM = dyn_cast<DIDerivedType>(El)) {
      emitDebugTypeMember(DTM, Ctx, Res, SourceReg, BasicTypeRegPairs,
                          MemberRegs);
    }
  }

  SmallVector<Register, 12> Ops = {NameStr,        Tag,     SourceReg,
                                   Line,           Column,  CUReg,
                                   LinkageNameStr, SizeReg, FlagsReg};
  Ops.append(MemberRegs);

  Res = EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeComposite, Ops,
                          Ctx);

  Ctx.GR->addDebugValue(CompTy, Res);
}

void SPIRVEmitNonSemanticDI::emitDebugTypeMember(
    const DIDerivedType *Member, SPIRVCodeGenContext &Ctx,
    const Register &CompositeReg, const Register &SourceReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SmallVectorImpl<Register> &MemberRegs) {

  if (!Member || Member->getTag() != dwarf::DW_TAG_member)
    return;

  Register NameStr = EmitOpString(Member->getName(), Ctx);

  // Try to resolve the base type's SPIR-V register
  Register TypeReg =
      findEmittedBasicTypeReg(Member->getBaseType(), BasicTypeRegPairs);

  if (!TypeReg.isValid()) {
    llvm::errs() << "Warning: Failed to emit DebugTypeMember for "
                 << Member->getName() << ", base type not emitted\n";

    if (const auto *BT = dyn_cast<DIBasicType>(Member->getBaseType()))
      llvm::errs() << "BaseType is basic: " << BT->getName() << "\n";
    else if (const auto *DT = dyn_cast<DIDerivedType>(Member->getBaseType()))
      llvm::errs() << "BaseType is derived: " << DT->getName() << "\n";
  }

  Register LineReg = Ctx.GR->buildConstantInt(Member->getLine(), Ctx.MIRBuilder,
                                              Ctx.I32Ty, false);
  Register ColumnReg =
      Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register OffsetReg = Ctx.GR->buildConstantInt(
      Member->getOffsetInBits(), Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register SizeReg = Ctx.GR->buildConstantInt(Member->getSizeInBits(),
                                              Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register FlagsReg = Ctx.GR->buildConstantInt(
      Member->getFlags(), Ctx.MIRBuilder, Ctx.I32Ty, false);

  SmallVector<Register, 10> Ops = {NameStr,   TypeReg,   SourceReg, LineReg,
                                   ColumnReg, OffsetReg, SizeReg,   FlagsReg};

  Register MemberReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeMember, Ops, Ctx);

  MemberRegs.push_back(MemberReg);
}
void SPIRVEmitNonSemanticDI::emitDebugTypeEnum(
    const DICompositeType *EnumTy, SPIRVCodeGenContext &Ctx,
    const Register &SourceReg, const Register &CUReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  if (Register Existing = Ctx.GR->getDebugValue(EnumTy); Existing.isValid())
    return;

  Register NameStr = EmitOpString(EnumTy->getName(), Ctx);
  Register TypeReg = findEmittedBasicTypeReg(
      EnumTy->getBaseType(),
      BasicTypeRegPairs); // You may already have this helper
  Register Line = Ctx.GR->buildConstantInt(EnumTy->getLine(), Ctx.MIRBuilder,
                                           Ctx.I32Ty, false);
  Register Column =
      Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register Size = Ctx.GR->buildConstantInt(EnumTy->getSizeInBits(),
                                           Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register Flags = Ctx.GR->buildConstantInt(EnumTy->getFlags(), Ctx.MIRBuilder,
                                            Ctx.I32Ty, false);

  SmallVector<Register, 16> EnumOperands;
  for (Metadata *MD : EnumTy->getElements()) {
    if (auto *E = dyn_cast<DIEnumerator>(MD)) {
      int64_t ValRaw = E->getValue().getSExtValue();
      llvm::errs() << "Emitting enumerator: " << E->getName()
                   << " with value: " << ValRaw << "\n";
      Register Val =
          Ctx.GR->buildConstantInt(E->getValue().getZExtValue(), Ctx.MIRBuilder,
                                   Ctx.I32Ty, /*IsSigned*/ false);

      // Register Val = GR->buildConstantInt(ValRaw, MIRBuilder, Ctx.I32Ty,
      // true);

      Register Name = EmitOpString(E->getName(), Ctx);
      EnumOperands.push_back(Val);
      EnumOperands.push_back(Name);
    }
  }
  SmallVector<Register, 12> Ops = {NameStr, TypeReg, SourceReg, Line,
                                   Column,  CUReg,   Size,      Flags};
  Ops.append(EnumOperands);

  Register Res =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeEnum, Ops, Ctx);
  Ctx.GR->addDebugValue(EnumTy, Res);
}
void SPIRVEmitNonSemanticDI::emitSingleCompilationUnit(
    StringRef FilePath, int64_t Language, SPIRVCodeGenContext &Ctx,
    Register DebugInfoVersionReg, Register DwarfVersionReg) {
  const Register FilePathStrReg = EmitOpString(FilePath, Ctx);

  Register DebugSourceResIdReg = EmitDIInstruction(
      SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, Ctx);

  SourceLanguage SpirvSourceLanguage = SourceLanguage::Unknown;
  switch (Language) {
  case dwarf::DW_LANG_OpenCL:
    SpirvSourceLanguage = SourceLanguage::OpenCL_C;
    break;
  case dwarf::DW_LANG_OpenCL_CPP:
    SpirvSourceLanguage = SourceLanguage::OpenCL_CPP;
    break;
  case dwarf::DW_LANG_CPP_for_OpenCL:
    SpirvSourceLanguage = SourceLanguage::CPP_for_OpenCL;
    break;
  case dwarf::DW_LANG_GLSL:
    SpirvSourceLanguage = SourceLanguage::GLSL;
    break;
  case dwarf::DW_LANG_HLSL:
    SpirvSourceLanguage = SourceLanguage::HLSL;
    break;
  case dwarf::DW_LANG_SYCL:
    SpirvSourceLanguage = SourceLanguage::SYCL;
    break;
  case dwarf::DW_LANG_Zig:
    SpirvSourceLanguage = SourceLanguage::Zig;
  }

  const Register SourceLanguageReg = Ctx.GR->buildConstantInt(
      SpirvSourceLanguage, Ctx.MIRBuilder, Ctx.I32Ty, false);

  [[maybe_unused]] Register DebugCompUnitResIdReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugCompilationUnit,
                        {DebugInfoVersionReg, DwarfVersionReg,
                         DebugSourceResIdReg, SourceLanguageReg},
                        Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugLinePerInstruction(
    MachineInstr &MI, SPIRVCodeGenContext &Ctx, Register DebugSourceResIdReg) {
  DebugLoc DL = MI.getDebugLoc();
  if (!DL) {
    // Optionally emit DebugNoLine extinst.
    // DebugNoLine has no operands in the NonSemantic extset.
    // Insert point: before MI
    Ctx.MIRBuilder.setInsertPt(*MI.getParent(), MI.getIterator());
    EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugNoLine,
                      ArrayRef<Register>{}, Ctx);
    return;
  } // No debug info → nothing to emit

  const DILocation *DIL = DL.get();
  if (!DIL)
    return; // unexpected, but safe-guard

  const DIFile *File = DIL->getFile();
  if (!File)
    return;

  // Build constants for line/column
  Register LineReg = Ctx.GR->buildConstantInt(DIL->getLine(), Ctx.MIRBuilder,
                                              Ctx.I32Ty, false);
  Register ColReg = Ctx.GR->buildConstantInt(DIL->getColumn(), Ctx.MIRBuilder,
                                             Ctx.I32Ty, false);

  // Insert BEFORE the MI
  Ctx.MIRBuilder.setInsertPt(*MI.getParent(), MI.getIterator());

  // Compose operands as the NonSemantic DebugLine expects:
  SmallVector<Register, 5> Ops;
  Ops.push_back(DebugSourceResIdReg);
  Ops.push_back(LineReg); // LineStart
  Ops.push_back(LineReg); // LineEnd (same for single-instr)
  Ops.push_back(ColReg);  // ColumnStart
  Ops.push_back(ColReg);

  EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugLine, Ops, Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugLineInstructions(
    SPIRVCodeGenContext &Ctx, Register DebugSourceResIdReg) {
  // New logic using a per-instruction approach
  for (auto &MBB : Ctx.MF) {
    for (auto &MI : MBB) {
      emitDebugLinePerInstruction(MI, Ctx, DebugSourceResIdReg);
    }
  }
}
Register SPIRVEmitNonSemanticDI::emitDebugGlobalVariable(
    const DIGlobalVariableExpression *GVE, SPIRVCodeGenContext &Ctx,
    const Register &DebugSourceResIdReg,
    const Register &DebugCompUnitResIdReg) {

  const DIGlobalVariable *DIGV = GVE->getVariable();
  StringRef Name = DIGV->getName();
  StringRef LinkageName = DIGV->getLinkageName();
  DIType *Type = DIGV->getType();
  unsigned Line = DIGV->getLine();
  unsigned Column = 1;
  const DIScope *ParentScope = DIGV->getScope();
  uint32_t Flags = 1;

  GlobalVariable *MatchedGV = nullptr;
  for (GlobalVariable &G : Ctx.MF.getFunction().getParent()->globals()) {
    SmallVector<DIGlobalVariableExpression *, 4> Exprs;
    G.getDebugInfo(Exprs);
    for (DIGlobalVariableExpression *CurrentGVE : Exprs) {
      if (CurrentGVE == GVE) {
        MatchedGV = &G;
        break;
      }
    }
    if (MatchedGV)
      break;
  }
  Register NameStrReg = EmitOpString(Name, Ctx);
  Register LinkageStrReg = EmitOpString(LinkageName, Ctx);
  Register LineReg =
      Ctx.GR->buildConstantInt(Line, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register ColumnReg =
      Ctx.GR->buildConstantInt(Column, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register FlagsReg =
      Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register TypeReg = Ctx.GR->getDebugValue(Type);
  Register ParentReg;
  if (ParentScope) {
    ParentReg = Ctx.GR->getDebugValue(ParentScope);
  } else {
    ParentReg = DebugCompUnitResIdReg;
  }

  Register VariableReg;
  // if (MatchedGV) {
  //  VariableReg =
  //  Ctx.GR->getSPIRVTypeID(Ctx.GR->getOrCreateSPIRVType(MatchedGV->getType(),
  //  Ctx.MIRBuilder));
  // } else {
  VariableReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
  // }

  SmallVector<Register, 9> Ops = {
      NameStrReg, TypeReg,       DebugSourceResIdReg, LineReg, ColumnReg,
      ParentReg,  LinkageStrReg, VariableReg,         FlagsReg};

  return EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugGlobalVariable, Ops,
                           Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugTypeInheritance(
    const SmallPtrSetImpl<DIDerivedType *> &InheritedTypes,
    SPIRVCodeGenContext &Ctx) {

  if (!InheritedTypes.empty()) {
    for (const auto *Inh : InheritedTypes) {
      assert(Inh && Inh->getTag() == dwarf::DW_TAG_inheritance &&
             "emitDebugTypeInheritance expects DW_TAG_inheritance");

      // ---- Child (Set) is the composite this inheritance belongs to (scope)
      const auto *Scope = dyn_cast<DICompositeType>(Inh->getScope());
      Register SetReg = Scope ? Ctx.GR->getDebugValue(Scope) : Register();
      if (!SetReg.isValid()) {
        // Fallback: some producers might omit scope; bail out if we can't
        // recover.
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
      }

      // ---- Parent (base class/struct)
      const DIType *BaseTy = Inh->getBaseType();
      if (!BaseTy) {
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
      }
      Register ParentReg = Ctx.GR->getDebugValue(BaseTy);
      if (!ParentReg.isValid()) {
        // Force materialize a type if it hasn't been emitted yet.
        ParentReg = Ctx.GR->getDebugValue(BaseTy);
        if (!ParentReg.isValid())
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
      }

      // ---- Offset in bits
      // Prefer the canonical API; some toolchains also stash it in extraData.
      uint64_t OffsetBits = Inh->getOffsetInBits();
      if (!OffsetBits) {
        if (const Metadata *ED = Inh->getExtraData()) {
          if (const auto *CM = dyn_cast<ConstantAsMetadata>(ED)) {
            if (const auto *CI = dyn_cast<ConstantInt>(CM->getValue()))
              OffsetBits = CI->getZExtValue();
          }
        }
      }
      Register OffsetReg = Ctx.GR->buildConstantInt(OffsetBits, Ctx.MIRBuilder,
                                                    Ctx.I32Ty, false);

      // ---- Size in bits of the base within the derived layout
      uint64_t SizeBits = 0;
      if (const auto *BT = dyn_cast<DIType>(BaseTy))
        SizeBits = BT->getSizeInBits();
      Register SizeReg =
          Ctx.GR->buildConstantInt(SizeBits, Ctx.MIRBuilder, Ctx.I32Ty, false);
      const uint32_t Flags = 1;
      Register FlagsReg =
          Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

      // ---- Build NonSemantic.DebugTypeInheritance:
      // Operands: Set (child), Parent, Offset, Size, Flags
      SmallVector<Register, 5> Ops = {SetReg, ParentReg, OffsetReg, SizeReg,
                                      FlagsReg};

      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeInheritance, Ops,
                        Ctx);
    }
  }
}
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
#include "llvm/IR/LegacyPassManager.h"
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
struct DebugInfoCollector {
  SmallPtrSet<DIBasicType *, 12> BasicTypes;
  SmallPtrSet<DIDerivedType *, 12> PointerDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> QualifiedDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> TypedefTypes;
  SmallPtrSet<DIDerivedType *, 12> InheritedTypes;
  SmallPtrSet<DIDerivedType *, 12> PtrToMemberTypes;
  SmallVector<const DIImportedEntity *, 5> ImportedEntities;
  SmallPtrSet<DICompositeType *, 12> ArrayTypes;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypesWithTemplates;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypes;
  SmallPtrSet<const DICompositeType *, 8> EnumTypes;
  DenseSet<const DIType *> visitedTypes;
};
struct SPIRVEmitNonSemanticDI : public llvm::ModulePass {
  static char ID;
  SPIRVTargetMachine *TM;
  SPIRVEmitNonSemanticDI(SPIRVTargetMachine *TM = nullptr)
      : ModulePass(ID), TM(TM) {}

  bool runOnModule(llvm::Module &M) override;

private:
  bool IsGlobalDIEmitted = false;
  bool emitGlobalDI(MachineFunction &MF);
  uint32_t mapDwarfTagToTypeComposite(const DICompositeType *CT);
  uint32_t mapDwarfTagToTypeQualifier(unsigned Tag);
  Register EmitOpString(StringRef, SPIRVCodeGenContext &Ctx);
  uint32_t transDebugFlags(const DINode *DN);
  uint32_t mapDebugFlags(DINode::DIFlags DFlags);

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

  Register emitDebugGlobalVariable(
      const DIGlobalVariableExpression *GVE, SPIRVCodeGenContext &Ctx,
      const Register &DebugSourceResIdReg,
      const Register &DebugCompUnitResIdReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitAllDebugGlobalVariables(
      const llvm::DIGlobalVariableExpressionArray &GlobalVars,
      SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
      const Register &DebugCompUnitResIdReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

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

  Register findEmittedBasicTypeReg(
      const DIType *BaseType,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void extractTypeMetadata(DIType *Ty, DebugInfoCollector &Collector);

  void emitAllTemplateDebugInstructions(
      const SmallPtrSetImpl<const DICompositeType *> &TemplatedTypes,
      SPIRVCodeGenContext &Ctx,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      const Register DebugSourceResIdReg);

  void emitAllDebugTypeComposites(
      const SmallPtrSetImpl<const DICompositeType *> &CompositeTypes,
      SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
      const Register &DebugCompUnitResIdReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitAllDebugTypeEnum(
      const SmallPtrSetImpl<const DICompositeType *> &EnumTypes,
      SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
      const Register &DebugCompUnitResIdReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

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
                                 Register DwarfVersionReg,
                                 Register &DebugSourceResIdReg,
                                 Register &DebugCompUnitResIdReg);

  void emitDebugTypePtrToMember(
      const SmallPtrSetImpl<DIDerivedType *> &PtrToMemberTypes,
      SPIRVCodeGenContext &Ctx);

  void getAnalysisUsage(AnalysisUsage &AU) const;
};
} // namespace

INITIALIZE_PASS(SPIRVEmitNonSemanticDI, DEBUG_TYPE,
                "SPIRV NonSemantic.Shader.DebugInfo.100 emitter", false, false)

char SPIRVEmitNonSemanticDI::ID = 0;

llvm::ModulePass *
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
enum CompositeTypeAttributeEncoding { Class = 0, Struct = 1, Union = 2 };

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

enum Flag {
  FlagIsProtected = 1 << 0,
  FlagIsPrivate = 1 << 1,
  FlagIsPublic = FlagIsPrivate | FlagIsProtected,
  FlagAccess = FlagIsPublic,
  FlagIsLocal = 1 << 2,
  FlagIsDefinition = 1 << 3,
  FlagIsFwdDecl = 1 << 4,
  FlagIsArtificial = 1 << 5,
  FlagIsExplicit = 1 << 6,
  FlagIsPrototyped = 1 << 7,
  FlagIsObjectPointer = 1 << 8,
  FlagIsStaticMember = 1 << 9,
  FlagIsIndirectVariable = 1 << 10,
  FlagIsLValueReference = 1 << 11,
  FlagIsRValueReference = 1 << 12,
  FlagIsOptimized = 1 << 13,
  FlagIsEnumClass = 1 << 14,
  FlagTypePassByValue = 1 << 15,
  FlagTypePassByReference = 1 << 16,
  FlagUnknownPhysicalLayout = 1 << 17,
  FlagBitField = 1 << 18
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
  SmallString<128> BuildIdentifier;
  SmallString<128> BuildStoragePath;
  Register DebugSourceResIdReg;
  Register DebugCompUnitResIdReg;
  DebugInfoCollector Collector;

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
            extractTypeMetadata(DIGV->getType(), Collector);
          }
        }
        for (const auto *IE : CompileUnit->getImportedEntities()) {
          if (const auto *Imported = dyn_cast<DIImportedEntity>(IE)) {
            Collector.ImportedEntities.push_back(Imported);
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
            if (DILocalVariable *LocalVariable = DVR.getVariable())
              extractTypeMetadata(LocalVariable->getType(), Collector);

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

    SmallVector<std::pair<const DIBasicType *const, const Register>, 12>
        BasicTypeRegPairs;

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

    for (unsigned Idx = 0; Idx < LLVMSourceLanguages.size(); ++Idx) {
      emitSingleCompilationUnit(FilePaths[Idx], LLVMSourceLanguages[Idx], Ctx,
                                DebugInfoVersionReg, DwarfVersionReg,
                                DebugSourceResIdReg, DebugCompUnitResIdReg);
    }

    emitDebugLineInstructions(Ctx, DebugSourceResIdReg);
    emitDebugMacroDefs(CU, Ctx);
    emitDebugBuildIdentifier(BuildIdentifier, Ctx);
    emitDebugStoragePath(BuildStoragePath, Ctx);

    for (const auto &Node : CU->getRetainedTypes()) {
      extractTypeMetadata(dyn_cast<DICompositeType>(Node), Collector);
    }

    for (DIGlobalVariableExpression *GVE : CU->getGlobalVariables()) {
      if (const auto *GV = GVE->getVariable()) {
        extractTypeMetadata(GV->getType(), Collector);
      }
    }

    emitDebugBasicTypes(Collector.BasicTypes, BasicTypeRegPairs, Ctx);
    emitAllDebugGlobalVariables(CU->getGlobalVariables(), Ctx,
                                DebugSourceResIdReg, DebugCompUnitResIdReg,
                                BasicTypeRegPairs);

    emitAllDebugTypeComposites(Collector.CompositeTypes, Ctx,
                               DebugSourceResIdReg, DebugCompUnitResIdReg,
                               BasicTypeRegPairs);
    emitAllDebugTypeEnum(Collector.EnumTypes, Ctx, DebugSourceResIdReg,
                         DebugCompUnitResIdReg, BasicTypeRegPairs);

    emitDebugQualifiedTypes(Collector.QualifiedDerivedTypes, BasicTypeRegPairs,
                            Ctx);
    emitDebugTypedefs(Collector.TypedefTypes, BasicTypeRegPairs, Ctx,
                      DebugSourceResIdReg);
    emitDebugImportedEntities(Collector.ImportedEntities, Ctx);
    emitDebugArrayTypes(Collector.ArrayTypes, BasicTypeRegPairs, Ctx);
    emitAllTemplateDebugInstructions(Collector.CompositeTypesWithTemplates, Ctx,
                                     BasicTypeRegPairs, DebugSourceResIdReg);
    emitDebugTypeInheritance(Collector.InheritedTypes, Ctx);
    emitDebugPointerTypes(Collector.PointerDerivedTypes, BasicTypeRegPairs,
                          Ctx);
    emitDebugTypePtrToMember(Collector.PtrToMemberTypes, Ctx);
  }
  return true;
}

// bool SPIRVEmitNonSemanticDI::runOnMachineFunction(MachineFunction &MF) {
//   bool Res = false;
//   // emitGlobalDI needs to be executed only once to avoid
//   // emitting duplicates
//   if (!IsGlobalDIEmitted) {
//     IsGlobalDIEmitted = true;
//     Res = emitGlobalDI(MF);
//   }
//   return Res;
// }

uint32_t SPIRVEmitNonSemanticDI::mapDwarfTagToTypeQualifier(unsigned Tag) {
  switch (Tag) {
  case dwarf::DW_TAG_const_type:
    return 0;
  case dwarf::DW_TAG_volatile_type:
    return 1;
  case dwarf::DW_TAG_restrict_type:
    return 2;
  case dwarf::DW_TAG_atomic_type:
    return 3;
  default:
    llvm_unreachable("Unknown DWARF tag for DebugTypeQualifier");
  }
}

void SPIRVEmitNonSemanticDI::extractTypeMetadata(
    DIType *Ty, DebugInfoCollector &Collector) {

  if (!Ty)
    return;

  if (!Collector.visitedTypes.insert(Ty).second)
    return;

  if (auto *CT = dyn_cast<DICompositeType>(Ty)) {
    if (!CT->getTemplateParams().empty()) {
      Collector.CompositeTypesWithTemplates.insert(CT);
      for (const auto *MD : CT->getTemplateParams()) {
        if (const auto *TTP = dyn_cast<DITemplateTypeParameter>(MD)) {
          extractTypeMetadata(TTP->getType(), Collector);
        } else if (const auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
          extractTypeMetadata(TVP->getType(), Collector);
        }
      }
    }

    if (CT->getTag() == dwarf::DW_TAG_array_type) {
      Collector.ArrayTypes.insert(CT);
    } else if (CT->getTag() == dwarf::DW_TAG_structure_type ||
               CT->getTag() == dwarf::DW_TAG_class_type ||
               CT->getTag() == dwarf::DW_TAG_union_type) {
      Collector.CompositeTypes.insert(CT);
    } else if (CT->getTag() == dwarf::DW_TAG_enumeration_type) {
      Collector.EnumTypes.insert(CT);
    }

    for (Metadata *Element : CT->getElements()) {
      if (auto *Member = dyn_cast<DIDerivedType>(Element)) {
        extractTypeMetadata(Member->getBaseType(), Collector);
      } else if (auto *SR = dyn_cast<DISubrange>(Element)) {
        if (auto *CountVar = SR->getCount().dyn_cast<DIVariable *>()) {
          extractTypeMetadata(CountVar->getType(), Collector);
        }
      }
    }

    // Process the base type of the composite.
    extractTypeMetadata(CT->getBaseType(), Collector);
    return;
  }

  if (auto *BT = dyn_cast<DIBasicType>(Ty)) {
    Collector.BasicTypes.insert(BT);
    return;
  }

  if (auto *DT = dyn_cast<DIDerivedType>(Ty)) {
    switch (DT->getTag()) {
    case dwarf::DW_TAG_pointer_type:
      Collector.PointerDerivedTypes.insert(DT);
      break;
    case dwarf::DW_TAG_const_type:
    case dwarf::DW_TAG_volatile_type:
    case dwarf::DW_TAG_restrict_type:
    case dwarf::DW_TAG_atomic_type:
      Collector.QualifiedDerivedTypes.insert(DT);
      break;
    case dwarf::DW_TAG_typedef:
      Collector.TypedefTypes.insert(DT);
      break;
    case dwarf::DW_TAG_inheritance:
      Collector.InheritedTypes.insert(DT);
      break;
    case dwarf::DW_TAG_ptr_to_member_type:
      Collector.PtrToMemberTypes.insert(DT);
      break;
    case dwarf::DW_TAG_member:
      break;
    default:
      break;
    }
    extractTypeMetadata(DT->getBaseType(), Collector);
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
    EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugBuildIdentifier,
                      {BuildIdStrReg, FlagsReg}, Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugStoragePath(StringRef BuildStoragePath,
                                                  SPIRVCodeGenContext &Ctx) {
  if (!BuildStoragePath.empty()) {
    const Register PathStrReg = EmitOpString(BuildStoragePath, Ctx);
    EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugStoragePath, {PathStrReg},
                      Ctx);
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

    const Register TypedefNameReg = EmitOpString(TypedefDT->getName(), Ctx);
    DIFile *File = TypedefDT->getFile();
    const Register FilePathStrReg =
        EmitOpString((File ? File->getFilename() : "<unknown>"), Ctx);
    const Register DebugSourceReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, Ctx);
    const Register LineReg = Ctx.GR->buildConstantInt(
        TypedefDT->getLine(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register ColumnReg =
        Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
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
    if (!Imported->getEntity())
      continue;

    const Register NameStrReg = EmitOpString(Imported->getName(), Ctx);
    const Register TagReg = Ctx.GR->buildConstantInt(
        Imported->getTag(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register FilePathStrReg = EmitOpString(
        Imported->getFile() ? Imported->getFile()->getFilename() : "<unknown>",
        Ctx);
    const Register DebugSourceReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, Ctx);
    const Register EntityReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
    const Register LineReg = Ctx.GR->buildConstantInt(
        Imported->getLine(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register ColumnReg =
        Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
    const Register ScopeReg =
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

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
      const uint32_t Flags = transDebugFlags(PointerDerivedType);
      Register FlagsReg =
          Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

      // If the Pointer is representing a void type it's getBaseType
      // is a nullptr
      const auto *MaybeNestedBasicType =
          dyn_cast_or_null<DIBasicType>(PointerDerivedType->getBaseType());
      if (MaybeNestedBasicType) {
        for (const auto &BasicTypeRegPair : BasicTypeRegPairs) {
          const auto &[DefinedBasicType, BasicTypeReg] = BasicTypeRegPair;
          if (DefinedBasicType == MaybeNestedBasicType) {
            [[maybe_unused]]
            const Register DebugPointerTypeReg =
                EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypePointer,
                                  {BasicTypeReg, StorageClassReg, FlagsReg},
                                  Ctx);
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
  auto It = MacroDefRegs.find(Name);
  if (It == MacroDefRegs.end())
    return;

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

void SPIRVEmitNonSemanticDI::emitAllTemplateDebugInstructions(
    const SmallPtrSetImpl<const DICompositeType *> &TemplatedTypes,
    SPIRVCodeGenContext &Ctx,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    const Register DebugSourceResIdReg) {

  // The loop that was previously at the call site is now inside this function.
  for (const DICompositeType *CompTy : TemplatedTypes) {
    const DINodeArray TemplateParams = CompTy->getTemplateParams();
    // If a type has no template params, skip to the next one in the collection.
    if (TemplateParams.empty())
      continue;

    Register LineReg = Ctx.GR->buildConstantInt(
        CompTy->getLine(), Ctx.MIRBuilder, Ctx.I32Ty, false);
    Register ColumnReg =
        Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);

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
        Register NoneReg = EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);

        ParamRegs.push_back(EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
            {NameStr, TypeReg, NoneReg, DebugSourceResIdReg, LineReg,
             ColumnReg},
            Ctx));

      } else if (auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
        Register NameStr = EmitOpString(TVP->getName(), Ctx);
        Register TypeReg =
            findEmittedBasicTypeReg(TVP->getType(), BasicTypeRegPairs);
        if (!TypeReg)
          continue;

        int64_t ActualValue = 0;
        if (auto *CAM = dyn_cast_or_null<ConstantAsMetadata>(TVP->getValue())) {
          if (auto *CI = dyn_cast<ConstantInt>(CAM->getValue())) {
            ActualValue = CI->getSExtValue();
          }
        }
        Register ValueReg = Ctx.GR->buildConstantInt(
            ActualValue, Ctx.MIRBuilder, Ctx.I32Ty, false);

        ParamRegs.push_back(EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
            {NameStr, TypeReg, ValueReg, DebugSourceResIdReg, LineReg,
             ColumnReg},
            Ctx));
      }
    }

    Register CompositeReg = Ctx.GR->getDebugValue(CompTy);
    if (!CompositeReg.isValid()) {
      llvm::errs() << "Missing DebugTypeComposite for templated type: "
                   << CompTy->getName() << "\n";
      continue;
    }

    ParamRegs.insert(ParamRegs.begin(), CompositeReg);

    EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeTemplate, ParamRegs,
                      Ctx);
  }
}

void SPIRVEmitNonSemanticDI::emitAllDebugTypeComposites(
    const SmallPtrSetImpl<const DICompositeType *> &CompositeTypes,
    SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
    const Register &DebugCompUnitResIdReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  for (auto *CT : CompositeTypes) {
    emitDebugTypeComposite(CT, Ctx, DebugSourceResIdReg, DebugCompUnitResIdReg,
                           BasicTypeRegPairs);
  }
}

void SPIRVEmitNonSemanticDI::emitAllDebugTypeEnum(
    const SmallPtrSetImpl<const DICompositeType *> &EnumTypes,
    SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
    const Register &DebugCompUnitResIdReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  for (auto *CT : EnumTypes) {
    emitDebugTypeEnum(CT, Ctx, DebugSourceResIdReg, DebugCompUnitResIdReg,
                      BasicTypeRegPairs);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugTypeComposite(
    const DICompositeType *CompTy, SPIRVCodeGenContext &Ctx,
    const Register &SourceReg, const Register &CUReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  if (!CompTy)
    return;

  Register NameStr = EmitOpString(CompTy->getName(), Ctx);
  Register LinkageNameStr = EmitOpString(CompTy->getIdentifier(), Ctx);
  uint32_t Tag = mapDwarfTagToTypeComposite(CompTy);
  Register Tags =
      Ctx.GR->buildConstantInt(Tag, Ctx.MIRBuilder, Ctx.I32Ty, false);
  llvm::errs() << "Emitting composite type: " << CompTy->getName()
               << " with tag " << CompTy->getTag() << "\n";
  Register Line = Ctx.GR->buildConstantInt(CompTy->getLine(), Ctx.MIRBuilder,
                                           Ctx.I32Ty, false);
  Register Column =
      Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register SizeReg = Ctx.GR->buildConstantInt(CompTy->getSizeInBits(),
                                              Ctx.MIRBuilder, Ctx.I32Ty, false);
  uint32_t Flags = transDebugFlags(CompTy);
  Register FlagsReg =
      Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

  Register Res = Ctx.MRI.createVirtualRegister(&SPIRV::IDRegClass);
  Ctx.MRI.setType(Res, LLT::scalar(32));

  SmallVector<Register, 4> MemberRegs;

  for (Metadata *El : CompTy->getElements()) {
    if (auto *DTM = dyn_cast<DIDerivedType>(El)) {
      emitDebugTypeMember(DTM, Ctx, Res, SourceReg, BasicTypeRegPairs,
                          MemberRegs);
    }
  }

  SmallVector<Register, 12> Ops = {NameStr,        Tags,    SourceReg,
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
  Register TypeReg =
      findEmittedBasicTypeReg(Member->getBaseType(), BasicTypeRegPairs);

  if (!TypeReg.isValid()) {
    llvm::errs() << "Warning: Failed to emit DebugTypeMember for "
                 << Member->getName() << ", base type not emitted\n";
  }

  Register LineReg = Ctx.GR->buildConstantInt(Member->getLine(), Ctx.MIRBuilder,
                                              Ctx.I32Ty, false);
  Register ColumnReg =
      Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register OffsetReg = Ctx.GR->buildConstantInt(
      Member->getOffsetInBits(), Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register SizeReg = Ctx.GR->buildConstantInt(Member->getSizeInBits(),
                                              Ctx.MIRBuilder, Ctx.I32Ty, false);
  uint32_t Flags = transDebugFlags(Member);
  Register FlagsReg =
      Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

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
  Register TypeReg =
      findEmittedBasicTypeReg(EnumTy->getBaseType(), BasicTypeRegPairs);
  Register Line = Ctx.GR->buildConstantInt(EnumTy->getLine(), Ctx.MIRBuilder,
                                           Ctx.I32Ty, false);
  Register Column =
      Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false);
  Register Size = Ctx.GR->buildConstantInt(EnumTy->getSizeInBits(),
                                           Ctx.MIRBuilder, Ctx.I32Ty, false);
  uint32_t Flags = transDebugFlags(EnumTy);
  Register FlagsReg =
      Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

  SmallVector<Register, 16> EnumOperands;
  for (Metadata *MD : EnumTy->getElements()) {
    if (auto *E = dyn_cast<DIEnumerator>(MD)) {
      int64_t ValRaw = E->getValue().getSExtValue();
      llvm::errs() << "Emitting enumerator: " << E->getName()
                   << " with value: " << ValRaw << "\n";
      Register Val = Ctx.GR->buildConstantInt(E->getValue().getZExtValue(),
                                              Ctx.MIRBuilder, Ctx.I32Ty, false);

      // Register Val = GR->buildConstantInt(ValRaw, MIRBuilder, Ctx.I32Ty,
      // true);

      Register Name = EmitOpString(E->getName(), Ctx);
      EnumOperands.push_back(Val);
      EnumOperands.push_back(Name);
    }
  }
  SmallVector<Register, 12> Ops = {NameStr, TypeReg, SourceReg, Line,
                                   Column,  CUReg,   Size,      FlagsReg};
  Ops.append(EnumOperands);

  Register Res =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeEnum, Ops, Ctx);
  Ctx.GR->addDebugValue(EnumTy, Res);
}
void SPIRVEmitNonSemanticDI::emitSingleCompilationUnit(
    StringRef FilePath, int64_t Language, SPIRVCodeGenContext &Ctx,
    Register DebugInfoVersionReg, Register DwarfVersionReg,
    Register &DebugSourceResIdReg, Register &DebugCompUnitResIdReg) {
  const Register FilePathStrReg = EmitOpString(FilePath, Ctx);

  DebugSourceResIdReg = EmitDIInstruction(
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

  DebugCompUnitResIdReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugCompilationUnit,
                        {DebugInfoVersionReg, DwarfVersionReg,
                         DebugSourceResIdReg, SourceLanguageReg},
                        Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugLinePerInstruction(
    MachineInstr &MI, SPIRVCodeGenContext &Ctx, Register DebugSourceResIdReg) {
  DebugLoc DL = MI.getDebugLoc();
  if (!DL) {
    Ctx.MIRBuilder.setInsertPt(*MI.getParent(), MI.getIterator());
    EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugNoLine,
                      ArrayRef<Register>{}, Ctx);
    return;
  }

  const DILocation *DIL = DL.get();
  if (!DIL)
    return;

  const DIFile *File = DIL->getFile();
  if (!File)
    return;

  Register LineReg = Ctx.GR->buildConstantInt(DIL->getLine(), Ctx.MIRBuilder,
                                              Ctx.I32Ty, false);
  Register ColReg = Ctx.GR->buildConstantInt(DIL->getColumn(), Ctx.MIRBuilder,
                                             Ctx.I32Ty, false);

  Ctx.MIRBuilder.setInsertPt(*MI.getParent(), MI.getIterator());

  SmallVector<Register, 5> Ops;
  Ops.push_back(DebugSourceResIdReg);
  Ops.push_back(LineReg);
  Ops.push_back(LineReg);
  Ops.push_back(ColReg);
  Ops.push_back(ColReg);

  EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugLine, Ops, Ctx);
}

void SPIRVEmitNonSemanticDI::emitDebugLineInstructions(
    SPIRVCodeGenContext &Ctx, Register DebugSourceResIdReg) {
  for (auto &MBB : Ctx.MF) {
    for (auto &MI : MBB) {
      emitDebugLinePerInstruction(MI, Ctx, DebugSourceResIdReg);
    }
  }
}
void SPIRVEmitNonSemanticDI::emitAllDebugGlobalVariables(
    const llvm::DIGlobalVariableExpressionArray &GlobalVars,
    SPIRVCodeGenContext &Ctx, const Register &DebugSourceResIdReg,
    const Register &DebugCompUnitResIdReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  for (auto *GVE : GlobalVars) {
    if (GVE) {
      emitDebugGlobalVariable(GVE, Ctx, DebugSourceResIdReg,
                              DebugCompUnitResIdReg, BasicTypeRegPairs);
    }
  }
}

Register SPIRVEmitNonSemanticDI::emitDebugGlobalVariable(
    const DIGlobalVariableExpression *GVE, SPIRVCodeGenContext &Ctx,
    const Register &DebugSourceResIdReg, const Register &DebugCompUnitResIdReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  const DIGlobalVariable *DIGV = GVE->getVariable();
  StringRef Name = DIGV->getName();
  StringRef LinkageName = DIGV->getLinkageName();
  unsigned Line = DIGV->getLine();
  unsigned Column = 1;
  const DIScope *ParentScope = DIGV->getScope();
  uint32_t Flags = transDebugFlags(DIGV);

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
  Register TypeReg =
      findEmittedBasicTypeReg(DIGV->getType(), BasicTypeRegPairs);
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
void SPIRVEmitNonSemanticDI::emitDebugTypePtrToMember(
    const SmallPtrSetImpl<DIDerivedType *> &PtrToMemberTypes,
    SPIRVCodeGenContext &Ctx) {
  if (!PtrToMemberTypes.empty()) {
    for (const auto *PtrToMemberType : PtrToMemberTypes) {
      assert(PtrToMemberType->getTag() == dwarf::DW_TAG_ptr_to_member_type &&
             "emitDebugTypePtrToMember expects DW_TAG_ptr_to_member_type");
      Register MemberTypeReg =
          Ctx.GR->getDebugValue(PtrToMemberType->getBaseType());
      Register ParentReg =
          Ctx.GR->getDebugValue(PtrToMemberType->getClassType());

      SmallVector<Register, 3> Ops;
      Ops.push_back(MemberTypeReg);
      Ops.push_back(ParentReg);

      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypePtrToMember, Ops,
                        Ctx);
    }
  }
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

      const DIType *BaseTy = Inh->getBaseType();
      if (!BaseTy) {
        EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
      }
      Register ParentReg = Ctx.GR->getDebugValue(BaseTy);
      if (!ParentReg.isValid()) {
        ParentReg = Ctx.GR->getDebugValue(BaseTy);
        if (!ParentReg.isValid())
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {}, Ctx);
      }

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

      uint64_t SizeBits = 0;
      if (const auto *BT = dyn_cast<DIType>(BaseTy))
        SizeBits = BT->getSizeInBits();
      Register SizeReg =
          Ctx.GR->buildConstantInt(SizeBits, Ctx.MIRBuilder, Ctx.I32Ty, false);
      uint32_t Flags = transDebugFlags(Inh);
      Register FlagsReg =
          Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder, Ctx.I32Ty, false);

      SmallVector<Register, 5> Ops = {SetReg, ParentReg, OffsetReg, SizeReg,
                                      FlagsReg};

      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeInheritance, Ops,
                        Ctx);
    }
  }
}
uint32_t SPIRVEmitNonSemanticDI::transDebugFlags(const DINode *DN) {
  uint32_t Flags = 0;
  if (const DIGlobalVariable *GV = dyn_cast<DIGlobalVariable>(DN)) {
    if (GV->isLocalToUnit())
      Flags |= Flag::FlagIsLocal;
    if (GV->isDefinition())
      Flags |= Flag::FlagIsDefinition;
  }
  if (const DISubprogram *DS = dyn_cast<DISubprogram>(DN)) {
    if (DS->isLocalToUnit())
      Flags |= Flag::FlagIsLocal;
    if (DS->isOptimized())
      Flags |= Flag::FlagIsOptimized;
    if (DS->isDefinition())
      Flags |= Flag::FlagIsDefinition;
    Flags |= mapDebugFlags(DS->getFlags());
  }
  if (DN->getTag() == dwarf::DW_TAG_reference_type)
    Flags |= Flag::FlagIsLValueReference;
  if (DN->getTag() == dwarf::DW_TAG_rvalue_reference_type)
    Flags |= Flag::FlagIsRValueReference;
  if (const DIType *DT = dyn_cast<DIType>(DN))
    Flags |= mapDebugFlags(DT->getFlags());
  if (const DILocalVariable *DLocVar = dyn_cast<DILocalVariable>(DN))
    Flags |= mapDebugFlags(DLocVar->getFlags());

  return Flags;
}

uint32_t SPIRVEmitNonSemanticDI::mapDebugFlags(DINode::DIFlags DFlags) {
  uint32_t Flags = 0;
  if ((DFlags & DINode::FlagAccessibility) == DINode::FlagPublic)
    Flags |= Flag::FlagIsPublic;
  if ((DFlags & DINode::FlagAccessibility) == DINode::FlagProtected)
    Flags |= Flag::FlagIsProtected;
  if ((DFlags & DINode::FlagAccessibility) == DINode::FlagPrivate)
    Flags |= Flag::FlagIsPrivate;

  if (DFlags & DINode::FlagFwdDecl)
    Flags |= Flag::FlagIsFwdDecl;
  if (DFlags & DINode::FlagArtificial)
    Flags |= Flag::FlagIsArtificial;
  if (DFlags & DINode::FlagExplicit)
    Flags |= Flag::FlagIsExplicit;
  if (DFlags & DINode::FlagPrototyped)
    Flags |= Flag::FlagIsPrototyped;
  if (DFlags & DINode::FlagObjectPointer)
    Flags |= Flag::FlagIsObjectPointer;
  if (DFlags & DINode::FlagStaticMember)
    Flags |= Flag::FlagIsStaticMember;
  // inderect variable flag ?
  if (DFlags & DINode::FlagLValueReference)
    Flags |= Flag::FlagIsLValueReference;
  if (DFlags & DINode::FlagRValueReference)
    Flags |= Flag::FlagIsRValueReference;
  if (DFlags & DINode::FlagTypePassByValue)
    Flags |= Flag::FlagTypePassByValue;
  if (DFlags & DINode::FlagTypePassByReference)
    Flags |= Flag::FlagTypePassByReference;
  if (DFlags & DINode::FlagEnumClass)
    Flags |= Flag::FlagIsEnumClass;
  return Flags;
}

uint32_t
SPIRVEmitNonSemanticDI::mapDwarfTagToTypeComposite(const DICompositeType *CT) {
  switch (CT->getTag()) {
  case dwarf::DW_TAG_structure_type:
    return 0;
  case dwarf::DW_TAG_class_type:
    return 1;
  case dwarf::DW_TAG_union_type:
    return 2;
  default:
    llvm_unreachable("Unknown DWARF tag for DebugTypeComposite");
  }
}

bool SPIRVEmitNonSemanticDI::runOnModule(llvm::Module &M) {
  bool Changed = false;

  auto *MMIWrapper = getAnalysisIfAvailable<MachineModuleInfoWrapperPass>();
  if (!MMIWrapper) {
    return Changed;
  }
  auto &MMI = MMIWrapper->getMMI();

  if (MMI.getModule() != &M) {
    return Changed;
  }

  if (M.begin() == M.end())
    return false;

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (MachineFunction *MF = MMI.getMachineFunction(F)) {

      emitGlobalDI(*MF);

    } else {
      continue;
    }
  }

  return Changed;
}

void SPIRVEmitNonSemanticDI::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<MachineModuleInfoWrapperPass>();
  AU.setPreservesAll();
}
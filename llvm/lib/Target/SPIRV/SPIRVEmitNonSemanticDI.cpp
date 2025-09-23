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
  Register DebugSourceResIdReg;
  SmallVector<std::pair<const DIBasicType *const, const Register>, 12>
      &BasicTypeRegPairs;
  SmallVector<std::pair<const DICompositeType *const, const Register>, 12>
      &CompositeTypeRegPairs;

  SPIRVCodeGenContext(
      MachineIRBuilder &Builder, MachineRegisterInfo &RegInfo,
      SPIRVGlobalRegistry *Registry, const SPIRVType *VTy,
      const SPIRVType *I32Ty, const SPIRVInstrInfo *TI,
      const SPIRVRegisterInfo *TR, const RegisterBankInfo *RB,
      MachineFunction &Function, const Register &ZeroReg,
      SPIRVTargetMachine *TargetMachine, Register DebugSrcReg,
      SmallVector<std::pair<const DIBasicType *const, const Register>, 12>
          &BasicTypePairs,
      SmallVector<std::pair<const DICompositeType *const, const Register>, 12>
          &CompositeTypePairs)
      : MIRBuilder(Builder), MRI(RegInfo), GR(Registry), VoidTy(VTy),
        I32Ty(I32Ty), TII(TI), TRI(TR), RBI(RB), MF(Function),
        I32ZeroReg(ZeroReg), TM(TargetMachine),
        DebugSourceResIdReg(DebugSrcReg), BasicTypeRegPairs(BasicTypePairs),
        CompositeTypeRegPairs(CompositeTypePairs) {}
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
  uint32_t transDebugFlags(const DINode *DN);
  uint32_t mapDebugFlags(DINode::DIFlags DFlags);
  void extractTypeMetadata(DIType *Ty, DebugInfoCollector &Collector);

  Register EmitDIInstruction(SPIRV::NonSemanticExtInst::NonSemanticExtInst Inst,
                             ArrayRef<Register> Operands,
                             SPIRVCodeGenContext &Ctx);

  void emitDebugBuildIdentifier(StringRef BuildIdentifier,
                                SPIRVCodeGenContext &Ctx);

  void emitDebugStoragePath(StringRef BuildStoragePath,
                            SPIRVCodeGenContext &Ctx);

  void emitDebugBasicTypes(const SmallPtrSetImpl<DIBasicType *> &BasicTypes,
                           SPIRVCodeGenContext &Ctx);
  void emitDebugPointerTypes(
      const SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
      SPIRVCodeGenContext &Ctx);

  Register findEmittedBasicTypeReg(
      const DIType *BaseType,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitSingleCompilationUnit(StringRef FilePath, int64_t SourceLanguage,
                                 SPIRVCodeGenContext &Ctx,
                                 Register DebugInfoVersionReg,
                                 Register DwarfVersionReg,
                                 Register &DebugSourceResIdReg,
                                 Register &DebugCompUnitResIdReg);
};

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

  // Required variables to get from metadata search
  LLVMContext *Context;
  SmallVector<SmallString<128>> FilePaths;
  SmallVector<int64_t> LLVMSourceLanguages;
  int64_t DwarfVersion = 1;
  int64_t DebugInfoVersion = 1;
  SmallString<128> BuildIdentifier;
  SmallString<128> BuildStoragePath;
  Register DebugCompUnitResIdReg;
  DebugInfoCollector Collector;
  // Searching through the Module metadata to find nescessary
  // information like DwarfVersion or SourceLanguage
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
        DIFile *File = CompileUnit->getFile();
        FilePaths.emplace_back();
        sys::path::append(FilePaths.back(), File->getDirectory(),
                          File->getFilename());
        LLVMSourceLanguages.push_back(
            CompileUnit->getSourceLanguage().getUnversionedName());
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

    // This traversal is the only supported way to access
    // instruction related DI metadata like DIBasicType
    for (auto &F : *M) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          for (DbgVariableRecord &DVR : filterDbgVars(I.getDbgRecordRange())) {
            DILocalVariable *LocalVariable = DVR.getVariable();
            if (DILocalVariable *LocalVariable = DVR.getVariable())
              extractTypeMetadata(LocalVariable->getType(), Collector);
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
    SmallVector<std::pair<const DICompositeType *const, const Register>, 12>
        CompositeTypeRegPairs;
    Register DebugSourceResIdReg;

    const SPIRVType *VoidTy =
        GR->getOrCreateSPIRVType(Type::getVoidTy(*Context), MIRBuilder,
                                 SPIRV::AccessQualifier::ReadWrite, false);

    const SPIRVType *I32Ty =
        GR->getOrCreateSPIRVType(Type::getInt32Ty(*Context), MIRBuilder,
                                 SPIRV::AccessQualifier::ReadWrite, false);

    const Register DwarfVersionReg =
        GR->buildConstantInt(DwarfVersion, MIRBuilder, I32Ty, false, false);

    const Register DebugInfoVersionReg =
        GR->buildConstantInt(DebugInfoVersion, MIRBuilder, I32Ty, false, false);

    const Register I32ZeroReg =
        GR->buildConstantInt(0, MIRBuilder, I32Ty, false, false);

    SPIRVCodeGenContext Ctx(MIRBuilder, MRI, GR, VoidTy, I32Ty, TII, TRI, RBI,
                            MF, I32ZeroReg, TM, DebugSourceResIdReg,
                            BasicTypeRegPairs, CompositeTypeRegPairs);

    const DICompileUnit *CU = MF.getFunction().getSubprogram()->getUnit();

    for (unsigned Idx = 0; Idx < LLVMSourceLanguages.size(); ++Idx) {
      emitSingleCompilationUnit(FilePaths[Idx], LLVMSourceLanguages[Idx], Ctx,
                                DebugInfoVersionReg, DwarfVersionReg,
                                Ctx.DebugSourceResIdReg, DebugCompUnitResIdReg);
      Ctx.GR->addDebugValue(CU, DebugCompUnitResIdReg);
    }

    emitDebugBuildIdentifier(BuildIdentifier, Ctx);
    emitDebugStoragePath(BuildStoragePath, Ctx);
    emitDebugBasicTypes(Collector.BasicTypes, Ctx);
    emitDebugPointerTypes(Collector.PointerDerivedTypes, Ctx);
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
        Ctx.GR->buildConstantInt(0, Ctx.MIRBuilder, Ctx.I32Ty, false, false);
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
        BasicType->getSizeInBits(), Ctx.MIRBuilder, Ctx.I32Ty, false, false);

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
        AttributeEncoding, Ctx.MIRBuilder, Ctx.I32Ty, false, false);

    const Register FlagsReg = Ctx.GR->buildConstantInt(
        transDebugFlags(BasicType), Ctx.MIRBuilder, Ctx.I32Ty, false, false);

    [[maybe_unused]]
    const Register BasicTypeReg = EmitDIInstruction(
        SPIRV::NonSemanticExtInst::DebugTypeBasic,
        {BasicTypeStrReg, ConstIntBitwidthReg, AttributeEncodingReg, FlagsReg},
        Ctx);
    Ctx.GR->addDebugValue(BasicType, BasicTypeReg);
    Ctx.BasicTypeRegPairs.emplace_back(BasicType, BasicTypeReg);
  }
}

void SPIRVEmitNonSemanticDI::emitDebugPointerTypes(
    const SmallPtrSetImpl<DIDerivedType *> &PointerDerivedTypes,
    SPIRVCodeGenContext &Ctx) {
  if (PointerDerivedTypes.size()) {
    for (const auto *PointerDerivedType : PointerDerivedTypes) {

      assert(PointerDerivedType->getDWARFAddressSpace().has_value());
      const Register StorageClassReg = Ctx.GR->buildConstantInt(
          addressSpaceToStorageClass(
              PointerDerivedType->getDWARFAddressSpace().value(),
              *Ctx.TM->getSubtargetImpl()),
          Ctx.MIRBuilder, Ctx.I32Ty, false, false);
      const uint32_t Flags = transDebugFlags(PointerDerivedType);
      Register FlagsReg = Ctx.GR->buildConstantInt(Flags, Ctx.MIRBuilder,
                                                   Ctx.I32Ty, false, false);

      // If the Pointer is representing a void type it's getBaseType
      // is a nullptr
      const auto *MaybeNestedBasicType =
          dyn_cast_or_null<DIBasicType>(PointerDerivedType->getBaseType());
      if (MaybeNestedBasicType) {
        for (const auto &BasicTypeRegPair : Ctx.BasicTypeRegPairs) {
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
    break;
  }

  Register SourceLanguageReg = Ctx.GR->buildConstantInt(
      SpirvSourceLanguage, Ctx.MIRBuilder, Ctx.I32Ty, false, false);

  DebugCompUnitResIdReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugCompilationUnit,
                        {DebugInfoVersionReg, DwarfVersionReg,
                         DebugSourceResIdReg, SourceLanguageReg},
                        Ctx);

  const DIFile *File = Ctx.MF.getFunction().getSubprogram()->getFile();
  Ctx.GR->addDebugValue(File, DebugCompUnitResIdReg);
  Ctx.GR->addDebugValue(Ctx.MF.getFunction().getSubprogram()->getUnit(),
                        DebugCompUnitResIdReg);
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

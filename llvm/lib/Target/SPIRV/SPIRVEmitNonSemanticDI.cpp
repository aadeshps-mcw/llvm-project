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
struct SPIRVEmitNonSemanticDI : public MachineFunctionPass {
  static char ID;
  SPIRVTargetMachine *TM;
  SPIRVEmitNonSemanticDI(SPIRVTargetMachine *TM = nullptr)
      : MachineFunctionPass(ID), TM(TM) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  bool IsGlobalDIEmitted = false;
  bool emitGlobalDI(MachineFunction &MF);
  Register EmitOpString(StringRef Str, MachineIRBuilder &MIRBuilder,
                        MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR);

  Register EmitDIInstruction(SPIRV::NonSemanticExtInst::NonSemanticExtInst Inst,
                             ArrayRef<Register> Operands,
                             MachineIRBuilder &MIRBuilder,
                             MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR,
                             const SPIRVType *VoidTy, const SPIRVInstrInfo *TII,
                             const SPIRVRegisterInfo *TRI,
                             const RegisterBankInfo *RBI, MachineFunction &MF);

  void emitDebugMacroDefs(const DICompileUnit *CU, MachineIRBuilder &MIRBuilder,
                          MachineFunction &MF, MachineRegisterInfo &MRI,
                          const SPIRVInstrInfo *TII,
                          const SPIRVRegisterInfo *TRI,
                          const RegisterBankInfo *RBI, SPIRVGlobalRegistry *GR,
                          const SPIRVType *VoidTy, const SPIRVType *I32Ty);

  void emitDebugMacroUndef(const DIMacro *MacroUndef, StringRef FileName,
                           MachineIRBuilder &MIRBuilder,
                           MachineRegisterInfo &MRI, const SPIRVInstrInfo *TII,
                           const SPIRVRegisterInfo *TRI,
                           const RegisterBankInfo *RBI, SPIRVGlobalRegistry *GR,
                           const SPIRVType *VoidTy, const SPIRVType *I32Ty,
                           const DenseMap<StringRef, Register> &MacroDefRegs,
                           MachineFunction &MF);

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
                      SmallPtrSetImpl<const DICompositeType *> &CompositeTypes);

  void emitTemplateDebugInstructions(
      const DICompositeType *CompTy, MachineIRBuilder &MIRBuilder,
      MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR,
      const SPIRVType *VoidTy, const SPIRVType *I32Ty,
      const SPIRVInstrInfo *TII, const SPIRVRegisterInfo *TRI,
      const RegisterBankInfo *RBI, MachineFunction &MF,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitDebugTypeComposite(
      const DICompositeType *CompTy, MachineIRBuilder &MIRBuilder,
      MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR,
      const SPIRVType *VoidTy, const SPIRVType *I32Ty,
      const SPIRVInstrInfo *TII, const SPIRVRegisterInfo *TRI,
      const RegisterBankInfo *RBI, MachineFunction &MF,
      const Register &SourceReg, const Register &CUReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs);

  void emitDebugTypeMember(
      const DIDerivedType *Member, MachineIRBuilder &MIRBuilder,
      MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR,
      const SPIRVType *VoidTy, const SPIRVType *I32Ty,
      const SPIRVInstrInfo *TII, const SPIRVRegisterInfo *TRI,
      const RegisterBankInfo *RBI, MachineFunction &MF,
      const Register &CompositeReg, const Register &SourceReg,
      const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
          &BasicTypeRegPairs,
      SmallVectorImpl<Register> &MemberRegs);
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

  // Required variables to get from metadata search
  LLVMContext *Context;
  SmallVector<SmallString<128>> FilePaths;
  SmallVector<int64_t> LLVMSourceLanguages;
  int64_t DwarfVersion = 0;
  int64_t DebugInfoVersion = 0;
  SmallPtrSet<DIBasicType *, 12> BasicTypes;
  SmallPtrSet<DIDerivedType *, 12> PointerDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> QualifiedDerivedTypes;
  SmallPtrSet<DIDerivedType *, 12> TypedefTypes;
  SmallVector<const DIImportedEntity *, 5> ImportedEntities;
  SmallPtrSet<DICompositeType *, 12> ArrayTypes;
  SmallString<128> BuildIdentifier;
  SmallString<128> BuildStoragePath;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypesWithTemplates;
  SmallPtrSet<const DICompositeType *, 8> CompositeTypes;
  Register DebugSourceResIdReg;
  Register DebugCompUnitResIdReg;
  // StringRef Name;
  // StringRef LinkageName;
  // DIFile *File = nullptr;
  // DIType *Type = nullptr;
  // DIScope *Scope = nullptr;
  // unsigned Column = 1; // Default fallback
  // uint32_t Flags = 1;
  // uint32_t Line;
  // Register DebugCompUnitResIdReg;
  // Register DebugSourceResIdReg;
  // Register TypeReg;
  // Register ParentReg;
  // Register VarReg;
  // GlobalVariable *MatchedGV;
  // Register a;

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

        // for (auto *GVE : CompileUnit->getGlobalVariables()) {
        //   if (const auto *DIGV =
        //           dyn_cast<DIGlobalVariable>(GVE->getVariable())) {
        //     // Name = DIGV->getName();
        //     // LinkageName = DIGV->getLinkageName();
        //     // File = DIGV->getFile();
        //     // Type = DIGV->getType();
        //     // Scope = DIGV->getScope();
        //     // Line = DIGV->getLine();
        //     // const DIScope *Scope = DIGV->getScope();
        //     // // if (isa<DINamespace>(Scope) || isa<DISubprogram>(Scope) ||
        //     // //     isa<DIModule>(Scope)) {
        //     // //   ParentReg = emitOrGetCachedDebugScope(Scope);
        //     // // }

        //     for (GlobalVariable &G : MF.getFunction().getParent()->globals())
        //     {
        //       SmallVector<DIGlobalVariableExpression *, 4> Exprs;
        //       G.getDebugInfo(Exprs);
        //       for (DIGlobalVariableExpression *GVE : Exprs) {
        //         if (GVE->getVariable() == DIGV) {
        //           MatchedGV = &G;
        //           break;
        //         }
        //       }
        //     }
        //   }
        // }
        for (auto *GVE : CompileUnit->getGlobalVariables()) {
          if (auto *DIGV = dyn_cast<DIGlobalVariable>(GVE->getVariable())) {

            extractTypeMetadata(DIGV->getType(), BasicTypes,
                                PointerDerivedTypes, QualifiedDerivedTypes,
                                TypedefTypes, ArrayTypes, CompositeTypes);
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

    // This traversal is the only supported way to access
    // instruction related DI metadata like DIBasicType
    for (auto &F : *M) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          for (DbgVariableRecord &DVR : filterDbgVars(I.getDbgRecordRange())) {
            DILocalVariable *LocalVariable = DVR.getVariable();
            if (auto *ArrayCT =
                    dyn_cast<DICompositeType>(LocalVariable->getType())) {

              if (ArrayCT->getTag() == dwarf::DW_TAG_structure_type) {
                // If the array type is a structure, we need to extract
                // its members and add them to CompositeTypes.
                CompositeTypes.insert(ArrayCT);
              }
              if (ArrayCT->getTag() == dwarf::DW_TAG_array_type) {
                ArrayTypes.insert(ArrayCT); // Already handled
              }
            }
            if (auto *BasicType =
                    dyn_cast<DIBasicType>(LocalVariable->getType())) {
              BasicTypes.insert(BasicType);
            } else if (auto *DerivedType =
                           dyn_cast<DIDerivedType>(LocalVariable->getType())) {
              DILocalVariable *LocalVariable = DVR.getVariable();
              extractTypeMetadata(LocalVariable->getType(), BasicTypes,
                                  PointerDerivedTypes, QualifiedDerivedTypes,
                                  TypedefTypes, ArrayTypes, CompositeTypes);
              if (DerivedType->getTag() == dwarf::DW_TAG_pointer_type) {
                PointerDerivedTypes.insert(DerivedType);
                // DIBasicType can be unreachable from DbgRecord and only
                // pointed on from other DI types
                // DerivedType->getBaseType is null when pointer
                // is representing a void type
                if (auto *BT = dyn_cast_or_null<DIBasicType>(
                        DerivedType->getBaseType()))
                  BasicTypes.insert(BT);
              }
            }
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

    const DICompileUnit *CU = MF.getFunction().getSubprogram()->getUnit();
    emitDebugMacroDefs(CU, MIRBuilder, MF, MRI, TII, TRI, RBI, GR, VoidTy,
                       I32Ty);

    for (unsigned Idx = 0; Idx < LLVMSourceLanguages.size(); ++Idx) {
      const Register FilePathStrReg =
          EmitOpString(FilePaths[Idx], MIRBuilder, MRI, GR);

      DebugSourceResIdReg = EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, MIRBuilder,
          MRI, GR, VoidTy, TII, TRI, RBI, MF);

      SourceLanguage SpirvSourceLanguage = SourceLanguage::Unknown;
      switch (LLVMSourceLanguages[Idx]) {
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

      const Register SourceLanguageReg =
          GR->buildConstantInt(SpirvSourceLanguage, MIRBuilder, I32Ty, false);

      [[maybe_unused]] DebugCompUnitResIdReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugCompilationUnit,
                            {DebugInfoVersionReg, DwarfVersionReg,
                             DebugSourceResIdReg, SourceLanguageReg},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
    }

    if (!BuildIdentifier.empty()) {
      const Register BuildIdStrReg =
          EmitOpString(BuildIdentifier, MIRBuilder, MRI, GR);
      const Register FlagsReg =
          GR->buildConstantInt(1, MIRBuilder, I32Ty, false);
      const Register DebugBuildIdReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugBuildIdentifier,
                            {BuildIdStrReg, FlagsReg}, MIRBuilder, MRI, GR,
                            VoidTy, TII, TRI, RBI, MF);
    }

    if (!BuildStoragePath.empty()) {
      const Register PathStrReg =
          EmitOpString(BuildStoragePath, MIRBuilder, MRI, GR);
      const Register DebugStoragePathReg = EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugStoragePath, {PathStrReg}, MIRBuilder,
          MRI, GR, VoidTy, TII, TRI, RBI, MF);
    }

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
                            CompositeTypes);

        if (auto *CT = dyn_cast<DICompositeType>(Ty)) {
          if (!CT->getTemplateParams().empty())
            CompositeTypesWithTemplates.insert(CT);
        }
      }
    }

    // Register NameStrReg = EmitOpString(Name);
    // Register LinkageStrReg = EmitOpString(LinkageName);
    // Register LineReg = GR->buildConstantInt(Line, MIRBuilder, I32Ty, false);

    // Register ColumnReg = GR->buildConstantInt(Column, MIRBuilder, I32Ty,
    // false);
    // // Register VariableReg = GR->getOrCreateDebugInfoNone();
    // Register FlagsReg = GR->buildConstantInt(Flags, MIRBuilder, I32Ty,
    // false);

    // if (const auto *BasicType = dyn_cast<DIBasicType>(Type)) {
    //   Register TypeNameReg = EmitOpString(BasicType->getName());

    //   Register BitWidthReg = GR->buildConstantInt(BasicType->getSizeInBits(),
    //                                               MIRBuilder, I32Ty, false);
    //   Register EncodingReg = GR->buildConstantInt(BasicType->getEncoding(),
    //                                               MIRBuilder, I32Ty, false);
    //   Register FlagsRegForType =
    //       GR->buildConstantInt(1, MIRBuilder, I32Ty, false); // Usually 0

    //   Register TypeReg = EmitDIInstruction(
    //       SPIRV::NonSemanticExtInst::DebugTypeBasic,
    //       {TypeNameReg, BitWidthReg, EncodingReg, FlagsRegForType});
    // }
    // if (MatchedGV) {
    //   // TypeReg = GR->getSPIRVTypeID(MatchedGV->getType());
    // }

    // const Register BasicTypeReg = EmitDIInstruction(
    //     SPIRV::NonSemanticExtInst::DebugGlobalVariable,
    //     {NameStrReg, TypeReg, DebugSourceResIdReg, LineReg, ColumnReg,
    //      DebugCompUnitResIdReg, LinkageStrReg, FlagsReg, a});

    // We aren't extracting any DebugInfoFlags now so we
    // emitting zero to use as <id>Flags argument for DebugBasicType
    const Register I32ZeroReg =
        GR->buildConstantInt(0, MIRBuilder, I32Ty, false, false);

    // We need to store pairs because further instructions reference
    // the DIBasicTypes and size will be always small so there isn't
    // need for any kind of map
    SmallVector<std::pair<const DIBasicType *const, const Register>, 12>
        BasicTypeRegPairs;
    for (auto *BasicType : BasicTypes) {
      llvm::errs() << "Collected BasicType: " << BasicType->getName() << "\n";
      const Register BasicTypeStrReg =
          EmitOpString(BasicType->getName(), MIRBuilder, MRI, GR);

      const Register ConstIntBitwidthReg = GR->buildConstantInt(
          BasicType->getSizeInBits(), MIRBuilder, I32Ty, false);

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

      const Register AttributeEncodingReg =
          GR->buildConstantInt(AttributeEncoding, MIRBuilder, I32Ty, false);

      const Register BasicTypeReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeBasic,
                            {BasicTypeStrReg, ConstIntBitwidthReg,
                             AttributeEncodingReg, I32ZeroReg},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
      GR->addDebugValue(BasicType, BasicTypeReg);
      BasicTypeRegPairs.emplace_back(BasicType, BasicTypeReg);
      llvm::errs() << "Emitted BasicType: " << BasicType->getName()
                   << " with Reg: " << BasicTypeReg << "\n";
    }

    // Emit all composite types now
    for (auto *CT : CompositeTypes) {

      extractTypeMetadata(const_cast<DICompositeType *>(CT), BasicTypes,
                          PointerDerivedTypes, QualifiedDerivedTypes,
                          TypedefTypes, ArrayTypes, CompositeTypes);
      emitDebugTypeComposite(CT, MIRBuilder, MRI, GR, VoidTy, I32Ty, TII, TRI,
                             RBI, MF, DebugSourceResIdReg,
                             DebugCompUnitResIdReg, BasicTypeRegPairs);
    }

    if (!QualifiedDerivedTypes.empty()) {
      for (const auto *QualifiedDT : QualifiedDerivedTypes) {
        Register BaseTypeReg = findEmittedBasicTypeReg(
            QualifiedDT->getBaseType(), BasicTypeRegPairs);
        if (!BaseTypeReg)
          continue;

        const uint32_t QualifierValue =
            mapDwarfTagToTypeQualifier(QualifiedDT->getTag());
        const Register QualifierConstReg =
            GR->buildConstantInt(QualifierValue, MIRBuilder, I32Ty, false);

        [[maybe_unused]]
        const Register DebugQualifiedTypeReg =
            EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeQualifier,
                              {BaseTypeReg, QualifierConstReg}, MIRBuilder, MRI,
                              GR, VoidTy, TII, TRI, RBI, MF);
      }
    }

    for (const auto *TypedefDT : TypedefTypes) {
      Register BaseTypeReg =
          findEmittedBasicTypeReg(TypedefDT->getBaseType(), BasicTypeRegPairs);
      if (!BaseTypeReg)
        continue;

      // Emit name string for typedef
      const Register TypedefNameReg =
          EmitOpString(TypedefDT->getName(), MIRBuilder, MRI, GR);

      // Emit source file string
      DIFile *File = TypedefDT->getFile();
      const Register FilePathStrReg = EmitOpString(
          (File ? File->getFilename() : "<unknown>"), MIRBuilder, MRI, GR);

      // Emit DebugSource
      const Register DebugSourceResIdReg = EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, MIRBuilder,
          MRI, GR, VoidTy, TII, TRI, RBI, MF);

      // Emit line and column constants
      const Register LineReg =
          GR->buildConstantInt(TypedefDT->getLine(), MIRBuilder, I32Ty, false);
      const Register ColumnReg = GR->buildConstantInt(
          /* fallback */ 0, MIRBuilder, I32Ty, false);

      // Emit scope (for now: just use DebugInfoNone)
      const Register ScopeReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

      [[maybe_unused]]
      const Register DebugTypedefReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypedef,
                            {TypedefNameReg, BaseTypeReg, DebugSourceResIdReg,
                             LineReg, ColumnReg, ScopeReg},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
    }

    for (const DICompositeType *CompTy : CompositeTypesWithTemplates) {
      // Ensure we extract all types used in template parameters
      for (const auto *MD : CompTy->getTemplateParams()) {
        if (const auto *TTP = dyn_cast<DITemplateTypeParameter>(MD)) {
          extractTypeMetadata(TTP->getType(), BasicTypes, PointerDerivedTypes,
                              QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                              CompositeTypes);
        } else if (const auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
          extractTypeMetadata(TVP->getType(), BasicTypes, PointerDerivedTypes,
                              QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                              CompositeTypes);
        }
      }

      llvm::errs() << "Emitting template instructions for: "
                   << CompTy->getName() << "\n";

      emitTemplateDebugInstructions(CompTy, MIRBuilder, MRI, GR, VoidTy, I32Ty,
                                    TII, TRI, RBI, MF, BasicTypeRegPairs);
    }

    for (const auto *Imported : ImportedEntities) {
      // Skip if no name or entity
      if (!Imported->getEntity())
        continue;

      const Register NameStrReg =
          EmitOpString(Imported->getName(), MIRBuilder, MRI, GR);

      // Tag (DW_TAG_imported_module or DW_TAG_imported_declaration, etc.)
      const Register TagReg =
          GR->buildConstantInt(Imported->getTag(), MIRBuilder, I32Ty, false);

      // Source file
      const Register FilePathStrReg =
          EmitOpString(Imported->getFile() ? Imported->getFile()->getFilename()
                                           : "<unknown>",
                       MIRBuilder, MRI, GR);
      const Register DebugSourceReg = EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugSource, {FilePathStrReg}, MIRBuilder,
          MRI, GR, VoidTy, TII, TRI, RBI, MF);

      // Entity being imported – this could be a namespace or declaration
      // For now: emit DebugInfoNone as a placeholder
      const Register EntityReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

      // Line and column
      const Register LineReg =
          GR->buildConstantInt(Imported->getLine(), MIRBuilder, I32Ty, false);
      const Register ColumnReg = GR->buildConstantInt(
          0, MIRBuilder, I32Ty, false); // Column info often unavailable

      // Parent scope – again use DebugInfoNone for now
      const Register ScopeReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

      // Emit the actual DebugImportedEntity instruction
      [[maybe_unused]]
      const Register DebugImportedEntityReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugImportedEntity,
                            {NameStrReg, TagReg, DebugSourceReg, EntityReg,
                             LineReg, ColumnReg, ScopeReg},
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
    }

    for (auto *ArrayTy : ArrayTypes) {
      DIType *ElementType = ArrayTy->getBaseType();
      Register BaseTypeReg =
          findEmittedBasicTypeReg(ElementType, BasicTypeRegPairs);
      if (!BaseTypeReg)
        continue;
      DINodeArray Subranges = ArrayTy->getElements();
      if (ArrayTy->isVector()) {
        assert(Subranges.size() == 1 && "Only 1D vectors supported!");

        Register ComponentCountReg;
        if (auto *SR = dyn_cast<DISubrange>(Subranges[0])) {
          auto CountValUnion = SR->getCount();
          if (auto *CountCI = CountValUnion.dyn_cast<ConstantInt *>()) {
            uint64_t CountVal = CountCI->getZExtValue();
            ComponentCountReg =
                GR->buildConstantInt(CountVal, MIRBuilder, I32Ty, false);
          } else {
            ComponentCountReg =
                GR->buildConstantInt(0, MIRBuilder, I32Ty, false); // fallback
          }
        }

        SmallVector<Register, 4> Ops;
        Ops.push_back(BaseTypeReg);
        Ops.push_back(ComponentCountReg);

        [[maybe_unused]]
        Register DebugVectorTypeReg =
            EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeVector, Ops,
                              MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
        continue;
      }

      SmallVector<Register, 4> ComponentCountRegs;
      for (Metadata *M : Subranges) {
        if (auto *SR = dyn_cast<DISubrange>(M)) {
          auto CountValUnion = SR->getCount();
          if (auto *CountCI = CountValUnion.dyn_cast<ConstantInt *>()) {
            uint64_t CountVal = CountCI->getZExtValue();
            Register ConstCountReg =
                GR->buildConstantInt(CountVal, MIRBuilder, I32Ty, false);
            ComponentCountRegs.push_back(ConstCountReg);
          } else {
            // Runtime-sized or unknown count — emit 0 constant
            Register ConstZero =
                GR->buildConstantInt(0, MIRBuilder, I32Ty, false);
            ComponentCountRegs.push_back(ConstZero);
          }
        }
      }

      SmallVector<Register, 6> Ops;
      Ops.push_back(BaseTypeReg);
      llvm::append_range(Ops, ComponentCountRegs);

      [[maybe_unused]]
      Register DebugArrayTypeReg =
          EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeArray, Ops,
                            MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
    }

    if (PointerDerivedTypes.size()) {
      for (const auto *PointerDerivedType : PointerDerivedTypes) {

        assert(PointerDerivedType->getDWARFAddressSpace().has_value());
        const Register StorageClassReg = GR->buildConstantInt(
            addressSpaceToStorageClass(
                PointerDerivedType->getDWARFAddressSpace().value(),
                *TM->getSubtargetImpl()),
            MIRBuilder, I32Ty, false);

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
                                    {BasicTypeReg, StorageClassReg, I32ZeroReg},
                                    MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI,
                                    MF);
            }
          }
        } else {
          const Register DebugInfoNoneReg =
              EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugInfoNone, {},
                                MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
          [[maybe_unused]]
          const Register DebugPointerTypeReg =
              EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypePointer,
                                {DebugInfoNoneReg, StorageClassReg, I32ZeroReg},
                                MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
        }
      }
    }
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
    SmallPtrSetImpl<const DICompositeType *> &CompositeTypes) {
  if (!Ty)
    return;
  if (auto *CT = dyn_cast<DICompositeType>(Ty)) {
    if (CT->getTag() == dwarf::DW_TAG_array_type)
      ArrayTypes.insert(CT);
    else if (CT->getTag() == dwarf::DW_TAG_structure_type ||
             CT->getTag() == dwarf::DW_TAG_class_type ||
             CT->getTag() == dwarf::DW_TAG_union_type) {
      llvm::errs() << "Extracting composite type: " << CT->getName() << "\n";
      CompositeTypes.insert(CT);
      if (const auto *CT = dyn_cast<DICompositeType>(Ty)) {
        for (Metadata *Element : CT->getElements()) {
          if (auto *Member = dyn_cast<DIDerivedType>(Element)) {
            extractTypeMetadata(Member->getBaseType(), BasicTypes,
                                PointerDerivedTypes, QualifiedDerivedTypes,
                                TypedefTypes, ArrayTypes, CompositeTypes);
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
                              TypedefTypes, ArrayTypes, CompositeTypes);
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
                        CompositeTypes);
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
    }
    extractTypeMetadata(DT->getBaseType(), BasicTypes, PointerDerivedTypes,
                        QualifiedDerivedTypes, TypedefTypes, ArrayTypes,
                        CompositeTypes);
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

void SPIRVEmitNonSemanticDI::emitDebugMacroDefs(
    const DICompileUnit *CU, MachineIRBuilder &MIRBuilder, MachineFunction &MF,
    MachineRegisterInfo &MRI, const SPIRVInstrInfo *TII,
    const SPIRVRegisterInfo *TRI, const RegisterBankInfo *RBI,
    SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy, const SPIRVType *I32Ty) {

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

        const Register SourceStrReg =
            EmitOpString(FileName, MIRBuilder, MRI, GR);
        const Register LineConstReg =
            GR->buildConstantInt(Line, MIRBuilder, I32Ty, false);
        const Register NameStrReg = EmitOpString(Name, MIRBuilder, MRI, GR);
        const Register ValueStrReg = EmitOpString(Value, MIRBuilder, MRI, GR);

        [[maybe_unused]] const Register DebugMacroDefReg = EmitDIInstruction(
            SPIRV::NonSemanticExtInst::DebugMacroDef,
            {SourceStrReg, LineConstReg, NameStrReg, ValueStrReg}, MIRBuilder,
            MRI, GR, VoidTy, TII, TRI, RBI, MF);
        MacroDefRegs[Macro->getName()] = DebugMacroDefReg;
      } else if (Macro->getMacinfoType() == dwarf::DW_MACINFO_undef) {
        emitDebugMacroUndef(Macro, FileName, MIRBuilder, MRI, TII, TRI, RBI, GR,
                            VoidTy, I32Ty, MacroDefRegs, MF);
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
    const DIMacro *MacroUndef, StringRef FileName, MachineIRBuilder &MIRBuilder,
    MachineRegisterInfo &MRI, const SPIRVInstrInfo *TII,
    const SPIRVRegisterInfo *TRI, const RegisterBankInfo *RBI,
    SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy, const SPIRVType *I32Ty,
    const DenseMap<StringRef, Register> &MacroDefRegs, MachineFunction &MF) {

  const StringRef Name = MacroUndef->getName();
  const unsigned Line = MacroUndef->getLine();

  // We need the macro def for this name
  auto It = MacroDefRegs.find(Name);
  if (It == MacroDefRegs.end())
    return; // No previous DebugMacroDef emitted — skip

  Register MacroDefReg = It->second;

  Register SourceStrReg = EmitOpString(FileName, MIRBuilder, MRI, GR);
  Register LineConstReg = GR->buildConstantInt(Line, MIRBuilder, I32Ty, false);

  [[maybe_unused]] Register MacroUndefReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugMacroUndef,
                        {SourceStrReg, LineConstReg, MacroDefReg}, MIRBuilder,
                        MRI, GR, VoidTy, TII, TRI, RBI, MF);
}

Register SPIRVEmitNonSemanticDI::EmitOpString(StringRef SR,
                                              MachineIRBuilder &MIRBuilder,
                                              MachineRegisterInfo &MRI,
                                              SPIRVGlobalRegistry *GR) {
  const Register StrReg = MRI.createVirtualRegister(&SPIRV::IDRegClass);
  MRI.setType(StrReg, LLT::scalar(32));
  MachineInstrBuilder MIB = MIRBuilder.buildInstr(SPIRV::OpString);
  MIB.addDef(StrReg);
  addStringImm(SR, MIB);
  return StrReg;
}
Register SPIRVEmitNonSemanticDI::EmitDIInstruction(
    SPIRV::NonSemanticExtInst::NonSemanticExtInst Inst,
    ArrayRef<Register> Operands, MachineIRBuilder &MIRBuilder,
    MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy,
    const SPIRVInstrInfo *TII, const SPIRVRegisterInfo *TRI,
    const RegisterBankInfo *RBI, MachineFunction &MF) {
  const Register InstReg = MRI.createVirtualRegister(&SPIRV::IDRegClass);
  MRI.setType(InstReg, LLT::scalar(32));
  MachineInstrBuilder MIB =
      MIRBuilder.buildInstr(SPIRV::OpExtInst)
          .addDef(InstReg)
          .addUse(GR->getSPIRVTypeID(VoidTy))
          .addImm(static_cast<int64_t>(
              SPIRV::InstructionSet::NonSemantic_Shader_DebugInfo_100))
          .addImm(Inst);
  for (auto Reg : Operands)
    MIB.addUse(Reg);
  MIB.constrainAllUses(*TII, *TRI, *RBI);
  GR->assignSPIRVTypeToVReg(VoidTy, InstReg, MF);
  return InstReg;
}
void SPIRVEmitNonSemanticDI::emitTemplateDebugInstructions(
    const DICompositeType *CompTy, MachineIRBuilder &MIRBuilder,
    MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy,
    const SPIRVType *I32Ty, const SPIRVInstrInfo *TII,
    const SPIRVRegisterInfo *TRI, const RegisterBankInfo *RBI,
    MachineFunction &MF,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  const DINodeArray TemplateParams = CompTy->getTemplateParams();
  if (TemplateParams.empty())
    return;

  // Get debug info context
  Register SourceReg = GR->getOrCreateSourceOperand(
      CompTy->getFile(), MIRBuilder, MRI, TII, VoidTy, GR);
  Register LineReg = GR->buildConstantInt(CompTy->getLine(), MIRBuilder, I32Ty);
  Register ColumnReg = GR->buildConstantInt(
      0, MIRBuilder, I32Ty); // No column info in DICompositeType

  SmallVector<Register, 4> ParamRegs;

  for (const auto *MD : TemplateParams) {
    if (auto *TTP = dyn_cast<DITemplateTypeParameter>(MD)) {
      Register NameStr = EmitOpString(TTP->getName(), MIRBuilder, MRI, GR);
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
          EmitDebugInfoNone(MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

      ParamRegs.push_back(EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
          {NameStr, TypeReg, NoneReg, SourceReg, LineReg, ColumnReg},
          MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF));

    } else if (auto *TVP = dyn_cast<DITemplateValueParameter>(MD)) {
      Register NameStr = EmitOpString(TVP->getName(), MIRBuilder, MRI, GR);
      Register TypeReg =
          findEmittedBasicTypeReg(TVP->getType(), BasicTypeRegPairs);
      if (!TypeReg)
        continue;

      // TODO: Extract actual constant value. For now fallback to 0.
      Register ValueReg = GR->buildConstantInt(0, MIRBuilder, I32Ty, false);

      ParamRegs.push_back(EmitDIInstruction(
          SPIRV::NonSemanticExtInst::DebugTypeTemplateParameter,
          {NameStr, TypeReg, ValueReg, SourceReg, LineReg, ColumnReg},
          MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF));
    }
  }

  Register CompositeReg = GR->getDebugValue(CompTy);
  if (!CompositeReg.isValid()) {
    llvm::errs() << "Missing DebugTypeComposite for templated type: "
                 << CompTy->getName() << "\n";
    return;
  }

  // Insert CompositeReg as first operand
  ParamRegs.insert(ParamRegs.begin(), CompositeReg);

  EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeTemplate, ParamRegs,
                    MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);
}

void SPIRVEmitNonSemanticDI::emitDebugTypeComposite(
    const DICompositeType *CompTy, MachineIRBuilder &MIRBuilder,
    MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy,
    const SPIRVType *I32Ty, const SPIRVInstrInfo *TII,
    const SPIRVRegisterInfo *TRI, const RegisterBankInfo *RBI,
    MachineFunction &MF, const Register &SourceReg, const Register &CUReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs) {

  if (!CompTy || CompTy->getTag() != dwarf::DW_TAG_structure_type)
    return;

  if (Register Existing = GR->getDebugValue(CompTy); Existing.isValid()) {
    llvm::errs() << "Skipping already emitted composite: " << CompTy->getName()
                 << "\n";
    return;
  }

  Register NameStr = EmitOpString(CompTy->getName(), MIRBuilder, MRI, GR);
  Register LinkageNameStr =
      EmitOpString(CompTy->getIdentifier(), MIRBuilder, MRI, GR);
  Register Tag =
      GR->buildConstantInt(CompTy->getTag(), MIRBuilder, I32Ty, false);
  Register Line =
      GR->buildConstantInt(CompTy->getLine(), MIRBuilder, I32Ty, false);
  Register Column =
      GR->buildConstantInt(0, MIRBuilder, I32Ty, false); // Not available
  Register SizeReg =
      GR->buildConstantInt(CompTy->getSizeInBits(), MIRBuilder, I32Ty, false);
  Register FlagsReg =
      GR->buildConstantInt(CompTy->getFlags(), MIRBuilder, I32Ty, false);

  Register Res = MRI.createVirtualRegister(&SPIRV::IDRegClass);
  MRI.setType(Res, LLT::scalar(32));

  SmallVector<Register, 4> MemberRegs;

  for (Metadata *El : CompTy->getElements()) {
    if (auto *DTM = dyn_cast<DIDerivedType>(El)) {
      emitDebugTypeMember(DTM, MIRBuilder, MRI, GR, VoidTy, I32Ty, TII, TRI,
                          RBI, MF, Res, SourceReg, BasicTypeRegPairs,
                          MemberRegs);
    }
  }

  SmallVector<Register, 12> Ops = {NameStr,        Tag,     SourceReg,
                                   Line,           Column,  CUReg,
                                   LinkageNameStr, SizeReg, FlagsReg};
  Ops.append(MemberRegs);

  Res = EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeComposite, Ops,
                          MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

  GR->addDebugValue(CompTy, Res);
}

void SPIRVEmitNonSemanticDI::emitDebugTypeMember(
    const DIDerivedType *Member, MachineIRBuilder &MIRBuilder,
    MachineRegisterInfo &MRI, SPIRVGlobalRegistry *GR, const SPIRVType *VoidTy,
    const SPIRVType *I32Ty, const SPIRVInstrInfo *TII,
    const SPIRVRegisterInfo *TRI, const RegisterBankInfo *RBI,
    MachineFunction &MF, const Register &CompositeReg,
    const Register &SourceReg,
    const SmallVectorImpl<std::pair<const DIBasicType *const, const Register>>
        &BasicTypeRegPairs,
    SmallVectorImpl<Register> &MemberRegs) {

  if (!Member || Member->getTag() != dwarf::DW_TAG_member)
    return;

  Register NameStr = EmitOpString(Member->getName(), MIRBuilder, MRI, GR);

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

  Register LineReg =
      GR->buildConstantInt(Member->getLine(), MIRBuilder, I32Ty, false);
  Register ColumnReg = GR->buildConstantInt(0, MIRBuilder, I32Ty, false);
  Register OffsetReg =
      GR->buildConstantInt(Member->getOffsetInBits(), MIRBuilder, I32Ty, false);
  Register SizeReg =
      GR->buildConstantInt(Member->getSizeInBits(), MIRBuilder, I32Ty, false);
  Register FlagsReg =
      GR->buildConstantInt(Member->getFlags(), MIRBuilder, I32Ty, false);

  SmallVector<Register, 10> Ops = {NameStr,   TypeReg,   SourceReg, LineReg,
                                   ColumnReg, OffsetReg, SizeReg,   FlagsReg};

  Register MemberReg =
      EmitDIInstruction(SPIRV::NonSemanticExtInst::DebugTypeMember, Ops,
                        MIRBuilder, MRI, GR, VoidTy, TII, TRI, RBI, MF);

  MemberRegs.push_back(MemberReg);
}

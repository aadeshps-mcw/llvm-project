; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info --print-after=spirv-nonsemantic-debug-info -O0 -mtriple=spirv64-unknown-unknown %s -o - 2>&1 | FileCheck %s --check-prefix=CHECK-MIR
; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - | FileCheck %s --check-prefix=CHECK-SPIRV
; RUN: llc --verify-machineinstrs -O0 -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_non_semantic_info %s -o - | FileCheck %s --check-prefix=CHECK-OPTION
; RUN: %if spirv-tools %{ llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - -filetype=obj | spirv-val %}

; CHECK-MIR-DAG: [[TYPE_INT:%[0-9]+:type]] = OpTypeInt 32, 0
; CHECK-MIR-DAG: [[TYPE_VOID:%[0-9]+:type.*]] = OpTypeVoid
; CHECK-MIR-DAG: [[STR_FILE:%[0-9]+:id\(s32\)]] = OpString {{[0-9, ]+}}
; CHECK-MIR: [[DBG_SOURCE:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 35
; CHECK-MIR: [[DBG_CU:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 1, {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, [[DBG_SOURCE]], {{%[0-9]+\:[a-z0-9\(\)]+}}
; CHECK-MIR-DAG: [[STR_INT:%[0-9]+:id\(s32\)]] = OpString 7630441
; CHECK-MIR: [[DBG_INT:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 2, [[STR_INT]]
; CHECK-MIR-DAG: [[STR_MEMBER:%[0-9]+:id\(s32\)]] = OpString 120
; CHECK-MIR: [[DBG_INHERIT:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 11, [[STR_MEMBER]], [[DBG_INT]], [[DBG_SOURCE]]
; CHECK-MIR: [[DBG_COMP2:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 10, {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, [[DBG_SOURCE]], {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, [[DBG_CU]], {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, [[DBG_INHERIT]]
; CHECK-MIR: [[DBG_FINAL:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 12, [[DBG_COMP2]], {{%[0-9]+\:[a-z0-9\(\)]+}}

; CHECK-SPIRV: [[y_str:%[0-9]+]] = OpString "y"
; CHECK-SPIRV: [[x_str:%[0-9]+]] = OpString "x"
; CHECK-SPIRV-DAG: [[type_void:%[0-9]+]] = OpTypeVoid
; CHECK-SPIRV-DAG: [[type_i32:%[0-9]+]] = OpTypeInt 32 0
; CHECK-SPIRV: [[dbg_src:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugSource 
; CHECK-SPIRV: [[dbg_cu:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugCompilationUnit %[[#]] %[[#]] [[dbg_src]] %[[#]]
; CHECK-SPIRV: [[dbg_int:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeBasic
; CHECK-SPIRV: [[dbg_member_y:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeMember [[y_str]] [[dbg_int]] [[dbg_src]] %[[#]] %[[#]] %[[#]] %[[#]] %[[#]]
; CHECK-SPIRV: [[dbg_derived:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeComposite %[[#]] %[[#]] [[dbg_src]] %[[#]] %[[#]] [[dbg_cu]] %[[#]] %[[#]] %[[#]] [[dbg_member_y]]
; CHECK-SPIRV: [[dbg_member_x:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeMember [[x_str]] [[dbg_int]] [[dbg_src]]
; CHECK-SPIRV: [[dbg_base:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeComposite %[[#]] %[[#]] [[dbg_src]] %[[#]] %[[#]] [[dbg_cu]] %[[#]] %[[#]] %[[#]] [[dbg_member_x]]
; CHECK-SPIRV: [[dbg_inherit:%[0-9]+]] = OpExtInst [[type_void]] %[[#]] DebugTypeInheritance [[dbg_base]]

; CHECK-OPTION-NOT: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

%class.Derived = type { %class.Base, i32 }
%class.Base = type { i32 }

define dso_local noundef i32 @main() #!dbg !10 {
entry:
  %retval = alloca i32, align 4
  %d = alloca %class.Derived, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %d, !15, !DIExpression(), !23)
  %x = getelementptr inbounds nuw %class.Base, ptr %d, i32 0, i32 0, !dbg !24
  store i32 10, ptr %x, align 4, !dbg !25
  %y = getelementptr inbounds nuw %class.Derived, ptr %d, i32 0, i32 1, !dbg !26
  store i32 20, ptr %y, align 4, !dbg !27
  %x1 = getelementptr inbounds nuw %class.Base, ptr %d, i32 0, i32 0, !dbg !28
  %0 = load i32, ptr %x1, align 4, !dbg !28
  %y2 = getelementptr inbounds nuw %class.Derived, ptr %d, i32 0, i32 1, !dbg !29
  %1 = load i32, ptr %y2, align 4, !dbg !29
  %add = add nsw i32 %0, %1, !dbg !30
  ret i32 %add, !dbg !31
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version XX.X.XXXX (FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "inheritance.cpp", directory: "/AAAAAAAAAA/BBBBBBBB/CCCCCCCCC", checksumkind: CSK_MD5, checksum: "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !11, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed, flags: DIFlagPublic)
!14 = !{}
!15 = !DILocalVariable(name: "d", scope: !10, file: !1, line: 13, type: !16)
!16 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Derived", file: !1, line: 7, size: 64, flags: DIFlagTypePassByValue, elements: !17, identifier: "_ZTS7Derived")
!17 = !{!18, !22}
!18 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !16, baseType: !19, flags: DIFlagPublic, extraData: i32 0)
!19 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base", file: !1, line: 2, size: 32, flags: DIFlagTypePassByValue, elements: !20, identifier: "_ZTS4Base")
!20 = !{!21}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !19, file: !1, line: 4, baseType: !13, size: 32, flags: DIFlagPublic)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !16, file: !1, line: 9, baseType: !13, size: 32, offset: 32, flags: DIFlagPublic)
!23 = !DILocation(line: 13, column: 13, scope: !10)
!24 = !DILocation(line: 14, column: 7, scope: !10)
!25 = !DILocation(line: 14, column: 9, scope: !10)
!26 = !DILocation(line: 15, column: 7, scope: !10)
!27 = !DILocation(line: 15, column: 9, scope: !10)
!28 = !DILocation(line: 16, column: 14, scope: !10)
!29 = !DILocation(line: 16, column: 20, scope: !10)
!30 = !DILocation(line: 16, column: 16, scope: !10)
!31 = !DILocation(line: 16, column: 5, scope: !10)

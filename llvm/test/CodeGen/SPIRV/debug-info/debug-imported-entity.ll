; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info --print-after=spirv-nonsemantic-debug-info -O0 -mtriple=spirv64-unknown-unknown %s -o - 2>&1 | FileCheck %s --check-prefix=CHECK-MIR
; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - | FileCheck %s --check-prefix=CHECK-SPIRV
; RUN: llc --verify-machineinstrs -O0 -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_non_semantic_info %s -o - | FileCheck %s --check-prefix=CHECK-OPTION
; RUN: %if spirv-tools %{ llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - -filetype=obj | spirv-val %}

; CHECK-MIR-DAG: [[i32type:%[0-9]+\:type]] = OpTypeInt 32, 0
; CHECK-MIR-DAG: [[void_type:%[0-9]+\:type\(s64\)]] = OpTypeVoid
; CHECK-MIR-DAG: [[i32_0:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 0
; CHECK-MIR-DAG: [[i32_1:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 1
; CHECK-MIR-DAG: [[i32_2:%[0-9]+\:iid]] = OpConstantI [[i32type]], 2
; CHECK-MIR-DAG: [[i32_3:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 3
; CHECK-MIR-DAG: [[i32_4:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 4
; CHECK-MIR-DAG: [[i32_5:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 5
; CHECK-MIR-DAG: [[i32_12:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 12
; CHECK-MIR-DAG: [[string_ns_h:%[0-9]+\:id\(s32\)]] = OpString 1869639017, 1663988850, 28784
; CHECK-MIR-DAG: [[debug_source_ns:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 35, [[string_ns_h]]
; CHECK-MIR-DAG: [[string_import_cpp:%[0-9]+\:id\(s32\)]] = OpString 1886221359, 1886218543, 779383407, 7368803
; CHECK-MIR-DAG: [[debug_source_import:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 35, [[string_import_cpp]]
; CHECK-MIR-DAG: [[string_empty:%[0-9]+\:id\(s32\)]] = OpString 0
; CHECK-MIR-DAG: [[debug_comp_unit:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 1, [[i32_3]], [[i32_5]], [[debug_source_import]], [[i32_12]]
; CHECK-MIR-DAG: [[debug_info_none:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 0
; CHECK-MIR-DAG: [[debug_imported_entity:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 34, [[string_empty]], [[i32_0]], [[debug_source_ns]], [[debug_info_none]], [[i32_4]], [[i32_1]], [[debug_comp_unit]]

; CHECK-SPIRV-DAG: [[i32type:%[0-9]+]] = OpTypeInt 32 0
; CHECK-SPIRV-DAG: [[void_type:%[0-9]+]] = OpTypeVoid
; CHECK-SPIRV-DAG: [[i32_0:%[0-9]+]] = OpConstant [[i32type]] 0
; CHECK-SPIRV-DAG: [[i32_1:%[0-9]+]] = OpConstant [[i32type]] 1
; CHECK-SPIRV-DAG: [[i32_2:%[0-9]+]] = OpConstant [[i32type]] 2
; CHECK-SPIRV-DAG: [[i32_3:%[0-9]+]] = OpConstant [[i32type]] 3
; CHECK-SPIRV-DAG: [[i32_4:%[0-9]+]] = OpConstant [[i32type]] 4
; CHECK-SPIRV-DAG: [[i32_5:%[0-9]+]] = OpConstant [[i32type]] 5
; CHECK-SPIRV-DAG: [[i32_12:%[0-9]+]] = OpConstant [[i32type]] 12
; CHECK-SPIRV-DAG: [[string_import_cpp:%[0-9]+]] = OpString "/tmp/import.cpp"
; CHECK-SPIRV-DAG: [[string_empty:%[0-9]+]] = OpString ""
; CHECK-SPIRV-DAG: [[string_ns_h:%[0-9]+]] = OpString "import.cpp"
; CHECK-SPIRV-DAG: [[debug_source_ns:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugSource [[string_ns_h]]
; CHECK-SPIRV-DAG: [[debug_source_import:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugSource [[string_import_cpp]]
; CHECK-SPIRV-DAG: [[debug_comp_unit:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugCompilationUnit [[i32_3]] [[i32_5]] [[debug_source_import]] [[i32_12]]
; CHECK-SPIRV-DAG: [[debug_info_none:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugInfoNone
; CHECK-SPIRV-DAG: [[debug_imported_entity:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugImportedEntity [[string_empty]] [[i32_0]] [[debug_source_ns]] [[debug_info_none]] [[i32_4]] [[i32_1]] [[debug_comp_unit]]

; CHECK-OPTION-NOT: DebugImportedEntity
; CHECK-OPTION-NOT: DebugTypeBasic
; CHECK-OPTION-NOT: DebugCompilationUnit

define dso_local noundef i32 @_ZN4myns9double_itEi(i32 noundef %x) !dbg !13 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
  ;dbg_declare(ptr %x.addr, !19, !DIExpression(), !20)
  %0 = load i32, ptr %x.addr, align 4, !dbg !21
  %mul = mul nsw i32 %0, 2, !dbg !22
  ret i32 %mul, !dbg !23
}

define dso_local noundef i32 @main() !dbg !24 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  ;dbg_declare(ptr %a, !27, !DIExpression(), !28)
  store i32 5, ptr %a, align 4, !dbg !28
  ;dbg_declare(ptr %b, !29, !DIExpression(), !30)
  %0 = load i32, ptr %a, align 4, !dbg !31
  %call = call noundef i32 @_ZN4myns9double_itEi(i32 noundef %0), !dbg !32
  store i32 %call, ptr %b, align 4, !dbg !30
  %1 = load i32, ptr %b, align 4, !dbg !33
  ret i32 %1, !dbg !34
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!5, !6, !7, !8, !9, !10, !11}

!0 = distinct !DICompileUnit(language: DW_LANG_Zig, file: !1, producer: "clang version XX.X", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "import.cpp", directory: "/tmp")
!2 = !{!3}
!3 = !DIImportedEntity(tag: DW_TAG_imported_module, scope: !0, entity: !4, file: !1, line: 4)
!4 = !DINamespace(name: "myns", scope: null)
!5 = !{i32 7, !"Dwarf Version", i32 5}
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{i32 8, !"PIC Level", i32 2}
!9 = !{i32 7, !"PIE Level", i32 2}
!10 = !{i32 7, !"uwtable", i32 2}
!11 = !{i32 7, !"frame-pointer", i32 2}
!13 = distinct !DISubprogram(name: "double_it", linkageName: "_ZN4myns9double_itEi", scope: !4, file: !14, line: 5, type: !15, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!14 = !DIFile(filename: "./ns.h", directory: "/tmp")
!15 = !DISubroutineType(types: !16)
!16 = !{!17, !17}
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !{}
!19 = !DILocalVariable(name: "x", arg: 1, scope: !13, file: !14, line: 5, type: !17)
!20 = !DILocation(line: 5, column: 23, scope: !13)
!21 = !DILocation(line: 5, column: 35, scope: !13)
!22 = !DILocation(line: 5, column: 37, scope: !13)
!23 = !DILocation(line: 5, column: 28, scope: !13)
!24 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !25, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!25 = !DISubroutineType(types: !26)
!26 = !{!17}
!27 = !DILocalVariable(name: "a", scope: !24, file: !1, line: 7, type: !17)
!28 = !DILocation(line: 7, column: 9, scope: !24)
!29 = !DILocalVariable(name: "b", scope: !24, file: !1, line: 8, type: !17)
!30 = !DILocation(line: 8, column: 9, scope: !24)
!31 = !DILocation(line: 8, column: 23, scope: !24)
!32 = !DILocation(line: 8, column: 13, scope: !24)
!33 = !DILocation(line: 9, column: 12, scope: !24)
!34 = !DILocation(line: 9, column: 5, scope: !24)
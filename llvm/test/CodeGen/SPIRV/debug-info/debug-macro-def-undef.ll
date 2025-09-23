; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info --print-after=spirv-nonsemantic-debug-info -O0 -mtriple=spirv64-unknown-unknown %s -o - 2>&1 | FileCheck %s --check-prefix=CHECK-MIR
; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - | FileCheck %s --check-prefix=CHECK-SPIRV
; RUN: llc --verify-machineinstrs -O0 -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_non_semantic_info %s -o - | FileCheck %s --check-prefix=CHECK-OPTION
; RUN: %if spirv-tools %{ llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - -filetype=obj | spirv-val %}

; CHECK-MIR-DAG: [[i32type:%[0-9]+\:type]] = OpTypeInt 32, 0
; CHECK-MIR-DAG: [[void_type:%[0-9]+\:type\(s64\)]] = OpTypeVoid
; CHECK-MIR-DAG: [[func_type:%[0-9]+\:type]] = OpTypeFunction [[i32type]]
; CHECK-MIR-DAG: [[ptr_type:%[0-9]+\:type]] = OpTypePointer 7, [[i32type]]
; CHECK-MIR-DAG: [[i32_0:%[0-9]+\:iid]] = OpConstantNull [[i32type]]
; CHECK-MIR-DAG: [[i32_1:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 1
; CHECK-MIR-DAG: [[i32_2:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 2
; CHECK-MIR-DAG: [[i32_3:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 3
; CHECK-MIR-DAG: [[i32_5:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 5
; CHECK-MIR-DAG: [[i32_12:%[0-9]+\:iid\(s32\)]] = OpConstantI [[i32type]], 12
; CHECK-MIR-DAG: [[string_file:%[0-9]+\:id\(s32\)]] = OpString 1886221359, 1684960559, 1663985253, 0
; CHECK-MIR-DAG: [[debug_source:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 35, [[string_file]]
; CHECK-MIR-DAG: [[string_macro_name:%[0-9]+\:id\(s32\)]] = OpString 1701080693, 6499942
; CHECK-MIR-DAG: [[string_empty:%[0-9]+\:id\(s32\)]] = OpString 1163544915, 0
; CHECK-MIR-DAG: [[string_macro_value:%[0-9]+\:id\(s32\)]] = OpString 53
; CHECK-MIR-DAG: [[debug_comp_unit:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 1, [[i32_3]], [[i32_5]], [[debug_source]], [[i32_12]]
; CHECK-MIR-DAG: [[debug_macrodef:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 32, [[string_macro_name]], [[i32_1]], [[string_empty]], [[string_macro_value]]
; CHECK-MIR-DAG: [[string_undef:%[0-9]+\:id\(s32\)]] = OpString 1701080693, 6499942
; CHECK-MIR-DAG: [[debug_macro_undef:%[0-9]+\:id\(s32\)]] = OpExtInst [[void_type]], 3, 33, [[string_undef]], [[i32_2]], [[debug_macrodef]]

; CHECK-SPIRV-DAG: [[i32type:%[0-9]+]] = OpTypeInt 32 0
; CHECK-SPIRV-DAG: [[void_type:%[0-9]+]] = OpTypeVoid
; CHECK-SPIRV-DAG: [[func_type:%[0-9]+]] = OpTypeFunction [[i32type]]
; CHECK-SPIRV-DAG: [[ptr_type:%[0-9]+]] = OpTypePointer Function [[i32type]]
; CHECK-SPIRV-DAG: [[i32_0:%[0-9]+]] = OpConstantNull [[i32type]]
; CHECK-SPIRV-DAG: [[i32_1:%[0-9]+]] = OpConstant [[i32type]] 1
; CHECK-SPIRV-DAG: [[i32_2:%[0-9]+]] = OpConstant [[i32type]] 2
; CHECK-SPIRV-DAG: [[i32_3:%[0-9]+]] = OpConstant [[i32type]] 3
; CHECK-SPIRV-DAG: [[i32_5:%[0-9]+]] = OpConstant [[i32type]] 5
; CHECK-SPIRV-DAG: [[i32_12:%[0-9]+]] = OpConstant [[i32type]] 12
; CHECK-SPIRV-DAG: [[string_file:%[0-9]+]] = OpString "/tmp/undef.c"
; CHECK-SPIRV-DAG: [[debug_source:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugSource [[string_file]]
; CHECK-SPIRV-DAG: [[string_macro_name:%[0-9]+]] = OpString "undef.c"
; CHECK-SPIRV-DAG: [[string_macro_value:%[0-9]+]] = OpString "5"
; CHECK-SPIRV-DAG: [[string_size:%[0-9]+]] = OpString "SIZE"
; CHECK-SPIRV-DAG: [[debug_comp_unit:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugCompilationUnit [[i32_3]] [[i32_5]] [[debug_source]] [[i32_12]]
; CHECK-SPIRV-DAG: [[debug_macrodef:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugMacroDef [[string_macro_name]] [[i32_1]] [[string_size]] [[string_macro_value]]
; CHECK-SPIRV-DAG: [[string_undef:%[0-9]+]] = OpString "undef.c"
; CHECK-SPIRV-DAG: [[debug_macro_undef:%[0-9]+]] = OpExtInst [[void_type]] {{%[0-9]+}} DebugMacroUndef [[string_undef]] [[i32_2]] [[debug_macrodef]]

; CHECK-OPTION-NOT: DebugMacroDef
; CHECK-OPTION-NOT: DebugMacroUndef
; CHECK-OPTION-NOT: DebugSource
; CHECK-OPTION-NOT: DebugCompilationUnit

define dso_local i32 @main() #0 !dbg !24 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  ret i32 0, !dbg !28
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!16, !17, !18, !19, !20, !21, !22}

!0 = distinct !DICompileUnit(language: DW_LANG_Zig, file: !1, producer: "clang version XX.X" , isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, macros: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "undef.c", directory: "/tmp")
!2 = !{!3, !7, !8, !9, !10, !11, !12, !13, !14, !15}
!3 = !DIMacroFile(file: !1, nodes: !4)
!4 = !{!5, !6}
!5 = !DIMacro(type: DW_MACINFO_define, line: 1, name: "SIZE", value: "5")
!6 = !DIMacro(type: DW_MACINFO_undef, line: 2, name: "SIZE")
!7 = !DIMacro(type: DW_MACINFO_define, name: "__STDC__", value: "1")
!8 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_HOSTED__", value: "1")
!9 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_VERSION__", value: "201710L")
!10 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_UTF_16__", value: "1")
!11 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_UTF_32__", value: "1")
!12 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_NOT_FOUND__", value: "0")
!13 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_FOUND__", value: "1")
!14 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_EMPTY__", value: "2")
!15 = !DIMacro(type: DW_MACINFO_define, name: "__GCC_HAVE_DWARF2_CFI_ASM", value: "1")
!16 = !{i32 7, !"Dwarf Version", i32 5}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 1, !"wchar_size", i32 4}
!19 = !{i32 8, !"PIC Level", i32 2}
!20 = !{i32 7, !"PIE Level", i32 2}
!21 = !{i32 7, !"uwtable", i32 2}
!22 = !{i32 7, !"frame-pointer", i32 2}
!24 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 4, type: !25, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !0)
!25 = !DISubroutineType(types: !26)
!26 = !{!27}
!27 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed , flags: DIFlagPublic)
!28 = !DILocation(line: 5, column: 3, scope: !24)
; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info --print-after=spirv-nonsemantic-debug-info -O0 -mtriple=spirv64-unknown-unknown %s -o - 2>&1 | FileCheck %s --check-prefix=CHECK-MIR
; RUN: llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - | FileCheck %s --check-prefix=CHECK-SPIRV
; RUN: llc --verify-machineinstrs -O0 -mtriple=spirv64-unknown-unknown --spirv-ext=+SPV_KHR_non_semantic_info %s -o - | FileCheck %s --check-prefix=CHECK-OPTION
; RUN: %if spirv-tools %{ llc --verify-machineinstrs --spv-emit-nonsemantic-debug-info --spirv-ext=+SPV_KHR_non_semantic_info -O0 -mtriple=spirv64-unknown-unknown %s -o - -filetype=obj | spirv-val %}

; CHECK-MIR-DAG: [[TYPE_I32:%[0-9]+:type.*]] = OpTypeInt 32, 0
; CHECK-MIR-DAG: [[TYPE_VOID:%[0-9]+:type.*]] = OpTypeVoid
; CHECK-MIR: [[DBG_SOURCE:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 35
; CHECK-MIR: [[DBG_CU:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 1, {{%[0-9]+\:[a-z0-9\(\)]+}}, {{%[0-9]+\:[a-z0-9\(\)]+}}, [[DBG_SOURCE]], {{%[0-9]+\:[a-z0-9\(\)]+}}
; CHECK-MIR-DAG: [[STR_INT:%[0-9]+:id\(s32\)]] = OpString 7630441
; CHECK-MIR: [[DBG_INT:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 2, [[STR_INT]]
; CHECK-MIR: [[DBG_VEC:%[0-9]+:id\(s32\)]] = OpExtInst [[TYPE_VOID]], 3, 6, [[DBG_INT]]

; CHECK-SPIRV-DAG: [[int_ty:%[0-9]+]] = OpTypeInt 32 0
; CHECK-SPIRV-DAG: [[void_ty:%[0-9]+]] = OpTypeVoid
; CHECK-SPIRV-DAG: [[const5:%[0-9]+]] = OpConstant [[int_ty]] 5
; CHECK-SPIRV-DAG: [[const3:%[0-9]+]] = OpConstant [[int_ty]] 3
; CHECK-SPIRV-DAG: [[const1:%[0-9]+]] = OpConstant [[int_ty]] 1
; CHECK-SPIRV-DAG: [[const12:%[0-9]+]] = OpConstant [[int_ty]] 12
; CHECK-SPIRV-DAG: [[const32:%[0-9]+]] = OpConstant [[int_ty]] 32
; CHECK-SPIRV-DAG: [[const4:%[0-9]+]] = OpConstant [[int_ty]] 4
; CHECK-SPIRV-DAG: [[const0:%[0-9]+]] = OpConstant [[int_ty]] 0
; CHECK-SPIRV: [[dbg_src:%[0-9]+]] = OpExtInst [[void_ty]] %[[#]] DebugSource
; CHECK-SPIRV: OpExtInst [[void_ty]] %[[#]] DebugCompilationUnit [[const3]] [[const5]] [[dbg_src]] [[const12]]
; CHECK-SPIRV: [[dbg_int:%[0-9]+]] = OpExtInst [[void_ty]] %[[#]] DebugTypeBasic %[[#]] [[const32]] [[const4]] [[const0]]
; CHECK-SPIRV: [[dbg_vec4:%[0-9]+]] = OpExtInst [[void_ty]] %[[#]] DebugTypeVector [[dbg_int]] [[const4]]

; CHECK-OPTION-NOT: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"

define dso_local void @foo(<4 x i32> noundef %a)!dbg !10 {
entry:
  %a.addr = alloca <4 x i32>, align 16
  %b = alloca <4 x i32>, align 16
  store <4 x i32> %a, ptr %a.addr, align 16
    #dbg_declare(ptr %a.addr, !19, !DIExpression(), !20)
    #dbg_declare(ptr %b, !21, !DIExpression(), !22)
  %0 = load <4 x i32>, ptr %a.addr, align 16, !dbg !23
  %1 = load <4 x i32>, ptr %a.addr, align 16, !dbg !24
  %add = add <4 x i32> %0, %1, !dbg !25
  store <4 x i32> %add, ptr %b, align 16, !dbg !22
  ret void, !dbg !26
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: ""clang version XX.X.XXXX (FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "vector.c", directory: "/AAAAAAAAAA/BBBBBBBB/CCCCCCCCC", checksumkind: CSK_MD5, checksum: "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!10 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 3, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13}
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "v4i", file: !1, line: 1, baseType: !14)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !15, size: 128, flags: DIFlagVector, elements: !16)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !{!17}
!17 = !DISubrange(count: 4)
!18 = !{}
!19 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 3, type: !13)
!20 = !DILocation(line: 3, column: 14, scope: !10)
!21 = !DILocalVariable(name: "b", scope: !10, file: !1, line: 4, type: !13)
!22 = !DILocation(line: 4, column: 7, scope: !10)
!23 = !DILocation(line: 4, column: 11, scope: !10)
!24 = !DILocation(line: 4, column: 15, scope: !10)
!25 = !DILocation(line: 4, column: 13, scope: !10)
!26 = !DILocation(line: 5, column: 1, scope: !10)

; ModuleID = 'const_int_test'
source_filename = "const_int_test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spirv64-unknown-unknown"

@main_global = dso_local local_unnamed_addr global i32 0, align 4, !dbg !22

define dso_local i32 @main() !dbg !12 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 0, ptr %a, align 4
  call void @llvm.dbg.declare(metadata ptr %a, metadata !17, metadata !DIExpression()), !dbg !20
  ret i32 0, !dbg !21
}

declare void @llvm.dbg.declare(metadata, metadata, metadata)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "const_int_test.c", directory: "/tmp")
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{!"clang"}
!10 = distinct !DIGlobalVariable(name: "main_global", linkageName: "main_global", scope: !0, file: !1, line: 1, type: !11, isLocal: true, isDefinition: true)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !13, scopeLine: 1, unit: !0, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition)
!13 = !DISubroutineType(types: !14)
!14 = !{!11}
!17 = !DILocalVariable(name: "a", scope: !12, file: !1, line: 2, type: !18)
!18 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !11, name: "const int")
!20 = !DILocation(line: 2, column: 10, scope: !12)
!21 = !DILocation(line: 3, column: 3, scope: !12)
!22 = !DIGlobalVariableExpression(var: !10, expr: !DIExpression())

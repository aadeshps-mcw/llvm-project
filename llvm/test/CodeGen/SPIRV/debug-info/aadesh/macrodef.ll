; ModuleID = 'macrodef.c'
source_filename = "macrodef.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local global [5 x i32] zeroinitializer, align 16, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !30 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load i32, ptr @arr, align 16, !dbg !33
  ret i32 %0, !dbg !34
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!22, !23, !24, !25, !26, !27, !28}
!llvm.ident = !{!29}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "arr", scope: !2, file: !3, line: 3, type: !18, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, macros: !5, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "macrodef.c", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "0b5c9b95600d6e0ccd1a9b7d45f5f1b6")
!4 = !{!0}
!5 = !{!6, !9, !10, !11, !12, !13, !14, !15, !16, !17}
!6 = !DIMacroFile(file: !3, nodes: !7)
!7 = !{!8}
!8 = !DIMacro(type: DW_MACINFO_define, line: 1, name: "SIZE", value: "5")
!9 = !DIMacro(type: DW_MACINFO_define, name: "__STDC__", value: "1")
!10 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_HOSTED__", value: "1")
!11 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_VERSION__", value: "201710L")
!12 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_UTF_16__", value: "1")
!13 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_UTF_32__", value: "1")
!14 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_NOT_FOUND__", value: "0")
!15 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_FOUND__", value: "1")
!16 = !DIMacro(type: DW_MACINFO_define, name: "__STDC_EMBED_EMPTY__", value: "2")
!17 = !DIMacro(type: DW_MACINFO_define, name: "__GCC_HAVE_DWARF2_CFI_ASM", value: "1")
!18 = !DICompositeType(tag: DW_TAG_array_type, baseType: !19, size: 160, elements: !20)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !{!21}
!21 = !DISubrange(count: 5)
!22 = !{i32 7, !"Dwarf Version", i32 5}
!23 = !{i32 2, !"Debug Info Version", i32 3}
!24 = !{i32 1, !"wchar_size", i32 4}
!25 = !{i32 8, !"PIC Level", i32 2}
!26 = !{i32 7, !"PIE Level", i32 2}
!27 = !{i32 7, !"uwtable", i32 2}
!28 = !{i32 7, !"frame-pointer", i32 2}
!29 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!30 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 5, type: !31, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2)
!31 = !DISubroutineType(types: !32)
!32 = !{!19}
!33 = !DILocation(line: 6, column: 12, scope: !30)
!34 = !DILocation(line: 6, column: 5, scope: !30)

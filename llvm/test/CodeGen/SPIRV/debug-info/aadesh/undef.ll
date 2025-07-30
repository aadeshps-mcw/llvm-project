; ModuleID = 'undef.c'
source_filename = "undef.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !24 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  ret i32 0, !dbg !28
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!16, !17, !18, !19, !20, !21, !22}
!llvm.ident = !{!23}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, macros: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "undef.c", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "3c328b0f9df7407d0da43fe093695bba")
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
!23 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!24 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 4, type: !25, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !0)
!25 = !DISubroutineType(types: !26)
!26 = !{!27}
!27 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!28 = !DILocation(line: 5, column: 3, scope: !24)

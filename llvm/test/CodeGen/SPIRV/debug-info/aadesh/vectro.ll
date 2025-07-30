; ModuleID = 'vector.c'
source_filename = "vector.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(<4 x i32> noundef %a) #0 !dbg !10 {
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

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="128" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "vector.c", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "ef8e158df1fc70de8682dacb4637446e")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
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

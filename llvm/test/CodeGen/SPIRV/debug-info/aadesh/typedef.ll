; ModuleID = 'typedef.c'
source_filename = "typedef.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @square(i32 noundef %x) #0 !dbg !10 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
    #dbg_declare(ptr %x.addr, !16, !DIExpression(), !17)
  %0 = load i32, ptr %x.addr, align 4, !dbg !18
  %1 = load i32, ptr %x.addr, align 4, !dbg !19
  %mul = mul nsw i32 %0, %1, !dbg !20
  ret i32 %mul, !dbg !21
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !22 {
entry:
  %retval = alloca i32, align 4
  %val = alloca i32, align 4
  %result = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %val, !25, !DIExpression(), !26)
  store i32 7, ptr %val, align 4, !dbg !26
    #dbg_declare(ptr %result, !27, !DIExpression(), !28)
  %0 = load i32, ptr %val, align 4, !dbg !29
  %call = call i32 @square(i32 noundef %0), !dbg !30
  store i32 %call, ptr %result, align 4, !dbg !28
  %1 = load i32, ptr %result, align 4, !dbg !31
  ret i32 %1, !dbg !32
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "typedef.c", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "3fc004b3da8c70dfabbc37485328e2f9")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!10 = distinct !DISubprogram(name: "square", scope: !1, file: !1, line: 5, type: !11, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !13}
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "myint", file: !1, line: 3, baseType: !14)
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !{}
!16 = !DILocalVariable(name: "x", arg: 1, scope: !10, file: !1, line: 5, type: !13)
!17 = !DILocation(line: 5, column: 20, scope: !10)
!18 = !DILocation(line: 6, column: 12, scope: !10)
!19 = !DILocation(line: 6, column: 16, scope: !10)
!20 = !DILocation(line: 6, column: 14, scope: !10)
!21 = !DILocation(line: 6, column: 5, scope: !10)
!22 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !23, scopeLine: 9, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!23 = !DISubroutineType(types: !24)
!24 = !{!14}
!25 = !DILocalVariable(name: "val", scope: !22, file: !1, line: 10, type: !13)
!26 = !DILocation(line: 10, column: 11, scope: !22)
!27 = !DILocalVariable(name: "result", scope: !22, file: !1, line: 11, type: !13)
!28 = !DILocation(line: 11, column: 11, scope: !22)
!29 = !DILocation(line: 11, column: 27, scope: !22)
!30 = !DILocation(line: 11, column: 20, scope: !22)
!31 = !DILocation(line: 12, column: 12, scope: !22)
!32 = !DILocation(line: 12, column: 5, scope: !22)

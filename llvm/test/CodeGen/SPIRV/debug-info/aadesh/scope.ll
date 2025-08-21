; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i32 @foo(i32 noundef %0) local_unnamed_addr #0 !dbg !10 {
  tail call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !17
  %2 = add nsw i32 %0, 1, !dbg !18
  tail call void @llvm.dbg.value(metadata i32 %2, metadata !16, metadata !DIExpression()), !dbg !17
  ret i32 %2, !dbg !19
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local i32 @bar(i32 noundef %0) local_unnamed_addr #0 !dbg !20 {
  tail call void @llvm.dbg.value(metadata i32 %0, metadata !22, metadata !DIExpression()), !dbg !24
  %2 = shl nsw i32 %0, 1, !dbg !25
  tail call void @llvm.dbg.value(metadata i32 %2, metadata !23, metadata !DIExpression()), !dbg !24
  ret i32 %2, !dbg !26
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 !dbg !27 {
  tail call void @llvm.dbg.value(metadata i32 4, metadata !31, metadata !DIExpression()), !dbg !33
  tail call void @llvm.dbg.value(metadata i32 8, metadata !32, metadata !DIExpression()), !dbg !33
  ret i32 12, !dbg !34
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "Ubuntu clang version 18.1.3 (1ubuntu1)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/home/user/Ebin/llvm-project/llvm/test/CodeGen/SPIRV/debug-info", checksumkind: CSK_MD5, checksum: "f10967bf988e0df2d90961fe265242d5")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"debug-info-assignment-tracking", i1 true}
!9 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!10 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{!15, !16}
!15 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !1, line: 1, type: !13)
!16 = !DILocalVariable(name: "x", scope: !10, file: !1, line: 2, type: !13)
!17 = !DILocation(line: 0, scope: !10)
!18 = !DILocation(line: 2, column: 13, scope: !10)
!19 = !DILocation(line: 3, column: 3, scope: !10)
!20 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 6, type: !11, scopeLine: 6, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !21)
!21 = !{!22, !23}
!22 = !DILocalVariable(name: "b", arg: 1, scope: !20, file: !1, line: 6, type: !13)
!23 = !DILocalVariable(name: "y", scope: !20, file: !1, line: 7, type: !13)
!24 = !DILocation(line: 0, scope: !20)
!25 = !DILocation(line: 7, column: 13, scope: !20)
!26 = !DILocation(line: 8, column: 3, scope: !20)
!27 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !28, scopeLine: 11, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !30)
!28 = !DISubroutineType(types: !29)
!29 = !{!13}
!30 = !{!31, !32}
!31 = !DILocalVariable(name: "v1", scope: !27, file: !1, line: 12, type: !13)
!32 = !DILocalVariable(name: "v2", scope: !27, file: !1, line: 13, type: !13)
!33 = !DILocation(line: 0, scope: !27)
!34 = !DILocation(line: 14, column: 3, scope: !27)

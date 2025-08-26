; ModuleID = 'doublefunction.cpp'
source_filename = "doublefunction.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global_counter = dso_local global i32 0, align 4, !dbg !0

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z7add_onei(i32 noundef %value) #0 !dbg !14 {
entry:
  %value.addr = alloca i32, align 4
  store i32 %value, ptr %value.addr, align 4
    #dbg_declare(ptr %value.addr, !18, !DIExpression(), !19)
  %0 = load i32, ptr %value.addr, align 4, !dbg !20
  %add = add nsw i32 %0, 1, !dbg !21
  ret i32 %add, !dbg !22
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #1 !dbg !23 {
entry:
  %retval = alloca i32, align 4
  %local_sum = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %local_sum, !26, !DIExpression(), !27)
  store i32 0, ptr %local_sum, align 4, !dbg !27
    #dbg_declare(ptr %i, !28, !DIExpression(), !30)
  store i32 0, ptr %i, align 4, !dbg !30
  br label %for.cond, !dbg !31

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4, !dbg !32
  %cmp = icmp slt i32 %0, 5, !dbg !34
  br i1 %cmp, label %for.body, label %for.end, !dbg !35

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4, !dbg !36
  %call = call noundef i32 @_Z7add_onei(i32 noundef %1), !dbg !38
  %2 = load i32, ptr %local_sum, align 4, !dbg !39
  %add = add nsw i32 %2, %call, !dbg !39
  store i32 %add, ptr %local_sum, align 4, !dbg !39
  %3 = load i32, ptr @global_counter, align 4, !dbg !40
  %add1 = add nsw i32 %3, 1, !dbg !40
  store i32 %add1, ptr @global_counter, align 4, !dbg !40
  br label %for.inc, !dbg !41

for.inc:                                          ; preds = %for.body
  %4 = load i32, ptr %i, align 4, !dbg !42
  %inc = add nsw i32 %4, 1, !dbg !42
  store i32 %inc, ptr %i, align 4, !dbg !42
  br label %for.cond, !dbg !43, !llvm.loop !44

for.end:                                          ; preds = %for.cond
  %5 = load i32, ptr %local_sum, align 4, !dbg !47
  ret i32 %5, !dbg !48
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!6, !7, !8, !9, !10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "global_counter", scope: !2, file: !3, line: 8, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "doublefunction.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "6815d8262296403adb533639c2988e32")
!4 = !{!0}
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!6 = !{i32 7, !"Dwarf Version", i32 5}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 8, !"PIC Level", i32 2}
!10 = !{i32 7, !"PIE Level", i32 2}
!11 = !{i32 7, !"uwtable", i32 2}
!12 = !{i32 7, !"frame-pointer", i32 2}
!13 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!14 = distinct !DISubprogram(name: "add_one", linkageName: "_Z7add_onei", scope: !3, file: !3, line: 11, type: !15, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !17)
!15 = !DISubroutineType(types: !16)
!16 = !{!5, !5}
!17 = !{}
!18 = !DILocalVariable(name: "value", arg: 1, scope: !14, file: !3, line: 11, type: !5)
!19 = !DILocation(line: 11, column: 17, scope: !14)
!20 = !DILocation(line: 12, column: 12, scope: !14)
!21 = !DILocation(line: 12, column: 18, scope: !14)
!22 = !DILocation(line: 12, column: 5, scope: !14)
!23 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 16, type: !24, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !17)
!24 = !DISubroutineType(types: !25)
!25 = !{!5}
!26 = !DILocalVariable(name: "local_sum", scope: !23, file: !3, line: 18, type: !5)
!27 = !DILocation(line: 18, column: 9, scope: !23)
!28 = !DILocalVariable(name: "i", scope: !29, file: !3, line: 21, type: !5)
!29 = distinct !DILexicalBlock(scope: !23, file: !3, line: 21, column: 5)
!30 = !DILocation(line: 21, column: 14, scope: !29)
!31 = !DILocation(line: 21, column: 10, scope: !29)
!32 = !DILocation(line: 21, column: 21, scope: !33)
!33 = distinct !DILexicalBlock(scope: !29, file: !3, line: 21, column: 5)
!34 = !DILocation(line: 21, column: 23, scope: !33)
!35 = !DILocation(line: 21, column: 5, scope: !29)
!36 = !DILocation(line: 22, column: 30, scope: !37)
!37 = distinct !DILexicalBlock(scope: !33, file: !3, line: 21, column: 33)
!38 = !DILocation(line: 22, column: 22, scope: !37)
!39 = !DILocation(line: 22, column: 19, scope: !37)
!40 = !DILocation(line: 23, column: 24, scope: !37)
!41 = !DILocation(line: 24, column: 5, scope: !37)
!42 = !DILocation(line: 21, column: 28, scope: !33)
!43 = !DILocation(line: 21, column: 5, scope: !33)
!44 = distinct !{!44, !35, !45, !46}
!45 = !DILocation(line: 24, column: 5, scope: !29)
!46 = !{!"llvm.loop.mustprogress"}
!47 = !DILocation(line: 26, column: 12, scope: !23)
!48 = !DILocation(line: 26, column: 5, scope: !23)

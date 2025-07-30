; ModuleID = 'import.cpp'
source_filename = "import.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_ZN4myns9double_itEi(i32 noundef %x) #0 !dbg !13 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, ptr %x.addr, align 4
    #dbg_declare(ptr %x.addr, !19, !DIExpression(), !20)
  %0 = load i32, ptr %x.addr, align 4, !dbg !21
  %mul = mul nsw i32 %0, 2, !dbg !22
  ret i32 %mul, !dbg !23
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #1 !dbg !24 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %a, !27, !DIExpression(), !28)
  store i32 5, ptr %a, align 4, !dbg !28
    #dbg_declare(ptr %b, !29, !DIExpression(), !30)
  %0 = load i32, ptr %a, align 4, !dbg !31
  %call = call noundef i32 @_ZN4myns9double_itEi(i32 noundef %0), !dbg !32
  store i32 %call, ptr %b, align 4, !dbg !30
  %1 = load i32, ptr %b, align 4, !dbg !33
  ret i32 %1, !dbg !34
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!5, !6, !7, !8, !9, !10, !11}
!llvm.ident = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, imports: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "import.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "6783aecc6b63eff1212b8259efc649ff")
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
!12 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!13 = distinct !DISubprogram(name: "double_it", linkageName: "_ZN4myns9double_itEi", scope: !4, file: !14, line: 5, type: !15, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!14 = !DIFile(filename: "./ns.h", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "3b90ea49936eec76699a09a57811a4b3")
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

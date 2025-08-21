; ModuleID = 'inheritance.cpp'
source_filename = "inheritance.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Derived = type { %class.Base, i32 }
%class.Base = type { i32 }

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !10 {
entry:
  %retval = alloca i32, align 4
  %d = alloca %class.Derived, align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %d, !15, !DIExpression(), !23)
  %x = getelementptr inbounds nuw %class.Base, ptr %d, i32 0, i32 0, !dbg !24
  store i32 10, ptr %x, align 4, !dbg !25
  %y = getelementptr inbounds nuw %class.Derived, ptr %d, i32 0, i32 1, !dbg !26
  store i32 20, ptr %y, align 4, !dbg !27
  %x1 = getelementptr inbounds nuw %class.Base, ptr %d, i32 0, i32 0, !dbg !28
  %0 = load i32, ptr %x1, align 4, !dbg !28
  %y2 = getelementptr inbounds nuw %class.Derived, ptr %d, i32 0, i32 1, !dbg !29
  %1 = load i32, ptr %y2, align 4, !dbg !29
  %add = add nsw i32 %0, %1, !dbg !30
  ret i32 %add, !dbg !31
}

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "inheritance.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "3737b265027a322ab186c44fd08d1937")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
!9 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 12, type: !11, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!11 = !DISubroutineType(types: !12)
!12 = !{!13}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{}
!15 = !DILocalVariable(name: "d", scope: !10, file: !1, line: 13, type: !16)
!16 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Derived", file: !1, line: 7, size: 64, flags: DIFlagTypePassByValue, elements: !17, identifier: "_ZTS7Derived")
!17 = !{!18, !22}
!18 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !16, baseType: !19, flags: DIFlagPublic, extraData: i32 0)
!19 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base", file: !1, line: 2, size: 32, flags: DIFlagTypePassByValue, elements: !20, identifier: "_ZTS4Base")
!20 = !{!21}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !19, file: !1, line: 4, baseType: !13, size: 32, flags: DIFlagPublic)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !16, file: !1, line: 9, baseType: !13, size: 32, offset: 32, flags: DIFlagPublic)
!23 = !DILocation(line: 13, column: 13, scope: !10)
!24 = !DILocation(line: 14, column: 7, scope: !10)
!25 = !DILocation(line: 14, column: 9, scope: !10)
!26 = !DILocation(line: 15, column: 7, scope: !10)
!27 = !DILocation(line: 15, column: 9, scope: !10)
!28 = !DILocation(line: 16, column: 14, scope: !10)
!29 = !DILocation(line: 16, column: 20, scope: !10)
!30 = !DILocation(line: 16, column: 16, scope: !10)
!31 = !DILocation(line: 16, column: 5, scope: !10)

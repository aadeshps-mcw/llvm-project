; ModuleID = 'composite.cpp'
source_filename = "composite.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Point = type { i32, i32 }

@p = dso_local global %struct.Point zeroinitializer, align 4, !dbg !0

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !18 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 10, ptr @p, align 4, !dbg !21
  store i32 20, ptr getelementptr inbounds nuw (%struct.Point, ptr @p, i32 0, i32 1), align 4, !dbg !22
  ret i32 0, !dbg !23
}

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13, !14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "p", scope: !2, file: !3, line: 7, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "composite.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "b6f1e544a7e530a0b192fcc5f36f832c")
!4 = !{!0}
!5 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Point", file: !3, line: 2, size: 64, flags: DIFlagTypePassByValue, elements: !6, identifier: "_ZTS5Point")
!6 = !{!7, !9}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !5, file: !3, line: 3, baseType: !8, size: 32)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !5, file: !3, line: 4, baseType: !8, size: 32, offset: 32)
!10 = !{i32 7, !"Dwarf Version", i32 5}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 8, !"PIC Level", i32 2}
!14 = !{i32 7, !"PIE Level", i32 2}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{i32 7, !"frame-pointer", i32 2}
!17 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!18 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 8, type: !19, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2)
!19 = !DISubroutineType(types: !20)
!20 = !{!8}
!21 = !DILocation(line: 9, column: 7, scope: !18)
!22 = !DILocation(line: 10, column: 7, scope: !18)
!23 = !DILocation(line: 11, column: 3, scope: !18)

; ModuleID = 'template.cpp'
source_filename = "template.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.FixedArray = type { [10 x i32] }

@fa_global = dso_local global %struct.FixedArray zeroinitializer, align 4, !dbg !0

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #0 !dbg !23 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  ret i32 0, !dbg !26
}

attributes #0 = { mustprogress noinline norecurse nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!15, !16, !17, !18, !19, !20, !21}
!llvm.ident = !{!22}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "fa_global", scope: !2, file: !3, line: 8, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "template.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "24370309ef49dac4eabc32287d6ffde6")
!4 = !{!0}
!5 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "FixedArray<int, 10>", file: !3, line: 4, size: 320, flags: DIFlagTypePassByValue, elements: !6, templateParams: !12, identifier: "_ZTS10FixedArrayIiLi10EE")
!6 = !{!7}
!7 = !DIDerivedType(tag: DW_TAG_member, name: "data", scope: !5, file: !3, line: 5, baseType: !8, size: 320)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 320, elements: !10)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{!11}
!11 = !DISubrange(count: 10)
!12 = !{!13, !14}
!13 = !DITemplateTypeParameter(name: "T", type: !9)
!14 = !DITemplateValueParameter(name: "N", type: !9, value: i32 10)
!15 = !{i32 7, !"Dwarf Version", i32 5}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, !"wchar_size", i32 4}
!18 = !{i32 8, !"PIC Level", i32 2}
!19 = !{i32 7, !"PIE Level", i32 2}
!20 = !{i32 7, !"uwtable", i32 2}
!21 = !{i32 7, !"frame-pointer", i32 2}
!22 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!23 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 10, type: !24, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2)
!24 = !DISubroutineType(types: !25)
!25 = !{!9}
!26 = !DILocation(line: 11, column: 3, scope: !23)

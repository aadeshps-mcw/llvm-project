; ModuleID = 'ptrtomember.cpp'
source_filename = "ptrtomember.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32 }

@ptr = dso_local global i64 0, align 8, !dbg !0
@__const._Z3usev.s = private unnamed_addr constant %struct.S { i32 42 }, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z3usev() #0 !dbg !18 {
entry:
  %s = alloca %struct.S, align 4
    #dbg_declare(ptr %s, !22, !DIExpression(), !23)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %s, ptr align 4 @__const._Z3usev.s, i64 4, i1 false), !dbg !23
  %0 = load i64, ptr @ptr, align 8, !dbg !24
  %memptr.offset = getelementptr inbounds i8, ptr %s, i64 %0, !dbg !25
  %1 = load i32, ptr %memptr.offset, align 4, !dbg !25
  ret i32 %1, !dbg !26
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13, !14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ptr", scope: !2, file: !3, line: 7, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "ptrtomember.cpp", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "10a200d2bc605ebb1abe852bc777c8ad")
!4 = !{!0}
!5 = !DIDerivedType(tag: DW_TAG_ptr_to_member_type, baseType: !6, size: 64, extraData: !7)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S", file: !3, line: 3, size: 32, flags: DIFlagTypePassByValue, elements: !8, identifier: "_ZTS1S")
!8 = !{!9}
!9 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !7, file: !3, line: 4, baseType: !6, size: 32)
!10 = !{i32 7, !"Dwarf Version", i32 5}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 8, !"PIC Level", i32 2}
!14 = !{i32 7, !"PIE Level", i32 2}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{i32 7, !"frame-pointer", i32 2}
!17 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!18 = distinct !DISubprogram(name: "use", linkageName: "_Z3usev", scope: !3, file: !3, line: 9, type: !19, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !21)
!19 = !DISubroutineType(types: !20)
!20 = !{!6}
!21 = !{}
!22 = !DILocalVariable(name: "s", scope: !18, file: !3, line: 10, type: !7)
!23 = !DILocation(line: 10, column: 5, scope: !18)
!24 = !DILocation(line: 11, column: 13, scope: !18)
!25 = !DILocation(line: 11, column: 11, scope: !18)
!26 = !DILocation(line: 11, column: 3, scope: !18)

; ModuleID = 'typearray.c'
source_filename = "typearray.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.main.local_array = private unnamed_addr constant [2 x i32] [i32 1, i32 2], align 4
@global_array = dso_local global [4 x [3 x i32]] zeroinitializer, align 16, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !18 {
entry:
  %retval = alloca i32, align 4
  %local_array = alloca [2 x i32], align 4
  store i32 0, ptr %retval, align 4
    #dbg_declare(ptr %local_array, !22, !DIExpression(), !26)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %local_array, ptr align 4 @__const.main.local_array, i64 8, i1 false), !dbg !26
  %arrayidx = getelementptr inbounds [2 x i32], ptr %local_array, i64 0, i64 0, !dbg !27
  %0 = load i32, ptr %arrayidx, align 4, !dbg !27
  ret i32 %0, !dbg !28
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13, !14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "global_array", scope: !2, file: !3, line: 2, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "typearray.c", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/debug-info/aadesh", checksumkind: CSK_MD5, checksum: "f6d835f772c15d7b5802178dfbbbe886")
!4 = !{!0}
!5 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 384, elements: !7)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!8, !9}
!8 = !DISubrange(count: 4)
!9 = !DISubrange(count: 3)
!10 = !{i32 7, !"Dwarf Version", i32 5}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 8, !"PIC Level", i32 2}
!14 = !{i32 7, !"PIE Level", i32 2}
!15 = !{i32 7, !"uwtable", i32 2}
!16 = !{i32 7, !"frame-pointer", i32 2}
!17 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!18 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 4, type: !19, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !21)
!19 = !DISubroutineType(types: !20)
!20 = !{!6}
!21 = !{}
!22 = !DILocalVariable(name: "local_array", scope: !18, file: !3, line: 5, type: !23)
!23 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 64, elements: !24)
!24 = !{!25}
!25 = !DISubrange(count: 2)
!26 = !DILocation(line: 5, column: 9, scope: !18)
!27 = !DILocation(line: 6, column: 12, scope: !18)
!28 = !DILocation(line: 6, column: 5, scope: !18)

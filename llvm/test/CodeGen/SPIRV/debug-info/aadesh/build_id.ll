; ModuleID = 'debugidentifier.cl'
source_filename = "debugidentifier.cl"
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-G1"
target triple = "spir"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local spir_kernel void @test_kernel(ptr addrspace(1) noundef align 4 %A) #0 !dbg !8 !kernel_arg_addr_space !14 !kernel_arg_access_qual !15 !kernel_arg_type !16 !kernel_arg_base_type !16 !kernel_arg_type_qual !17 {
entry:
  %A.addr = alloca ptr addrspace(1), align 4
  %x = alloca i32, align 4
  store ptr addrspace(1) %A, ptr %A.addr, align 4
    #dbg_declare(ptr %A.addr, !18, !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef), !19)
    #dbg_declare(ptr %x, !20, !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef), !21)
  %0 = load ptr addrspace(1), ptr %A.addr, align 4, !dbg !22
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i32 0, !dbg !22
  %1 = load i32, ptr addrspace(1) %arrayidx, align 4, !dbg !22
  store i32 %1, ptr %x, align 4, !dbg !21
  ret void, !dbg !23
}

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5}
!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "debugidentifier.cl", directory: "/media/aadeshps/a7c95f0a-1803-444e-8334-da85467b1a4d/llvm-project/llvm/test/CodeGen/SPIRV/aadesh", checksumkind: CSK_MD5, checksum: "09884249cd7e00d8e17855e76ce00618")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{i32 2, i32 0}
!7 = !{!"clang version 21.0.0git (git@github.com:aadeshps-mcw/llvm-project.git c9095aa3103460c967fd5ee5dcc695284793ef3c)"}
!8 = distinct !DISubprogram(name: "test_kernel", scope: !1, file: !1, line: 2, type: !9, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 32, dwarfAddressSpace: 1)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{}
!14 = !{i32 1}
!15 = !{!"none"}
!16 = !{!"int*"}
!17 = !{!""}
!18 = !DILocalVariable(name: "A", arg: 1, scope: !8, file: !1, line: 2, type: !11)
!19 = !DILocation(line: 2, column: 41, scope: !8)
!20 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 3, type: !12)
!21 = !DILocation(line: 3, column: 7, scope: !8)
!22 = !DILocation(line: 3, column: 11, scope: !8)
!23 = !DILocation(line: 4, column: 1, scope: !8)

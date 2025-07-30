; ModuleID = 'debug_build_storage.ll'
source_filename = "debug_build_storage.cl"
target triple = "spir64-unknown-unknown"

; Simple dummy OpenCL kernel
define dso_local spir_kernel void @add_kernel(i32 addrspace(1)* %a, i32 addrspace(1)* %b, i32 addrspace(1)* %res) !dbg !8 {
entry:
  ret void, !dbg !10
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(
  language: DW_LANG_C99,
  file: !1,
  producer: "clang version test",
  isOptimized: false,
  runtimeVersion: 0,
  emissionKind: FullDebug,
  splitDebugFilename: "debug_storage_path.spv", ; <--- Storage path
  dwoId: 1234567890,                             ; <--- Build ID
  enums: !2,
  retainedTypes: !2,
  globals: !2,
  imports: !2
)

!1 = !DIFile(filename: "debug_build_storage.cl", directory: "/path/to/source")

!2 = !{}

!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version test"}

!8 = distinct !DISubprogram(name: "add_kernel", scope: !1, file: !1, line: 1,
                            type: !9, unit: !0, spFlags: DISPFlagDefinition,
                            retainedNodes: !2)

!9 = !DISubroutineType(types: !2)

!10 = !DILocation(line: 2, column: 1, scope: !8)

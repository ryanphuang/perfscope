; ModuleID = 'mapper.core.1.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@id = internal global i32 2, align 4

define i32 @foo(i32 %c) nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 %c, i32* %c.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %c.addr}, metadata !24), !dbg !25
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !26), !dbg !29
  store i32 0, i32* %i, align 4, !dbg !30
  br label %for.cond, !dbg !30

for.cond:                                         ; preds = %for.inc4, %entry
  %0 = load i32* %i, align 4, !dbg !30
  %cmp = icmp slt i32 %0, 10, !dbg !30
  br i1 %cmp, label %for.body, label %for.end6, !dbg !30

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.declare(metadata !{i32* %j}, metadata !31), !dbg !34
  store i32 0, i32* %j, align 4, !dbg !35
  br label %for.cond1, !dbg !35

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32* %j, align 4, !dbg !35
  %cmp2 = icmp slt i32 %1, 5, !dbg !35
  br i1 %cmp2, label %for.body3, label %for.end, !dbg !35

for.body3:                                        ; preds = %for.cond1
  %2 = load i32* %i, align 4, !dbg !36
  %3 = load i32* %c.addr, align 4, !dbg !36
  %add = add nsw i32 %3, %2, !dbg !36
  store i32 %add, i32* %c.addr, align 4, !dbg !36
  %4 = load i32* %j, align 4, !dbg !38
  %5 = load i32* %c.addr, align 4, !dbg !38
  %mul = mul nsw i32 %5, %4, !dbg !38
  store i32 %mul, i32* %c.addr, align 4, !dbg !38
  br label %for.inc, !dbg !39

for.inc:                                          ; preds = %for.body3
  %6 = load i32* %j, align 4, !dbg !40
  %inc = add nsw i32 %6, 1, !dbg !40
  store i32 %inc, i32* %j, align 4, !dbg !40
  br label %for.cond1, !dbg !40

for.end:                                          ; preds = %for.cond1
  br label %for.inc4, !dbg !41

for.inc4:                                         ; preds = %for.end
  %7 = load i32* %i, align 4, !dbg !42
  %inc5 = add nsw i32 %7, 1, !dbg !42
  store i32 %inc5, i32* %i, align 4, !dbg !42
  br label %for.cond, !dbg !42

for.end6:                                         ; preds = %for.cond
  %8 = load i32* %c.addr, align 4, !dbg !43
  %inc7 = add nsw i32 %8, 1, !dbg !43
  store i32 %inc7, i32* %c.addr, align 4, !dbg !43
  %9 = load i32* %c.addr, align 4, !dbg !44
  %cmp8 = icmp sgt i32 %9, 10, !dbg !44
  br i1 %cmp8, label %if.then, label %if.else, !dbg !44

if.then:                                          ; preds = %for.end6
  %10 = load i32* %c.addr, align 4, !dbg !45
  %11 = load i32* %c.addr, align 4, !dbg !45
  %mul9 = mul nsw i32 %11, %10, !dbg !45
  store i32 %mul9, i32* %c.addr, align 4, !dbg !45
  %12 = load i32* %c.addr, align 4, !dbg !47
  %inc10 = add nsw i32 %12, 1, !dbg !47
  store i32 %inc10, i32* %c.addr, align 4, !dbg !47
  %13 = load i32* %c.addr, align 4, !dbg !48
  store i32 %13, i32* %retval, !dbg !48
  br label %return, !dbg !48

if.else:                                          ; preds = %for.end6
  %14 = load i32* @id, align 4, !dbg !49
  %inc11 = add nsw i32 %14, 1, !dbg !49
  store i32 %inc11, i32* @id, align 4, !dbg !49
  %15 = load i32* @id, align 4, !dbg !51
  %cmp12 = icmp sgt i32 %15, 10, !dbg !51
  br i1 %cmp12, label %if.then13, label %if.else15, !dbg !51

if.then13:                                        ; preds = %if.else
  %16 = load i32* %c.addr, align 4, !dbg !52
  %inc14 = add nsw i32 %16, 1, !dbg !52
  store i32 %inc14, i32* %c.addr, align 4, !dbg !52
  br label %if.end, !dbg !54

if.else15:                                        ; preds = %if.else
  %17 = load i32* %c.addr, align 4, !dbg !55
  %dec = add nsw i32 %17, -1, !dbg !55
  store i32 %dec, i32* %c.addr, align 4, !dbg !55
  br label %if.end

if.end:                                           ; preds = %if.else15, %if.then13
  br label %if.end16

if.end16:                                         ; preds = %if.end
  call void @llvm.dbg.declare(metadata !{i32* %x}, metadata !57), !dbg !58
  %18 = load i32* %c.addr, align 4, !dbg !59
  %19 = load i32* @id, align 4, !dbg !59
  %mul17 = mul nsw i32 %18, %19, !dbg !59
  store i32 %mul17, i32* %x, align 4, !dbg !59
  %20 = load i32* %x, align 4, !dbg !60
  %21 = load i32* %x, align 4, !dbg !60
  %mul18 = mul nsw i32 %21, %20, !dbg !60
  store i32 %mul18, i32* %x, align 4, !dbg !60
  %22 = load i32* %x, align 4, !dbg !61
  store i32 %22, i32* %retval, !dbg !61
  br label %return, !dbg !61

return:                                           ; preds = %if.end16, %if.then
  %23 = load i32* %retval, !dbg !62
  ret i32 %23, !dbg !62
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

define i32 @larger(i32 %x, i32 %y) nounwind uwtable {
entry:
  %x.addr = alloca i32, align 4
  %y.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %x.addr}, metadata !63), !dbg !64
  store i32 %y, i32* %y.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %y.addr}, metadata !65), !dbg !66
  %0 = load i32* %x.addr, align 4, !dbg !67
  %1 = load i32* %y.addr, align 4, !dbg !67
  %cmp = icmp sgt i32 %0, %1, !dbg !67
  %conv = zext i1 %cmp to i32, !dbg !67
  ret i32 %conv, !dbg !67
}

define i32 @bar() nounwind uwtable {
entry:
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !69), !dbg !71
  call void @llvm.dbg.declare(metadata !{i32* %sum}, metadata !72), !dbg !73
  store i32 0, i32* %sum, align 4, !dbg !74
  store i32 1, i32* %i, align 4, !dbg !75
  br label %for.cond, !dbg !75

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !75
  %cmp = icmp slt i32 %0, 10, !dbg !75
  br i1 %cmp, label %for.body, label %for.end, !dbg !75

for.body:                                         ; preds = %for.cond
  %1 = load i32* %i, align 4, !dbg !77
  %2 = load i32* %sum, align 4, !dbg !77
  %add = add nsw i32 %2, %1, !dbg !77
  store i32 %add, i32* %sum, align 4, !dbg !77
  br label %for.inc, !dbg !79

for.inc:                                          ; preds = %for.body
  %3 = load i32* %i, align 4, !dbg !80
  %inc = add nsw i32 %3, 1, !dbg !80
  store i32 %inc, i32* %i, align 4, !dbg !80
  br label %for.cond, !dbg !80

for.end:                                          ; preds = %for.cond
  %4 = load i32* %sum, align 4, !dbg !81
  ret i32 %4, !dbg !81
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @bar(), !dbg !82
  %call1 = call i32 @foo(i32 %call), !dbg !82
  %0 = load i32* %retval, !dbg !84
  ret i32 %0, !dbg !84
}

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"mapper.core.1.c", metadata !"/home/ryan/project/perfscope/test/cases", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !21} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12, metadata !15, metadata !18}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"foo", metadata !"foo", metadata !"", metadata !6, i32 5, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @foo, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"mapper.core.1.c", metadata !"/home/ryan/project/perfscope/test/cases", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 720942, i32 0, metadata !6, metadata !"larger", metadata !"larger", metadata !"", metadata !6, i32 32, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32, i32)* @larger, null, null, metadata !13} ; [ DW_TAG_subprogram ]
!13 = metadata !{metadata !14}
!14 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!15 = metadata !{i32 720942, i32 0, metadata !6, metadata !"bar", metadata !"bar", metadata !"", metadata !6, i32 37, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @bar, null, null, metadata !16} ; [ DW_TAG_subprogram ]
!16 = metadata !{metadata !17}
!17 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!18 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 47, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @main, null, null, metadata !19} ; [ DW_TAG_subprogram ]
!19 = metadata !{metadata !20}
!20 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!21 = metadata !{metadata !22}
!22 = metadata !{metadata !23}
!23 = metadata !{i32 720948, i32 0, null, metadata !"id", metadata !"id", metadata !"", metadata !6, i32 2, metadata !9, i32 1, i32 1, i32* @id} ; [ DW_TAG_variable ]
!24 = metadata !{i32 721153, metadata !5, metadata !"c", metadata !6, i32 16777220, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!25 = metadata !{i32 4, i32 13, metadata !5, null}
!26 = metadata !{i32 721152, metadata !27, metadata !"i", metadata !6, i32 7, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!27 = metadata !{i32 720907, metadata !28, i32 7, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!28 = metadata !{i32 720907, metadata !5, i32 5, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!29 = metadata !{i32 7, i32 14, metadata !27, null}
!30 = metadata !{i32 7, i32 19, metadata !27, null}
!31 = metadata !{i32 721152, metadata !32, metadata !"j", metadata !6, i32 8, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!32 = metadata !{i32 720907, metadata !33, i32 8, i32 9, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!33 = metadata !{i32 720907, metadata !27, i32 7, i32 34, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!34 = metadata !{i32 8, i32 18, metadata !32, null}
!35 = metadata !{i32 8, i32 23, metadata !32, null}
!36 = metadata !{i32 9, i32 13, metadata !37, null}
!37 = metadata !{i32 720907, metadata !32, i32 8, i32 37, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!38 = metadata !{i32 10, i32 13, metadata !37, null}
!39 = metadata !{i32 11, i32 9, metadata !37, null}
!40 = metadata !{i32 8, i32 32, metadata !32, null}
!41 = metadata !{i32 12, i32 5, metadata !33, null}
!42 = metadata !{i32 7, i32 29, metadata !27, null}
!43 = metadata !{i32 13, i32 5, metadata !28, null}
!44 = metadata !{i32 14, i32 5, metadata !28, null}
!45 = metadata !{i32 15, i32 9, metadata !46, null}
!46 = metadata !{i32 720907, metadata !28, i32 14, i32 17, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!47 = metadata !{i32 16, i32 9, metadata !46, null}
!48 = metadata !{i32 17, i32 9, metadata !46, null}
!49 = metadata !{i32 20, i32 9, metadata !50, null}
!50 = metadata !{i32 720907, metadata !28, i32 19, i32 10, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!51 = metadata !{i32 21, i32 9, metadata !50, null}
!52 = metadata !{i32 22, i32 13, metadata !53, null}
!53 = metadata !{i32 720907, metadata !50, i32 21, i32 22, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!54 = metadata !{i32 23, i32 9, metadata !53, null}
!55 = metadata !{i32 25, i32 13, metadata !56, null}
!56 = metadata !{i32 720907, metadata !50, i32 24, i32 14, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!57 = metadata !{i32 721152, metadata !28, metadata !"x", metadata !6, i32 28, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!58 = metadata !{i32 28, i32 9, metadata !28, null}
!59 = metadata !{i32 28, i32 19, metadata !28, null}
!60 = metadata !{i32 29, i32 5, metadata !28, null}
!61 = metadata !{i32 30, i32 5, metadata !28, null}
!62 = metadata !{i32 31, i32 1, metadata !28, null}
!63 = metadata !{i32 721153, metadata !12, metadata !"x", metadata !6, i32 16777248, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!64 = metadata !{i32 32, i32 16, metadata !12, null}
!65 = metadata !{i32 721153, metadata !12, metadata !"y", metadata !6, i32 33554464, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!66 = metadata !{i32 32, i32 23, metadata !12, null}
!67 = metadata !{i32 32, i32 28, metadata !68, null}
!68 = metadata !{i32 720907, metadata !12, i32 32, i32 26, metadata !6, i32 9} ; [ DW_TAG_lexical_block ]
!69 = metadata !{i32 721152, metadata !70, metadata !"i", metadata !6, i32 38, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!70 = metadata !{i32 720907, metadata !15, i32 37, i32 1, metadata !6, i32 10} ; [ DW_TAG_lexical_block ]
!71 = metadata !{i32 38, i32 9, metadata !70, null}
!72 = metadata !{i32 721152, metadata !70, metadata !"sum", metadata !6, i32 39, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!73 = metadata !{i32 39, i32 9, metadata !70, null}
!74 = metadata !{i32 39, i32 16, metadata !70, null}
!75 = metadata !{i32 40, i32 10, metadata !76, null}
!76 = metadata !{i32 720907, metadata !70, i32 40, i32 5, metadata !6, i32 11} ; [ DW_TAG_lexical_block ]
!77 = metadata !{i32 41, i32 8, metadata !78, null}
!78 = metadata !{i32 720907, metadata !76, i32 40, i32 30, metadata !6, i32 12} ; [ DW_TAG_lexical_block ]
!79 = metadata !{i32 42, i32 5, metadata !78, null}
!80 = metadata !{i32 40, i32 25, metadata !76, null}
!81 = metadata !{i32 43, i32 5, metadata !70, null}
!82 = metadata !{i32 48, i32 9, metadata !83, null}
!83 = metadata !{i32 720907, metadata !18, i32 47, i32 1, metadata !6, i32 13} ; [ DW_TAG_lexical_block ]
!84 = metadata !{i32 49, i32 1, metadata !83, null}

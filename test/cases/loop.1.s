; ModuleID = 'test/cases/loop.1.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %c) nounwind uwtable {
entry:
  %c.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %c, i32* %c.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %c.addr}, metadata !21), !dbg !22
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !23), !dbg !26
  store i32 0, i32* %i, align 4, !dbg !27
  br label %for.cond, !dbg !27

for.cond:                                         ; preds = %for.inc4, %entry
  %0 = load i32* %i, align 4, !dbg !27
  %cmp = icmp slt i32 %0, 10, !dbg !27
  br i1 %cmp, label %for.body, label %for.end6, !dbg !27

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.declare(metadata !{i32* %j}, metadata !28), !dbg !31
  store i32 0, i32* %j, align 4, !dbg !32
  br label %for.cond1, !dbg !32

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32* %j, align 4, !dbg !32
  %cmp2 = icmp slt i32 %1, 5, !dbg !32
  br i1 %cmp2, label %for.body3, label %for.end, !dbg !32

for.body3:                                        ; preds = %for.cond1
  %2 = load i32* %i, align 4, !dbg !33
  %3 = load i32* %c.addr, align 4, !dbg !33
  %add = add nsw i32 %3, %2, !dbg !33
  store i32 %add, i32* %c.addr, align 4, !dbg !33
  %4 = load i32* %j, align 4, !dbg !35
  %5 = load i32* %c.addr, align 4, !dbg !35
  %mul = mul nsw i32 %5, %4, !dbg !35
  store i32 %mul, i32* %c.addr, align 4, !dbg !35
  br label %for.inc, !dbg !36

for.inc:                                          ; preds = %for.body3
  %6 = load i32* %j, align 4, !dbg !37
  %inc = add nsw i32 %6, 1, !dbg !37
  store i32 %inc, i32* %j, align 4, !dbg !37
  br label %for.cond1, !dbg !37

for.end:                                          ; preds = %for.cond1
  br label %for.inc4, !dbg !38

for.inc4:                                         ; preds = %for.end
  %7 = load i32* %i, align 4, !dbg !39
  %inc5 = add nsw i32 %7, 1, !dbg !39
  store i32 %inc5, i32* %i, align 4, !dbg !39
  br label %for.cond, !dbg !39

for.end6:                                         ; preds = %for.cond
  %8 = load i32* %c.addr, align 4, !dbg !40
  %inc7 = add nsw i32 %8, 1, !dbg !40
  store i32 %inc7, i32* %c.addr, align 4, !dbg !40
  %9 = load i32* %c.addr, align 4, !dbg !41
  ret i32 %9, !dbg !41
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

define i32 @add() nounwind uwtable {
entry:
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !42), !dbg !44
  call void @llvm.dbg.declare(metadata !{i32* %sum}, metadata !45), !dbg !46
  store i32 0, i32* %sum, align 4, !dbg !47
  store i32 1, i32* %i, align 4, !dbg !48
  br label %for.cond, !dbg !48

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !48
  %1 = load i32* %i, align 4, !dbg !48
  %mul = mul nsw i32 %0, %1, !dbg !48
  %cmp = icmp slt i32 %mul, 100, !dbg !48
  br i1 %cmp, label %for.body, label %for.end, !dbg !48

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !50
  %3 = load i32* %sum, align 4, !dbg !50
  %add = add nsw i32 %3, %2, !dbg !50
  store i32 %add, i32* %sum, align 4, !dbg !50
  br label %for.inc, !dbg !52

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !53
  %inc = add nsw i32 %4, 1, !dbg !53
  store i32 %inc, i32* %i, align 4, !dbg !53
  br label %for.cond, !dbg !53

for.end:                                          ; preds = %for.cond
  %5 = load i32* %sum, align 4, !dbg !54
  ret i32 %5, !dbg !54
}

define i32 @mul(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %product = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %n.addr}, metadata !55), !dbg !56
  call void @llvm.dbg.declare(metadata !{i32* %product}, metadata !57), !dbg !59
  store i32 1, i32* %product, align 4, !dbg !60
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !61), !dbg !63
  store i32 2, i32* %i, align 4, !dbg !64
  br label %for.cond, !dbg !64

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !64
  %1 = load i32* %n.addr, align 4, !dbg !64
  %cmp = icmp slt i32 %0, %1, !dbg !64
  br i1 %cmp, label %for.body, label %for.end, !dbg !64

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !65
  %3 = load i32* %product, align 4, !dbg !65
  %mul = mul nsw i32 %3, %2, !dbg !65
  store i32 %mul, i32* %product, align 4, !dbg !65
  br label %for.inc, !dbg !67

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !68
  %inc = add nsw i32 %4, 1, !dbg !68
  store i32 %inc, i32* %i, align 4, !dbg !68
  br label %for.cond, !dbg !68

for.end:                                          ; preds = %for.cond
  %5 = load i32* %product, align 4, !dbg !69
  ret i32 %5, !dbg !69
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @add(), !dbg !70
  %call1 = call i32 @foo(i32 %call), !dbg !70
  %call2 = call i32 @mul(i32 10), !dbg !72
  %0 = load i32* %retval, !dbg !73
  ret i32 %0, !dbg !73
}

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"test/cases/loop.1.c", metadata !"/home/ryan/project/perfscope", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12, metadata !15, metadata !18}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"foo", metadata !"foo", metadata !"", metadata !6, i32 4, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @foo, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"test/cases/loop.1.c", metadata !"/home/ryan/project/perfscope", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 720942, i32 0, metadata !6, metadata !"add", metadata !"add", metadata !"", metadata !6, i32 16, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @add, null, null, metadata !13} ; [ DW_TAG_subprogram ]
!13 = metadata !{metadata !14}
!14 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!15 = metadata !{i32 720942, i32 0, metadata !6, metadata !"mul", metadata !"mul", metadata !"", metadata !6, i32 26, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @mul, null, null, metadata !16} ; [ DW_TAG_subprogram ]
!16 = metadata !{metadata !17}
!17 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!18 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 35, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @main, null, null, metadata !19} ; [ DW_TAG_subprogram ]
!19 = metadata !{metadata !20}
!20 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!21 = metadata !{i32 721153, metadata !5, metadata !"c", metadata !6, i32 16777219, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!22 = metadata !{i32 3, i32 13, metadata !5, null}
!23 = metadata !{i32 721152, metadata !24, metadata !"i", metadata !6, i32 5, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!24 = metadata !{i32 720907, metadata !25, i32 5, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!25 = metadata !{i32 720907, metadata !5, i32 4, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!26 = metadata !{i32 5, i32 14, metadata !24, null}
!27 = metadata !{i32 5, i32 19, metadata !24, null}
!28 = metadata !{i32 721152, metadata !29, metadata !"j", metadata !6, i32 6, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!29 = metadata !{i32 720907, metadata !30, i32 6, i32 9, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!30 = metadata !{i32 720907, metadata !24, i32 5, i32 34, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!31 = metadata !{i32 6, i32 18, metadata !29, null}
!32 = metadata !{i32 6, i32 23, metadata !29, null}
!33 = metadata !{i32 7, i32 13, metadata !34, null}
!34 = metadata !{i32 720907, metadata !29, i32 6, i32 37, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!35 = metadata !{i32 8, i32 13, metadata !34, null}
!36 = metadata !{i32 9, i32 9, metadata !34, null}
!37 = metadata !{i32 6, i32 32, metadata !29, null}
!38 = metadata !{i32 10, i32 5, metadata !30, null}
!39 = metadata !{i32 5, i32 29, metadata !24, null}
!40 = metadata !{i32 11, i32 5, metadata !25, null}
!41 = metadata !{i32 12, i32 5, metadata !25, null}
!42 = metadata !{i32 721152, metadata !43, metadata !"i", metadata !6, i32 17, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!43 = metadata !{i32 720907, metadata !12, i32 16, i32 1, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!44 = metadata !{i32 17, i32 9, metadata !43, null}
!45 = metadata !{i32 721152, metadata !43, metadata !"sum", metadata !6, i32 18, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!46 = metadata !{i32 18, i32 9, metadata !43, null}
!47 = metadata !{i32 18, i32 16, metadata !43, null}
!48 = metadata !{i32 19, i32 10, metadata !49, null}
!49 = metadata !{i32 720907, metadata !43, i32 19, i32 5, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!50 = metadata !{i32 20, i32 8, metadata !51, null}
!51 = metadata !{i32 720907, metadata !49, i32 19, i32 35, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!52 = metadata !{i32 21, i32 5, metadata !51, null}
!53 = metadata !{i32 19, i32 30, metadata !49, null}
!54 = metadata !{i32 22, i32 5, metadata !43, null}
!55 = metadata !{i32 721153, metadata !15, metadata !"n", metadata !6, i32 16777241, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!56 = metadata !{i32 25, i32 13, metadata !15, null}
!57 = metadata !{i32 721152, metadata !58, metadata !"product", metadata !6, i32 27, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!58 = metadata !{i32 720907, metadata !15, i32 26, i32 1, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!59 = metadata !{i32 27, i32 7, metadata !58, null}
!60 = metadata !{i32 27, i32 18, metadata !58, null}
!61 = metadata !{i32 721152, metadata !62, metadata !"i", metadata !6, i32 28, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!62 = metadata !{i32 720907, metadata !58, i32 28, i32 3, metadata !6, i32 9} ; [ DW_TAG_lexical_block ]
!63 = metadata !{i32 28, i32 12, metadata !62, null}
!64 = metadata !{i32 28, i32 17, metadata !62, null}
!65 = metadata !{i32 29, i32 5, metadata !66, null}
!66 = metadata !{i32 720907, metadata !62, i32 28, i32 31, metadata !6, i32 10} ; [ DW_TAG_lexical_block ]
!67 = metadata !{i32 30, i32 3, metadata !66, null}
!68 = metadata !{i32 28, i32 26, metadata !62, null}
!69 = metadata !{i32 31, i32 3, metadata !58, null}
!70 = metadata !{i32 36, i32 9, metadata !71, null}
!71 = metadata !{i32 720907, metadata !18, i32 35, i32 1, metadata !6, i32 11} ; [ DW_TAG_lexical_block ]
!72 = metadata !{i32 37, i32 5, metadata !71, null}
!73 = metadata !{i32 38, i32 1, metadata !71, null}

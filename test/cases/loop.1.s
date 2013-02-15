; ModuleID = 'test/cases/loop.1.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %c) nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 %c, i32* %c.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %c.addr}, metadata !24), !dbg !25
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !26), !dbg !29
  store i32 0, i32* %i, align 4, !dbg !30
  br label %for.cond, !dbg !30

for.cond:                                         ; preds = %for.inc11, %entry
  %0 = load i32* %i, align 4, !dbg !30
  %cmp = icmp slt i32 %0, 10, !dbg !30
  br i1 %cmp, label %for.body, label %for.end13, !dbg !30

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.declare(metadata !{i32* %j}, metadata !31), !dbg !34
  store i32 0, i32* %j, align 4, !dbg !35
  br label %for.cond1, !dbg !35

for.cond1:                                        ; preds = %for.inc7, %for.body
  %1 = load i32* %j, align 4, !dbg !35
  %cmp2 = icmp slt i32 %1, 3, !dbg !35
  br i1 %cmp2, label %for.body3, label %for.end9, !dbg !35

for.body3:                                        ; preds = %for.cond1
  call void @llvm.dbg.declare(metadata !{i32* %k}, metadata !36), !dbg !39
  store i32 0, i32* %k, align 4, !dbg !40
  br label %for.cond4, !dbg !40

for.cond4:                                        ; preds = %for.inc, %for.body3
  %2 = load i32* %k, align 4, !dbg !40
  %cmp5 = icmp slt i32 %2, 5, !dbg !40
  br i1 %cmp5, label %for.body6, label %for.end, !dbg !40

for.body6:                                        ; preds = %for.cond4
  %3 = load i32* %i, align 4, !dbg !41
  %4 = load i32* %c.addr, align 4, !dbg !41
  %add = add nsw i32 %4, %3, !dbg !41
  store i32 %add, i32* %c.addr, align 4, !dbg !41
  %5 = load i32* %j, align 4, !dbg !43
  %6 = load i32* %c.addr, align 4, !dbg !43
  %mul = mul nsw i32 %6, %5, !dbg !43
  store i32 %mul, i32* %c.addr, align 4, !dbg !43
  br label %for.inc, !dbg !44

for.inc:                                          ; preds = %for.body6
  %7 = load i32* %k, align 4, !dbg !45
  %inc = add nsw i32 %7, 1, !dbg !45
  store i32 %inc, i32* %k, align 4, !dbg !45
  br label %for.cond4, !dbg !45

for.end:                                          ; preds = %for.cond4
  br label %for.inc7, !dbg !46

for.inc7:                                         ; preds = %for.end
  %8 = load i32* %j, align 4, !dbg !47
  %inc8 = add nsw i32 %8, 1, !dbg !47
  store i32 %inc8, i32* %j, align 4, !dbg !47
  br label %for.cond1, !dbg !47

for.end9:                                         ; preds = %for.cond1
  %9 = load i32* %c.addr, align 4, !dbg !48
  %cmp10 = icmp sgt i32 %9, 100, !dbg !48
  br i1 %cmp10, label %if.then, label %if.end, !dbg !48

if.then:                                          ; preds = %for.end9
  %10 = load i32* %c.addr, align 4, !dbg !49
  store i32 %10, i32* %retval, !dbg !49
  br label %return, !dbg !49

if.end:                                           ; preds = %for.end9
  br label %for.inc11, !dbg !50

for.inc11:                                        ; preds = %if.end
  %11 = load i32* %i, align 4, !dbg !51
  %inc12 = add nsw i32 %11, 1, !dbg !51
  store i32 %inc12, i32* %i, align 4, !dbg !51
  br label %for.cond, !dbg !51

for.end13:                                        ; preds = %for.cond
  %12 = load i32* %c.addr, align 4, !dbg !52
  %inc14 = add nsw i32 %12, 1, !dbg !52
  store i32 %inc14, i32* %c.addr, align 4, !dbg !52
  %13 = load i32* %c.addr, align 4, !dbg !53
  store i32 %13, i32* %retval, !dbg !53
  br label %return, !dbg !53

return:                                           ; preds = %for.end13, %if.then
  %14 = load i32* %retval, !dbg !54
  ret i32 %14, !dbg !54
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

define i32 @add() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !55), !dbg !57
  call void @llvm.dbg.declare(metadata !{i32* %sum}, metadata !58), !dbg !59
  store i32 0, i32* %sum, align 4, !dbg !60
  store i32 1, i32* %i, align 4, !dbg !61
  br label %for.cond, !dbg !61

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !61
  %1 = load i32* %i, align 4, !dbg !61
  %mul = mul nsw i32 %0, %1, !dbg !61
  %cmp = icmp slt i32 %mul, 81, !dbg !61
  br i1 %cmp, label %for.body, label %for.end, !dbg !61

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !63
  %3 = load i32* %sum, align 4, !dbg !63
  %add = add nsw i32 %3, %2, !dbg !63
  store i32 %add, i32* %sum, align 4, !dbg !63
  br label %for.inc, !dbg !65

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !66
  %inc = add nsw i32 %4, 1, !dbg !66
  store i32 %inc, i32* %i, align 4, !dbg !66
  br label %for.cond, !dbg !66

for.end:                                          ; preds = %for.cond
  %5 = load i32* %sum, align 4, !dbg !67
  %cmp1 = icmp sgt i32 %5, 10000, !dbg !67
  br i1 %cmp1, label %if.then, label %if.end, !dbg !67

if.then:                                          ; preds = %for.end
  store i32 10000, i32* %retval, !dbg !68
  br label %return, !dbg !68

if.end:                                           ; preds = %for.end
  %6 = load i32* %sum, align 4, !dbg !69
  store i32 %6, i32* %retval, !dbg !69
  br label %return, !dbg !69

return:                                           ; preds = %if.end, %if.then
  %7 = load i32* %retval, !dbg !70
  ret i32 %7, !dbg !70
}

define i32 @mul(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %product = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %n.addr}, metadata !71), !dbg !72
  call void @llvm.dbg.declare(metadata !{i32* %product}, metadata !73), !dbg !75
  store i32 1, i32* %product, align 4, !dbg !76
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !77), !dbg !79
  store i32 2, i32* %i, align 4, !dbg !80
  br label %for.cond, !dbg !80

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !80
  %1 = load i32* %n.addr, align 4, !dbg !80
  %cmp = icmp slt i32 %0, %1, !dbg !80
  br i1 %cmp, label %for.body, label %for.end, !dbg !80

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !81
  %3 = load i32* %product, align 4, !dbg !81
  %mul = mul nsw i32 %3, %2, !dbg !81
  store i32 %mul, i32* %product, align 4, !dbg !81
  br label %for.inc, !dbg !83

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !84
  %inc = add nsw i32 %4, 1, !dbg !84
  store i32 %inc, i32* %i, align 4, !dbg !84
  br label %for.cond, !dbg !84

for.end:                                          ; preds = %for.cond
  %5 = load i32* %product, align 4, !dbg !85
  ret i32 %5, !dbg !85
}

define i32 @bar(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %ret = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %n.addr}, metadata !86), !dbg !87
  call void @llvm.dbg.declare(metadata !{i32* %ret}, metadata !88), !dbg !90
  %0 = load i32* %n.addr, align 4, !dbg !91
  %cmp = icmp sgt i32 %0, 10, !dbg !91
  br i1 %cmp, label %if.then, label %if.else, !dbg !91

if.then:                                          ; preds = %entry
  store i32 10, i32* %ret, align 4, !dbg !92
  br label %if.end, !dbg !92

if.else:                                          ; preds = %entry
  %1 = load i32* %n.addr, align 4, !dbg !93
  %div = sdiv i32 %1, 2, !dbg !93
  store i32 %div, i32* %ret, align 4, !dbg !93
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = load i32* %ret, align 4, !dbg !94
  ret i32 %2, !dbg !94
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @add(), !dbg !95
  %call1 = call i32 @foo(i32 %call), !dbg !95
  %call2 = call i32 @mul(i32 10), !dbg !97
  %0 = load i32* %retval, !dbg !98
  ret i32 %0, !dbg !98
}

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"test/cases/loop.1.c", metadata !"/home/ryan/project/perfscope", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12, metadata !15, metadata !18, metadata !21}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"foo", metadata !"foo", metadata !"", metadata !6, i32 4, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @foo, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"test/cases/loop.1.c", metadata !"/home/ryan/project/perfscope", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 720942, i32 0, metadata !6, metadata !"add", metadata !"add", metadata !"", metadata !6, i32 20, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @add, null, null, metadata !13} ; [ DW_TAG_subprogram ]
!13 = metadata !{metadata !14}
!14 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!15 = metadata !{i32 720942, i32 0, metadata !6, metadata !"mul", metadata !"mul", metadata !"", metadata !6, i32 32, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @mul, null, null, metadata !16} ; [ DW_TAG_subprogram ]
!16 = metadata !{metadata !17}
!17 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!18 = metadata !{i32 720942, i32 0, metadata !6, metadata !"bar", metadata !"bar", metadata !"", metadata !6, i32 41, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @bar, null, null, metadata !19} ; [ DW_TAG_subprogram ]
!19 = metadata !{metadata !20}
!20 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!21 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 51, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @main, null, null, metadata !22} ; [ DW_TAG_subprogram ]
!22 = metadata !{metadata !23}
!23 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!24 = metadata !{i32 721153, metadata !5, metadata !"c", metadata !6, i32 16777219, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!25 = metadata !{i32 3, i32 13, metadata !5, null}
!26 = metadata !{i32 721152, metadata !27, metadata !"i", metadata !6, i32 5, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!27 = metadata !{i32 720907, metadata !28, i32 5, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!28 = metadata !{i32 720907, metadata !5, i32 4, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!29 = metadata !{i32 5, i32 14, metadata !27, null}
!30 = metadata !{i32 5, i32 19, metadata !27, null}
!31 = metadata !{i32 721152, metadata !32, metadata !"j", metadata !6, i32 6, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!32 = metadata !{i32 720907, metadata !33, i32 6, i32 9, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!33 = metadata !{i32 720907, metadata !27, i32 5, i32 34, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!34 = metadata !{i32 6, i32 18, metadata !32, null}
!35 = metadata !{i32 6, i32 23, metadata !32, null}
!36 = metadata !{i32 721152, metadata !37, metadata !"k", metadata !6, i32 7, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!37 = metadata !{i32 720907, metadata !38, i32 7, i32 11, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!38 = metadata !{i32 720907, metadata !32, i32 6, i32 37, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!39 = metadata !{i32 7, i32 20, metadata !37, null}
!40 = metadata !{i32 7, i32 25, metadata !37, null}
!41 = metadata !{i32 8, i32 13, metadata !42, null}
!42 = metadata !{i32 720907, metadata !37, i32 7, i32 39, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!43 = metadata !{i32 9, i32 13, metadata !42, null}
!44 = metadata !{i32 10, i32 11, metadata !42, null}
!45 = metadata !{i32 7, i32 34, metadata !37, null}
!46 = metadata !{i32 11, i32 9, metadata !38, null}
!47 = metadata !{i32 6, i32 32, metadata !32, null}
!48 = metadata !{i32 12, i32 9, metadata !33, null}
!49 = metadata !{i32 13, i32 11, metadata !33, null}
!50 = metadata !{i32 14, i32 5, metadata !33, null}
!51 = metadata !{i32 5, i32 29, metadata !27, null}
!52 = metadata !{i32 15, i32 5, metadata !28, null}
!53 = metadata !{i32 16, i32 5, metadata !28, null}
!54 = metadata !{i32 17, i32 1, metadata !28, null}
!55 = metadata !{i32 721152, metadata !56, metadata !"i", metadata !6, i32 21, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!56 = metadata !{i32 720907, metadata !12, i32 20, i32 1, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!57 = metadata !{i32 21, i32 9, metadata !56, null}
!58 = metadata !{i32 721152, metadata !56, metadata !"sum", metadata !6, i32 22, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!59 = metadata !{i32 22, i32 9, metadata !56, null}
!60 = metadata !{i32 22, i32 16, metadata !56, null}
!61 = metadata !{i32 23, i32 10, metadata !62, null}
!62 = metadata !{i32 720907, metadata !56, i32 23, i32 5, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!63 = metadata !{i32 24, i32 8, metadata !64, null}
!64 = metadata !{i32 720907, metadata !62, i32 23, i32 34, metadata !6, i32 9} ; [ DW_TAG_lexical_block ]
!65 = metadata !{i32 25, i32 5, metadata !64, null}
!66 = metadata !{i32 23, i32 29, metadata !62, null}
!67 = metadata !{i32 26, i32 5, metadata !56, null}
!68 = metadata !{i32 27, i32 7, metadata !56, null}
!69 = metadata !{i32 28, i32 5, metadata !56, null}
!70 = metadata !{i32 29, i32 1, metadata !56, null}
!71 = metadata !{i32 721153, metadata !15, metadata !"n", metadata !6, i32 16777247, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!72 = metadata !{i32 31, i32 13, metadata !15, null}
!73 = metadata !{i32 721152, metadata !74, metadata !"product", metadata !6, i32 33, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!74 = metadata !{i32 720907, metadata !15, i32 32, i32 1, metadata !6, i32 10} ; [ DW_TAG_lexical_block ]
!75 = metadata !{i32 33, i32 7, metadata !74, null}
!76 = metadata !{i32 33, i32 18, metadata !74, null}
!77 = metadata !{i32 721152, metadata !78, metadata !"i", metadata !6, i32 34, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!78 = metadata !{i32 720907, metadata !74, i32 34, i32 3, metadata !6, i32 11} ; [ DW_TAG_lexical_block ]
!79 = metadata !{i32 34, i32 12, metadata !78, null}
!80 = metadata !{i32 34, i32 17, metadata !78, null}
!81 = metadata !{i32 35, i32 5, metadata !82, null}
!82 = metadata !{i32 720907, metadata !78, i32 34, i32 31, metadata !6, i32 12} ; [ DW_TAG_lexical_block ]
!83 = metadata !{i32 36, i32 3, metadata !82, null}
!84 = metadata !{i32 34, i32 26, metadata !78, null}
!85 = metadata !{i32 37, i32 3, metadata !74, null}
!86 = metadata !{i32 721153, metadata !18, metadata !"n", metadata !6, i32 16777256, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!87 = metadata !{i32 40, i32 13, metadata !18, null}
!88 = metadata !{i32 721152, metadata !89, metadata !"ret", metadata !6, i32 42, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!89 = metadata !{i32 720907, metadata !18, i32 41, i32 1, metadata !6, i32 13} ; [ DW_TAG_lexical_block ]
!90 = metadata !{i32 42, i32 7, metadata !89, null}
!91 = metadata !{i32 43, i32 3, metadata !89, null}
!92 = metadata !{i32 44, i32 5, metadata !89, null}
!93 = metadata !{i32 46, i32 5, metadata !89, null}
!94 = metadata !{i32 47, i32 3, metadata !89, null}
!95 = metadata !{i32 52, i32 9, metadata !96, null}
!96 = metadata !{i32 720907, metadata !21, i32 51, i32 1, metadata !6, i32 14} ; [ DW_TAG_lexical_block ]
!97 = metadata !{i32 53, i32 5, metadata !96, null}
!98 = metadata !{i32 54, i32 1, metadata !96, null}

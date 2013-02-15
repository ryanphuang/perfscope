; ModuleID = 'test/cases/loop.1.new.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %c) nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %pid = alloca i32, align 4
  store i32 %c, i32* %c.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %c.addr}, metadata !24), !dbg !25
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !26), !dbg !29
  store i32 0, i32* %i, align 4, !dbg !30
  br label %for.cond, !dbg !30

for.cond:                                         ; preds = %for.inc5, %entry
  %0 = load i32* %i, align 4, !dbg !30
  %cmp = icmp slt i32 %0, 10, !dbg !30
  br i1 %cmp, label %for.body, label %for.end7, !dbg !30

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
  call void @llvm.dbg.declare(metadata !{i32* %pid}, metadata !39), !dbg !42
  %call = call i32 @getpid() nounwind, !dbg !43
  store i32 %call, i32* %pid, align 4, !dbg !43
  br label %for.inc, !dbg !44

for.inc:                                          ; preds = %for.body3
  %6 = load i32* %j, align 4, !dbg !45
  %inc = add nsw i32 %6, 1, !dbg !45
  store i32 %inc, i32* %j, align 4, !dbg !45
  br label %for.cond1, !dbg !45

for.end:                                          ; preds = %for.cond1
  %7 = load i32* %c.addr, align 4, !dbg !46
  %cmp4 = icmp sgt i32 %7, 100, !dbg !46
  br i1 %cmp4, label %if.then, label %if.end, !dbg !46

if.then:                                          ; preds = %for.end
  %8 = load i32* %c.addr, align 4, !dbg !47
  store i32 %8, i32* %retval, !dbg !47
  br label %return, !dbg !47

if.end:                                           ; preds = %for.end
  br label %for.inc5, !dbg !48

for.inc5:                                         ; preds = %if.end
  %9 = load i32* %i, align 4, !dbg !49
  %inc6 = add nsw i32 %9, 1, !dbg !49
  store i32 %inc6, i32* %i, align 4, !dbg !49
  br label %for.cond, !dbg !49

for.end7:                                         ; preds = %for.cond
  %10 = load i32* %c.addr, align 4, !dbg !50
  %inc8 = add nsw i32 %10, 1, !dbg !50
  store i32 %inc8, i32* %c.addr, align 4, !dbg !50
  %11 = load i32* %c.addr, align 4, !dbg !51
  store i32 %11, i32* %retval, !dbg !51
  br label %return, !dbg !51

return:                                           ; preds = %for.end7, %if.then
  %12 = load i32* %retval, !dbg !52
  ret i32 %12, !dbg !52
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare i32 @getpid() nounwind

define i32 @add() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !53), !dbg !55
  call void @llvm.dbg.declare(metadata !{i32* %sum}, metadata !56), !dbg !57
  store i32 0, i32* %sum, align 4, !dbg !58
  store i32 1, i32* %i, align 4, !dbg !59
  br label %for.cond, !dbg !59

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !59
  %1 = load i32* %i, align 4, !dbg !59
  %mul = mul nsw i32 %0, %1, !dbg !59
  %cmp = icmp slt i32 %mul, 100, !dbg !59
  br i1 %cmp, label %for.body, label %for.end, !dbg !59

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !61
  %3 = load i32* %sum, align 4, !dbg !61
  %add = add nsw i32 %3, %2, !dbg !61
  store i32 %add, i32* %sum, align 4, !dbg !61
  br label %for.inc, !dbg !63

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !64
  %inc = add nsw i32 %4, 1, !dbg !64
  store i32 %inc, i32* %i, align 4, !dbg !64
  br label %for.cond, !dbg !64

for.end:                                          ; preds = %for.cond
  %5 = load i32* %sum, align 4, !dbg !65
  %cmp1 = icmp sgt i32 %5, 10000, !dbg !65
  br i1 %cmp1, label %if.then, label %if.end, !dbg !65

if.then:                                          ; preds = %for.end
  store i32 10000, i32* %retval, !dbg !66
  br label %return, !dbg !66

if.end:                                           ; preds = %for.end
  %6 = load i32* %sum, align 4, !dbg !67
  store i32 %6, i32* %retval, !dbg !67
  br label %return, !dbg !67

return:                                           ; preds = %if.end, %if.then
  %7 = load i32* %retval, !dbg !68
  ret i32 %7, !dbg !68
}

define i32 @mul(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %product = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %n.addr}, metadata !69), !dbg !70
  call void @llvm.dbg.declare(metadata !{i32* %product}, metadata !71), !dbg !73
  store i32 1, i32* %product, align 4, !dbg !74
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !75), !dbg !77
  store i32 2, i32* %i, align 4, !dbg !78
  br label %for.cond, !dbg !78

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !78
  %1 = load i32* %n.addr, align 4, !dbg !78
  %cmp = icmp slt i32 %0, %1, !dbg !78
  br i1 %cmp, label %for.body, label %for.end, !dbg !78

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4, !dbg !79
  %3 = load i32* %product, align 4, !dbg !79
  %mul = mul nsw i32 %3, %2, !dbg !79
  store i32 %mul, i32* %product, align 4, !dbg !79
  br label %for.inc, !dbg !81

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4, !dbg !82
  %inc = add nsw i32 %4, 1, !dbg !82
  store i32 %inc, i32* %i, align 4, !dbg !82
  br label %for.cond, !dbg !82

for.end:                                          ; preds = %for.cond
  %5 = load i32* %product, align 4, !dbg !83
  ret i32 %5, !dbg !83
}

define i32 @bar(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %ret = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  call void @llvm.dbg.declare(metadata !{i32* %n.addr}, metadata !84), !dbg !85
  call void @llvm.dbg.declare(metadata !{i32* %ret}, metadata !86), !dbg !88
  %0 = load i32* %n.addr, align 4, !dbg !89
  %cmp = icmp sgt i32 %0, 10, !dbg !89
  br i1 %cmp, label %if.then, label %if.else, !dbg !89

if.then:                                          ; preds = %entry
  store i32 10, i32* %ret, align 4, !dbg !90
  br label %if.end, !dbg !90

if.else:                                          ; preds = %entry
  %1 = load i32* %n.addr, align 4, !dbg !91
  %div = sdiv i32 %1, 2, !dbg !91
  store i32 %div, i32* %ret, align 4, !dbg !91
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = load i32* %ret, align 4, !dbg !92
  ret i32 %2, !dbg !92
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @add(), !dbg !93
  %call1 = call i32 @foo(i32 %call), !dbg !93
  %call2 = call i32 @mul(i32 10), !dbg !95
  %0 = load i32* %retval, !dbg !96
  ret i32 %0, !dbg !96
}

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"test/cases/loop.1.new.c", metadata !"/home/ryan/project/perfscope", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12, metadata !15, metadata !18, metadata !21}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"foo", metadata !"foo", metadata !"", metadata !6, i32 6, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @foo, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"test/cases/loop.1.new.c", metadata !"/home/ryan/project/perfscope", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 720942, i32 0, metadata !6, metadata !"add", metadata !"add", metadata !"", metadata !6, i32 21, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @add, null, null, metadata !13} ; [ DW_TAG_subprogram ]
!13 = metadata !{metadata !14}
!14 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!15 = metadata !{i32 720942, i32 0, metadata !6, metadata !"mul", metadata !"mul", metadata !"", metadata !6, i32 33, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @mul, null, null, metadata !16} ; [ DW_TAG_subprogram ]
!16 = metadata !{metadata !17}
!17 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!18 = metadata !{i32 720942, i32 0, metadata !6, metadata !"bar", metadata !"bar", metadata !"", metadata !6, i32 42, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32)* @bar, null, null, metadata !19} ; [ DW_TAG_subprogram ]
!19 = metadata !{metadata !20}
!20 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!21 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 52, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @main, null, null, metadata !22} ; [ DW_TAG_subprogram ]
!22 = metadata !{metadata !23}
!23 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!24 = metadata !{i32 721153, metadata !5, metadata !"c", metadata !6, i32 16777221, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!25 = metadata !{i32 5, i32 13, metadata !5, null}
!26 = metadata !{i32 721152, metadata !27, metadata !"i", metadata !6, i32 7, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!27 = metadata !{i32 720907, metadata !28, i32 7, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!28 = metadata !{i32 720907, metadata !5, i32 6, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
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
!39 = metadata !{i32 721152, metadata !37, metadata !"pid", metadata !6, i32 11, metadata !40, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!40 = metadata !{i32 720918, null, metadata !"pid_t", metadata !6, i32 99, i64 0, i64 0, i64 0, i32 0, metadata !41} ; [ DW_TAG_typedef ]
!41 = metadata !{i32 720918, null, metadata !"__pid_t", metadata !6, i32 143, i64 0, i64 0, i64 0, i32 0, metadata !9} ; [ DW_TAG_typedef ]
!42 = metadata !{i32 11, i32 19, metadata !37, null}
!43 = metadata !{i32 11, i32 25, metadata !37, null}
!44 = metadata !{i32 12, i32 9, metadata !37, null}
!45 = metadata !{i32 8, i32 32, metadata !32, null}
!46 = metadata !{i32 13, i32 9, metadata !33, null}
!47 = metadata !{i32 14, i32 11, metadata !33, null}
!48 = metadata !{i32 15, i32 5, metadata !33, null}
!49 = metadata !{i32 7, i32 29, metadata !27, null}
!50 = metadata !{i32 16, i32 5, metadata !28, null}
!51 = metadata !{i32 17, i32 5, metadata !28, null}
!52 = metadata !{i32 18, i32 1, metadata !28, null}
!53 = metadata !{i32 721152, metadata !54, metadata !"i", metadata !6, i32 22, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!54 = metadata !{i32 720907, metadata !12, i32 21, i32 1, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!55 = metadata !{i32 22, i32 9, metadata !54, null}
!56 = metadata !{i32 721152, metadata !54, metadata !"sum", metadata !6, i32 23, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!57 = metadata !{i32 23, i32 9, metadata !54, null}
!58 = metadata !{i32 23, i32 16, metadata !54, null}
!59 = metadata !{i32 24, i32 10, metadata !60, null}
!60 = metadata !{i32 720907, metadata !54, i32 24, i32 5, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!61 = metadata !{i32 25, i32 8, metadata !62, null}
!62 = metadata !{i32 720907, metadata !60, i32 24, i32 35, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!63 = metadata !{i32 26, i32 5, metadata !62, null}
!64 = metadata !{i32 24, i32 30, metadata !60, null}
!65 = metadata !{i32 27, i32 5, metadata !54, null}
!66 = metadata !{i32 28, i32 7, metadata !54, null}
!67 = metadata !{i32 29, i32 5, metadata !54, null}
!68 = metadata !{i32 30, i32 1, metadata !54, null}
!69 = metadata !{i32 721153, metadata !15, metadata !"n", metadata !6, i32 16777248, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!70 = metadata !{i32 32, i32 13, metadata !15, null}
!71 = metadata !{i32 721152, metadata !72, metadata !"product", metadata !6, i32 34, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!72 = metadata !{i32 720907, metadata !15, i32 33, i32 1, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!73 = metadata !{i32 34, i32 7, metadata !72, null}
!74 = metadata !{i32 34, i32 18, metadata !72, null}
!75 = metadata !{i32 721152, metadata !76, metadata !"i", metadata !6, i32 35, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!76 = metadata !{i32 720907, metadata !72, i32 35, i32 3, metadata !6, i32 9} ; [ DW_TAG_lexical_block ]
!77 = metadata !{i32 35, i32 12, metadata !76, null}
!78 = metadata !{i32 35, i32 17, metadata !76, null}
!79 = metadata !{i32 36, i32 5, metadata !80, null}
!80 = metadata !{i32 720907, metadata !76, i32 35, i32 31, metadata !6, i32 10} ; [ DW_TAG_lexical_block ]
!81 = metadata !{i32 37, i32 3, metadata !80, null}
!82 = metadata !{i32 35, i32 26, metadata !76, null}
!83 = metadata !{i32 38, i32 3, metadata !72, null}
!84 = metadata !{i32 721153, metadata !18, metadata !"n", metadata !6, i32 16777257, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!85 = metadata !{i32 41, i32 13, metadata !18, null}
!86 = metadata !{i32 721152, metadata !87, metadata !"ret", metadata !6, i32 43, metadata !9, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!87 = metadata !{i32 720907, metadata !18, i32 42, i32 1, metadata !6, i32 11} ; [ DW_TAG_lexical_block ]
!88 = metadata !{i32 43, i32 7, metadata !87, null}
!89 = metadata !{i32 44, i32 3, metadata !87, null}
!90 = metadata !{i32 45, i32 5, metadata !87, null}
!91 = metadata !{i32 47, i32 5, metadata !87, null}
!92 = metadata !{i32 48, i32 3, metadata !87, null}
!93 = metadata !{i32 53, i32 9, metadata !94, null}
!94 = metadata !{i32 720907, metadata !21, i32 52, i32 1, metadata !6, i32 12} ; [ DW_TAG_lexical_block ]
!95 = metadata !{i32 54, i32 5, metadata !94, null}
!96 = metadata !{i32 55, i32 1, metadata !94, null}

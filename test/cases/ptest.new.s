; ModuleID = 'test/cases/ptest.new.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%union.pthread_mutex_t = type { %struct.__pthread_mutex_s }
%struct.__pthread_mutex_s = type { i32, i32, i32, i32, i32, i32, %struct.__pthread_internal_list }
%struct.__pthread_internal_list = type { %struct.__pthread_internal_list*, %struct.__pthread_internal_list* }
%union.pthread_attr_t = type { i64, [48 x i8] }

@mutex = common global %union.pthread_mutex_t zeroinitializer, align 8
@.str = private unnamed_addr constant [16 x i8] c"hello %d start\0A\00", align 1
@counter = internal global i32 1, align 4
@.str1 = private unnamed_addr constant [14 x i8] c"hello %d end\0A\00", align 1
@.str2 = private unnamed_addr constant [25 x i8] c"Cannot create thread %d\0A\00", align 1
@.str3 = private unnamed_addr constant [23 x i8] c"Cannot join thread %d\0A\00", align 1

define i8* @routine(i8* %arg) nounwind uwtable {
entry:
  %arg.addr = alloca i8*, align 8
  %i = alloca i32, align 4
  store i8* %arg, i8** %arg.addr, align 8
  call void @llvm.dbg.declare(metadata !{i8** %arg.addr}, metadata !50), !dbg !51
  %call = call i32 @pthread_mutex_lock(%union.pthread_mutex_t* @mutex) nounwind, !dbg !52
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !54), !dbg !55
  store i32 0, i32* %i, align 4, !dbg !56
  br label %for.cond, !dbg !56

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !56
  %cmp = icmp ult i32 %0, 1048575, !dbg !56
  br i1 %cmp, label %for.body, label %for.end, !dbg !56

for.body:                                         ; preds = %for.cond
  br label %for.inc, !dbg !56

for.inc:                                          ; preds = %for.body
  %1 = load i32* %i, align 4, !dbg !58
  %inc = add i32 %1, 1, !dbg !58
  store i32 %inc, i32* %i, align 4, !dbg !58
  br label %for.cond, !dbg !58

for.end:                                          ; preds = %for.cond
  %2 = load i32* @counter, align 4, !dbg !59
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([16 x i8]* @.str, i32 0, i32 0), i32 %2), !dbg !59
  store i32 0, i32* %i, align 4, !dbg !60
  br label %for.cond2, !dbg !60

for.cond2:                                        ; preds = %for.inc5, %for.end
  %3 = load i32* %i, align 4, !dbg !60
  %cmp3 = icmp ult i32 %3, 1048575, !dbg !60
  br i1 %cmp3, label %for.body4, label %for.end7, !dbg !60

for.body4:                                        ; preds = %for.cond2
  br label %for.inc5, !dbg !60

for.inc5:                                         ; preds = %for.body4
  %4 = load i32* %i, align 4, !dbg !62
  %inc6 = add i32 %4, 1, !dbg !62
  store i32 %inc6, i32* %i, align 4, !dbg !62
  br label %for.cond2, !dbg !62

for.end7:                                         ; preds = %for.cond2
  %5 = load i32* @counter, align 4, !dbg !63
  %call8 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @.str1, i32 0, i32 0), i32 %5), !dbg !63
  %6 = load i32* @counter, align 4, !dbg !64
  %inc9 = add nsw i32 %6, 1, !dbg !64
  store i32 %inc9, i32* @counter, align 4, !dbg !64
  %call10 = call i32 @pthread_mutex_unlock(%union.pthread_mutex_t* @mutex) nounwind, !dbg !65
  ret i8* null, !dbg !66
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare i32 @pthread_mutex_lock(%union.pthread_mutex_t*) nounwind

declare i32 @printf(i8*, ...)

declare i32 @pthread_mutex_unlock(%union.pthread_mutex_t*) nounwind

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %thr = alloca [2 x i64], align 16
  %i = alloca i32, align 4
  store i32 0, i32* %retval
  call void @llvm.dbg.declare(metadata !{[2 x i64]* %thr}, metadata !67), !dbg !74
  call void @llvm.dbg.declare(metadata !{i32* %i}, metadata !75), !dbg !76
  store i32 0, i32* %i, align 4, !dbg !77
  br label %for.cond, !dbg !77

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4, !dbg !77
  %cmp = icmp slt i32 %0, 2, !dbg !77
  br i1 %cmp, label %for.body, label %for.end, !dbg !77

for.body:                                         ; preds = %for.cond
  %1 = load i32* %i, align 4, !dbg !79
  %idxprom = sext i32 %1 to i64, !dbg !79
  %arrayidx = getelementptr inbounds [2 x i64]* %thr, i32 0, i64 %idxprom, !dbg !79
  %call = call i32 @pthread_create(i64* %arrayidx, %union.pthread_attr_t* null, i8* (i8*)* @routine, i8* null) nounwind, !dbg !79
  %tobool = icmp ne i32 %call, 0, !dbg !79
  br i1 %tobool, label %if.then, label %if.end, !dbg !79

if.then:                                          ; preds = %for.body
  %2 = load i32* %i, align 4, !dbg !81
  %call1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([25 x i8]* @.str2, i32 0, i32 0), i32 %2), !dbg !81
  call void @exit(i32 1) noreturn nounwind, !dbg !83
  unreachable, !dbg !83

if.end:                                           ; preds = %for.body
  br label %for.inc, !dbg !84

for.inc:                                          ; preds = %if.end
  %3 = load i32* %i, align 4, !dbg !85
  %inc = add nsw i32 %3, 1, !dbg !85
  store i32 %inc, i32* %i, align 4, !dbg !85
  br label %for.cond, !dbg !85

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %i, align 4, !dbg !86
  br label %for.cond2, !dbg !86

for.cond2:                                        ; preds = %for.inc12, %for.end
  %4 = load i32* %i, align 4, !dbg !86
  %cmp3 = icmp slt i32 %4, 2, !dbg !86
  br i1 %cmp3, label %for.body4, label %for.end14, !dbg !86

for.body4:                                        ; preds = %for.cond2
  %5 = load i32* %i, align 4, !dbg !88
  %idxprom5 = sext i32 %5 to i64, !dbg !88
  %arrayidx6 = getelementptr inbounds [2 x i64]* %thr, i32 0, i64 %idxprom5, !dbg !88
  %6 = load i64* %arrayidx6, align 8, !dbg !88
  %call7 = call i32 @pthread_join(i64 %6, i8** null), !dbg !88
  %tobool8 = icmp ne i32 %call7, 0, !dbg !88
  br i1 %tobool8, label %if.then9, label %if.end11, !dbg !88

if.then9:                                         ; preds = %for.body4
  %7 = load i32* %i, align 4, !dbg !90
  %call10 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([23 x i8]* @.str3, i32 0, i32 0), i32 %7), !dbg !90
  br label %if.end11, !dbg !90

if.end11:                                         ; preds = %if.then9, %for.body4
  br label %for.inc12, !dbg !91

for.inc12:                                        ; preds = %if.end11
  %8 = load i32* %i, align 4, !dbg !92
  %inc13 = add nsw i32 %8, 1, !dbg !92
  store i32 %inc13, i32* %i, align 4, !dbg !92
  br label %for.cond2, !dbg !92

for.end14:                                        ; preds = %for.cond2
  ret i32 0, !dbg !93
}

declare i32 @pthread_create(i64*, %union.pthread_attr_t*, i8* (i8*)*, i8*) nounwind

declare void @exit(i32) noreturn nounwind

declare i32 @pthread_join(i64, i8**)

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"test/cases/ptest.new.c", metadata !"/home/ryan/project/perfscope", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !18} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5, metadata !12}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"routine", metadata !"routine", metadata !"", metadata !6, i32 9, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i8* (i8*)* @routine, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"test/cases/ptest.new.c", metadata !"/home/ryan/project/perfscope", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 23, metadata !13, i1 false, i1 true, i32 0, i32 0, i32 0, i32 0, i1 false, i32 ()* @main, null, null, metadata !16} ; [ DW_TAG_subprogram ]
!13 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !14, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!14 = metadata !{metadata !15}
!15 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!16 = metadata !{metadata !17}
!17 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!18 = metadata !{metadata !19}
!19 = metadata !{metadata !20, metadata !49}
!20 = metadata !{i32 720948, i32 0, null, metadata !"mutex", metadata !"mutex", metadata !"", metadata !6, i32 5, metadata !21, i32 0, i32 1, %union.pthread_mutex_t* @mutex} ; [ DW_TAG_variable ]
!21 = metadata !{i32 720918, null, metadata !"pthread_mutex_t", metadata !6, i32 104, i64 0, i64 0, i64 0, i32 0, metadata !22} ; [ DW_TAG_typedef ]
!22 = metadata !{i32 720919, null, metadata !"", metadata !23, i32 76, i64 320, i64 64, i64 0, i32 0, i32 0, metadata !24, i32 0, i32 0} ; [ DW_TAG_union_type ]
!23 = metadata !{i32 720937, metadata !"/usr/include/bits/pthreadtypes.h", metadata !"/home/ryan/project/perfscope", null} ; [ DW_TAG_file_type ]
!24 = metadata !{metadata !25, metadata !42, metadata !47}
!25 = metadata !{i32 720909, metadata !22, metadata !"__data", metadata !23, i32 101, i64 320, i64 64, i64 0, i32 0, metadata !26} ; [ DW_TAG_member ]
!26 = metadata !{i32 720915, null, metadata !"__pthread_mutex_s", metadata !23, i32 78, i64 320, i64 64, i32 0, i32 0, i32 0, metadata !27, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!27 = metadata !{metadata !28, metadata !29, metadata !31, metadata !32, metadata !33, metadata !34, metadata !35}
!28 = metadata !{i32 720909, metadata !26, metadata !"__lock", metadata !23, i32 80, i64 32, i64 32, i64 0, i32 0, metadata !15} ; [ DW_TAG_member ]
!29 = metadata !{i32 720909, metadata !26, metadata !"__count", metadata !23, i32 81, i64 32, i64 32, i64 32, i32 0, metadata !30} ; [ DW_TAG_member ]
!30 = metadata !{i32 720932, null, metadata !"unsigned int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!31 = metadata !{i32 720909, metadata !26, metadata !"__owner", metadata !23, i32 82, i64 32, i64 32, i64 64, i32 0, metadata !15} ; [ DW_TAG_member ]
!32 = metadata !{i32 720909, metadata !26, metadata !"__nusers", metadata !23, i32 84, i64 32, i64 32, i64 96, i32 0, metadata !30} ; [ DW_TAG_member ]
!33 = metadata !{i32 720909, metadata !26, metadata !"__kind", metadata !23, i32 88, i64 32, i64 32, i64 128, i32 0, metadata !15} ; [ DW_TAG_member ]
!34 = metadata !{i32 720909, metadata !26, metadata !"__spins", metadata !23, i32 90, i64 32, i64 32, i64 160, i32 0, metadata !15} ; [ DW_TAG_member ]
!35 = metadata !{i32 720909, metadata !26, metadata !"__list", metadata !23, i32 91, i64 128, i64 64, i64 192, i32 0, metadata !36} ; [ DW_TAG_member ]
!36 = metadata !{i32 720918, null, metadata !"__pthread_list_t", metadata !23, i32 65, i64 0, i64 0, i64 0, i32 0, metadata !37} ; [ DW_TAG_typedef ]
!37 = metadata !{i32 720915, null, metadata !"__pthread_internal_list", metadata !23, i32 61, i64 128, i64 64, i32 0, i32 0, i32 0, metadata !38, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!38 = metadata !{metadata !39, metadata !41}
!39 = metadata !{i32 720909, metadata !37, metadata !"__prev", metadata !23, i32 63, i64 64, i64 64, i64 0, i32 0, metadata !40} ; [ DW_TAG_member ]
!40 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !37} ; [ DW_TAG_pointer_type ]
!41 = metadata !{i32 720909, metadata !37, metadata !"__next", metadata !23, i32 64, i64 64, i64 64, i64 64, i32 0, metadata !40} ; [ DW_TAG_member ]
!42 = metadata !{i32 720909, metadata !22, metadata !"__size", metadata !23, i32 102, i64 320, i64 8, i64 0, i32 0, metadata !43} ; [ DW_TAG_member ]
!43 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 320, i64 8, i32 0, i32 0, metadata !44, metadata !45, i32 0, i32 0} ; [ DW_TAG_array_type ]
!44 = metadata !{i32 720932, null, metadata !"char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!45 = metadata !{metadata !46}
!46 = metadata !{i32 720929, i64 0, i64 39}       ; [ DW_TAG_subrange_type ]
!47 = metadata !{i32 720909, metadata !22, metadata !"__align", metadata !23, i32 103, i64 64, i64 64, i64 0, i32 0, metadata !48} ; [ DW_TAG_member ]
!48 = metadata !{i32 720932, null, metadata !"long int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!49 = metadata !{i32 720948, i32 0, null, metadata !"counter", metadata !"counter", metadata !"", metadata !6, i32 6, metadata !15, i32 1, i32 1, i32* @counter} ; [ DW_TAG_variable ]
!50 = metadata !{i32 721153, metadata !5, metadata !"arg", metadata !6, i32 16777224, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!51 = metadata !{i32 8, i32 22, metadata !5, null}
!52 = metadata !{i32 10, i32 3, metadata !53, null}
!53 = metadata !{i32 720907, metadata !5, i32 9, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!54 = metadata !{i32 721152, metadata !53, metadata !"i", metadata !6, i32 11, metadata !30, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!55 = metadata !{i32 11, i32 12, metadata !53, null}
!56 = metadata !{i32 12, i32 8, metadata !57, null}
!57 = metadata !{i32 720907, metadata !53, i32 12, i32 3, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!58 = metadata !{i32 12, i32 30, metadata !57, null}
!59 = metadata !{i32 13, i32 3, metadata !53, null}
!60 = metadata !{i32 15, i32 8, metadata !61, null}
!61 = metadata !{i32 720907, metadata !53, i32 15, i32 3, metadata !6, i32 2} ; [ DW_TAG_lexical_block ]
!62 = metadata !{i32 15, i32 30, metadata !61, null}
!63 = metadata !{i32 16, i32 3, metadata !53, null}
!64 = metadata !{i32 17, i32 3, metadata !53, null}
!65 = metadata !{i32 18, i32 3, metadata !53, null}
!66 = metadata !{i32 19, i32 3, metadata !53, null}
!67 = metadata !{i32 721152, metadata !68, metadata !"thr", metadata !6, i32 24, metadata !69, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!68 = metadata !{i32 720907, metadata !12, i32 23, i32 1, metadata !6, i32 3} ; [ DW_TAG_lexical_block ]
!69 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 128, i64 64, i32 0, i32 0, metadata !70, metadata !72, i32 0, i32 0} ; [ DW_TAG_array_type ]
!70 = metadata !{i32 720918, null, metadata !"pthread_t", metadata !6, i32 50, i64 0, i64 0, i64 0, i32 0, metadata !71} ; [ DW_TAG_typedef ]
!71 = metadata !{i32 720932, null, metadata !"long unsigned int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!72 = metadata !{metadata !73}
!73 = metadata !{i32 720929, i64 0, i64 1}        ; [ DW_TAG_subrange_type ]
!74 = metadata !{i32 24, i32 13, metadata !68, null}
!75 = metadata !{i32 721152, metadata !68, metadata !"i", metadata !6, i32 25, metadata !15, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!76 = metadata !{i32 25, i32 7, metadata !68, null}
!77 = metadata !{i32 26, i32 8, metadata !78, null}
!78 = metadata !{i32 720907, metadata !68, i32 26, i32 3, metadata !6, i32 4} ; [ DW_TAG_lexical_block ]
!79 = metadata !{i32 27, i32 9, metadata !80, null}
!80 = metadata !{i32 720907, metadata !78, i32 26, i32 27, metadata !6, i32 5} ; [ DW_TAG_lexical_block ]
!81 = metadata !{i32 28, i32 7, metadata !82, null}
!82 = metadata !{i32 720907, metadata !80, i32 27, i32 56, metadata !6, i32 6} ; [ DW_TAG_lexical_block ]
!83 = metadata !{i32 29, i32 7, metadata !82, null}
!84 = metadata !{i32 31, i32 3, metadata !80, null}
!85 = metadata !{i32 26, i32 22, metadata !78, null}
!86 = metadata !{i32 32, i32 8, metadata !87, null}
!87 = metadata !{i32 720907, metadata !68, i32 32, i32 3, metadata !6, i32 7} ; [ DW_TAG_lexical_block ]
!88 = metadata !{i32 33, i32 9, metadata !89, null}
!89 = metadata !{i32 720907, metadata !87, i32 32, i32 27, metadata !6, i32 8} ; [ DW_TAG_lexical_block ]
!90 = metadata !{i32 34, i32 7, metadata !89, null}
!91 = metadata !{i32 35, i32 3, metadata !89, null}
!92 = metadata !{i32 32, i32 22, metadata !87, null}
!93 = metadata !{i32 36, i32 3, metadata !68, null}

; ModuleID = 'a.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"sum = %d\0A\00", align 1
@.str1 = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1

define i32 @add(i32 %a, i32 %b) nounwind uwtable {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %0 = load i32* %a.addr, align 4
  %1 = load i32* %b.addr, align 4
  %add = add nsw i32 %0, %1
  ret i32 %add
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 1, i32* %i, align 4
  store i32 0, i32* %sum, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 11
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %1 = load i32* %sum, align 4
  %2 = load i32* %i, align 4
  %call = call i32 @add(i32 %1, i32 %2)
  store i32 %call, i32* %sum, align 4
  %3 = load i32* %i, align 4
  %call1 = call i32 @add(i32 %3, i32 1)
  store i32 %call1, i32* %i, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %4 = load i32* %sum, align 4
  %call2 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.str, i32 0, i32 0), i32 %4)
  %5 = load i32* %i, align 4
  %call3 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([8 x i8]* @.str1, i32 0, i32 0), i32 %5)
  ret i32 0
}

declare i32 @printf(i8*, ...)

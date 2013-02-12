; ModuleID = 'test/cases/loop.1.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32 %c) nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %c.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %c, i32* %c.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc5, %entry
  %0 = load i32* %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end7

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32* %j, align 4
  %cmp2 = icmp slt i32 %1, 5
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %2 = load i32* %i, align 4
  %3 = load i32* %c.addr, align 4
  %add = add nsw i32 %3, %2
  store i32 %add, i32* %c.addr, align 4
  %4 = load i32* %j, align 4
  %5 = load i32* %c.addr, align 4
  %mul = mul nsw i32 %5, %4
  store i32 %mul, i32* %c.addr, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %6 = load i32* %j, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  %7 = load i32* %c.addr, align 4
  %cmp4 = icmp sgt i32 %7, 100
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.end
  %8 = load i32* %c.addr, align 4
  store i32 %8, i32* %retval
  br label %return

if.end:                                           ; preds = %for.end
  br label %for.inc5

for.inc5:                                         ; preds = %if.end
  %9 = load i32* %i, align 4
  %inc6 = add nsw i32 %9, 1
  store i32 %inc6, i32* %i, align 4
  br label %for.cond

for.end7:                                         ; preds = %for.cond
  %10 = load i32* %c.addr, align 4
  %inc8 = add nsw i32 %10, 1
  store i32 %inc8, i32* %c.addr, align 4
  %11 = load i32* %c.addr, align 4
  store i32 %11, i32* %retval
  br label %return

return:                                           ; preds = %for.end7, %if.then
  %12 = load i32* %retval
  ret i32 %12
}

define i32 @add() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  store i32 0, i32* %sum, align 4
  store i32 1, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %1 = load i32* %i, align 4
  %mul = mul nsw i32 %0, %1
  %cmp = icmp slt i32 %mul, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4
  %3 = load i32* %sum, align 4
  %add = add nsw i32 %3, %2
  store i32 %add, i32* %sum, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32* %sum, align 4
  %cmp1 = icmp sgt i32 %5, 10000
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.end
  store i32 10000, i32* %retval
  br label %return

if.end:                                           ; preds = %for.end
  %6 = load i32* %sum, align 4
  store i32 %6, i32* %retval
  br label %return

return:                                           ; preds = %if.end, %if.then
  %7 = load i32* %retval
  ret i32 %7
}

define i32 @mul(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %product = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  store i32 1, i32* %product, align 4
  store i32 2, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32* %i, align 4
  %1 = load i32* %n.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32* %i, align 4
  %3 = load i32* %product, align 4
  %mul = mul nsw i32 %3, %2
  store i32 %mul, i32* %product, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %5 = load i32* %product, align 4
  ret i32 %5
}

define i32 @bar(i32 %n) nounwind uwtable {
entry:
  %n.addr = alloca i32, align 4
  %ret = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32* %n.addr, align 4
  %cmp = icmp sgt i32 %0, 10
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 10, i32* %ret, align 4
  br label %if.end

if.else:                                          ; preds = %entry
  %1 = load i32* %n.addr, align 4
  %div = sdiv i32 %1, 2
  store i32 %div, i32* %ret, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = load i32* %ret, align 4
  ret i32 %2
}

define i32 @main() nounwind uwtable {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  %call = call i32 @add()
  %call1 = call i32 @foo(i32 %call)
  %call2 = call i32 @mul(i32 10)
  %0 = load i32* %retval
  ret i32 %0
}

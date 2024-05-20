; ModuleID = 'llvm_code_example/test/loop.cpp'
source_filename = "llvm_code_example/test/loop.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local noundef i32 @_Z7runLoopjjj(i32 noundef %X, i32 noundef %Y, i32 noundef %Z) #0 {
entry:
  %X.addr = alloca i32, align 4
  %Y.addr = alloca i32, align 4
  %Z.addr = alloca i32, align 4
  %Count = alloca i32, align 4
  %I = alloca i32, align 4
  %J = alloca i32, align 4
  %K = alloca i32, align 4
  %J11 = alloca i32, align 4
  %K22 = alloca i32, align 4
  store i32 %X, ptr %X.addr, align 4
  store i32 %Y, ptr %Y.addr, align 4
  store i32 %Z, ptr %Z.addr, align 4
  store i32 0, ptr %Count, align 4
  store i32 0, ptr %I, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc19, %entry
  %0 = load i32, ptr %I, align 4
  %1 = load i32, ptr %X.addr, align 4
  %cmp = icmp ult i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end21

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %J, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc8, %for.body
  %2 = load i32, ptr %J, align 4
  %3 = load i32, ptr %Y.addr, align 4
  %cmp2 = icmp ult i32 %2, %3
  br i1 %cmp2, label %for.body3, label %for.end10

for.body3:                                        ; preds = %for.cond1
  store i32 0, ptr %K, align 4
  br label %for.cond4

for.cond4:                                        ; preds = %for.inc, %for.body3
  %4 = load i32, ptr %K, align 4
  %5 = load i32, ptr %Z.addr, align 4
  %cmp5 = icmp ult i32 %4, %5
  br i1 %cmp5, label %for.body6, label %for.end

for.body6:                                        ; preds = %for.cond4
  %6 = load i32, ptr %Count, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, ptr %Count, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %7 = load i32, ptr %K, align 4
  %inc7 = add i32 %7, 1
  store i32 %inc7, ptr %K, align 4
  br label %for.cond4, !llvm.loop !6

for.end:                                          ; preds = %for.cond4
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %8 = load i32, ptr %J, align 4
  %inc9 = add i32 %8, 1
  store i32 %inc9, ptr %J, align 4
  br label %for.cond1, !llvm.loop !8

for.end10:                                        ; preds = %for.cond1
  store i32 0, ptr %J11, align 4
  br label %for.cond12

for.cond12:                                       ; preds = %for.inc16, %for.end10
  %9 = load i32, ptr %J11, align 4
  %10 = load i32, ptr %Y.addr, align 4
  %cmp13 = icmp ult i32 %9, %10
  br i1 %cmp13, label %for.body14, label %for.end18

for.body14:                                       ; preds = %for.cond12
  %11 = load i32, ptr %Count, align 4
  %inc15 = add nsw i32 %11, 1
  store i32 %inc15, ptr %Count, align 4
  br label %for.inc16

for.inc16:                                        ; preds = %for.body14
  %12 = load i32, ptr %J11, align 4
  %inc17 = add i32 %12, 1
  store i32 %inc17, ptr %J11, align 4
  br label %for.cond12, !llvm.loop !9

for.end18:                                        ; preds = %for.cond12
  br label %for.inc19

for.inc19:                                        ; preds = %for.end18
  %13 = load i32, ptr %I, align 4
  %inc20 = add i32 %13, 1
  store i32 %inc20, ptr %I, align 4
  br label %for.cond, !llvm.loop !10

for.end21:                                        ; preds = %for.cond
  store i32 0, ptr %K22, align 4
  br label %for.cond23

for.cond23:                                       ; preds = %for.inc27, %for.end21
  %14 = load i32, ptr %K22, align 4
  %15 = load i32, ptr %Z.addr, align 4
  %cmp24 = icmp ult i32 %14, %15
  br i1 %cmp24, label %for.body25, label %for.end29

for.body25:                                       ; preds = %for.cond23
  %16 = load i32, ptr %Count, align 4
  %inc26 = add nsw i32 %16, 1
  store i32 %inc26, ptr %Count, align 4
  br label %for.inc27

for.inc27:                                        ; preds = %for.body25
  %17 = load i32, ptr %K22, align 4
  %inc28 = add i32 %17, 1
  store i32 %inc28, ptr %K22, align 4
  br label %for.cond23, !llvm.loop !11

for.end29:                                        ; preds = %for.cond23
  %18 = load i32, ptr %Count, align 4
  ret i32 %18
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.0.0 (++20240422031707+cce4dc7b7a80-1~exp1~20240422151838.1635)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}

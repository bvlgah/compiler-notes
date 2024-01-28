define i32 @branchfun() {
entry:
  %a_ptr = alloca i32, align 4
  %b_ptr = alloca i32, align 4
  %c_ptr = alloca i32, align 4
  %d_ptr = alloca i32, align 4
  %x_ptr = alloca i32, align 4
  store i32 3, ptr %a_ptr, align 4
  store i32 5, ptr %b_ptr, align 4
  store i32 4, ptr %d_ptr, align 4
  store i32 100, ptr %x_ptr, align 4
  %t0 = load i32, ptr %a_ptr, align 4
  %t1 = load i32, ptr %b_ptr, align 4
  %t2 = icmp sgt i32 %t0, %t1
  br i1 %t2, label %if_true, label %end_if
if_true:
  %t3 = load i32, ptr %a_ptr, align 4
  %t4 = load i32, ptr %b_ptr, align 4
  %t5 = add i32 %t3, %t4
  store i32 %t5, ptr %c_ptr, align 4
  store i32 2, ptr %d_ptr, align 4
  br label %end_if
end_if:
  store i32 4, ptr %c_ptr
  %t6 = load i32, ptr %b_ptr, align 4
  %t7 = load i32, ptr %d_ptr, align 4
  %t8 = load i32, ptr %c_ptr, align 4
  %t9 = mul i32 %t6, %t7
  %t10 = add i32 %t9, %t8
  ret i32 %t10
}

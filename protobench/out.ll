; ModuleID = 'main'
source_filename = "main"

declare i64 @input()

declare void @printint(i64)

define void @cssjit_main() {
entry:
  %0 = alloca i64, i64 0
  store i64 0, i64* %0
  br label %head

head:                                             ; preds = %b_5, %b_2, %b_1, %body, %entry
  %1 = call i64 @input()
  %cmp = icmp eq i64 %1, 0
  br i1 %cmp, label %exit, label %body

body:                                             ; preds = %head
  switch i64 %1, label %head [
    i64 1, label %b_1
    i64 2, label %b_2
    i64 5, label %b_5
  ]

exit:                                             ; preds = %head
  %2 = load i64, i64* %0
  call void @printint(i64 %2)
  ret void

b_1:                                              ; preds = %body
  %3 = load i64, i64* %0
  %4 = add i64 %3, 6
  store i64 %4, i64* %0
  br label %head

b_2:                                              ; preds = %body
  %5 = load i64, i64* %0
  %6 = add i64 %5, 4
  store i64 %6, i64* %0
  br label %head

b_5:                                              ; preds = %body
  %7 = load i64, i64* %0
  %8 = add i64 %7, 24
  store i64 %8, i64* %0
  br label %head
}

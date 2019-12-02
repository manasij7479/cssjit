; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [2 x i8] c"x\00"
@1 = private unnamed_addr constant [2 x i8] c"y\00"
@2 = private unnamed_addr constant [2 x i8] c"z\00"
@3 = private unnamed_addr constant [5 x i8] c"\2242\22\00"
@4 = private unnamed_addr constant [6 x i8] c"width\00"
@5 = private unnamed_addr constant [2 x i8] c"0\00"
@6 = private unnamed_addr constant [9 x i8] c"whatever\00"
@7 = private unnamed_addr constant [3 x i8] c"42\00"

declare void @printrule(i8*, i8*)

declare i64 @hash(i8*)

declare i32 @eq(i8*)

define void @match(i8*) {
entry:
  %1 = call i64 @hash(i8* %0)
  switch i64 %1, label %exit [
    i64 4993892634952068459, label %a
    i64 5876027268818684049, label %table
  ]

exit:                                             ; preds = %entry, %table, %a
  ret void

a:                                                ; preds = %entry
  call void @printrule(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @0, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0))
  call void @printrule(i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0))
  br label %exit

table:                                            ; preds = %entry
  call void @printrule(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @4, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @5, i32 0, i32 0))
  call void @printrule(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @6, i32 0, i32 0), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @7, i32 0, i32 0))
  br label %exit
}

; ModuleID = '/home/ubuntu/projects/llvm-test-suite/MultiSource/Applications/ClamAV/zlib_inffast.c'
source_filename = "/home/ubuntu/projects/llvm-test-suite/MultiSource/Applications/ClamAV/zlib_inffast.c"
target datalayout = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128"
target triple = "riscv64-unknown-linux-gnu"

%struct.code = type { i8, i8, i16 }

@z_verbose = external local_unnamed_addr global i32, align 4
@stderr = external local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [31 x i8] c"inflate:         literal '%c'\0A\00", align 1
@.str.1 = private unnamed_addr constant [33 x i8] c"inflate:         literal 0x%02x\0A\00", align 1
@.str.2 = private unnamed_addr constant [27 x i8] c"inflate:         hold %lu\0A\00", align 1
@.str.3 = private unnamed_addr constant [24 x i8] c"inflate:         op %u\0A\00", align 1
@.str.4 = private unnamed_addr constant [28 x i8] c"inflate:         length %u\0A\00", align 1
@.str.5 = private unnamed_addr constant [26 x i8] c"inflate:         bits %u\0A\00", align 1
@.str.6 = private unnamed_addr constant [27 x i8] c"inflate:         dmask %u\0A\00", align 1
@.str.7 = private unnamed_addr constant [31 x i8] c"inflate:         here.val %hu\0A\00", align 1
@.str.8 = private unnamed_addr constant [26 x i8] c"inflate:         dist %u\0A\00", align 1
@.str.9 = private unnamed_addr constant [30 x i8] c"inflate:         distance %u\0A\00", align 1
@.str.10 = private unnamed_addr constant [30 x i8] c"invalid distance too far back\00", align 1
@.str.11 = private unnamed_addr constant [22 x i8] c"invalid distance code\00", align 1
@.str.12 = private unnamed_addr constant [31 x i8] c"inflate:         end of block\0A\00", align 1
@.str.13 = private unnamed_addr constant [28 x i8] c"invalid literal/length code\00", align 1

; Function Attrs: nofree nounwind uwtable
define dso_local void @inflate_fast(ptr nocapture noundef %strm, i32 noundef signext %start) local_unnamed_addr #0 {
entry:
  %state1 = getelementptr inbounds i8, ptr %strm, i64 56
  %0 = load ptr, ptr %state1, align 8, !tbaa !9
  %1 = load ptr, ptr %strm, align 8, !tbaa !16
  %avail_in = getelementptr inbounds i8, ptr %strm, i64 8
  %2 = load i32, ptr %avail_in, align 8, !tbaa !17
  %sub = add i32 %2, -5
  %idx.ext = zext i32 %sub to i64
  %add.ptr = getelementptr inbounds i8, ptr %1, i64 %idx.ext
  %next_out = getelementptr inbounds i8, ptr %strm, i64 24
  %3 = load ptr, ptr %next_out, align 8, !tbaa !18
  %avail_out = getelementptr inbounds i8, ptr %strm, i64 32
  %4 = load i32, ptr %avail_out, align 8, !tbaa !19
  %sub2 = sub i32 %start, %4
  %idx.ext3 = zext i32 %sub2 to i64
  %idx.neg = sub nsw i64 0, %idx.ext3
  %add.ptr4 = getelementptr inbounds i8, ptr %3, i64 %idx.neg
  %sub6 = add i32 %4, -257
  %idx.ext7 = zext i32 %sub6 to i64
  %add.ptr8 = getelementptr inbounds i8, ptr %3, i64 %idx.ext7
  %wsize9 = getelementptr inbounds i8, ptr %0, i64 60
  %5 = load i32, ptr %wsize9, align 4, !tbaa !20
  %whave10 = getelementptr inbounds i8, ptr %0, i64 64
  %6 = load i32, ptr %whave10, align 8, !tbaa !22
  %wnext11 = getelementptr inbounds i8, ptr %0, i64 68
  %7 = load i32, ptr %wnext11, align 4, !tbaa !23
  %window12 = getelementptr inbounds i8, ptr %0, i64 72
  %8 = load ptr, ptr %window12, align 8, !tbaa !24
  %hold13 = getelementptr inbounds i8, ptr %0, i64 80
  %9 = load i64, ptr %hold13, align 8, !tbaa !25
  %bits14 = getelementptr inbounds i8, ptr %0, i64 88
  %10 = load i32, ptr %bits14, align 8, !tbaa !26
  %lencode = getelementptr inbounds i8, ptr %0, i64 104
  %11 = load ptr, ptr %lencode, align 8, !tbaa !27
  %distcode = getelementptr inbounds i8, ptr %0, i64 112
  %12 = load ptr, ptr %distcode, align 8, !tbaa !28
  %lenbits = getelementptr inbounds i8, ptr %0, i64 120
  %13 = load i32, ptr %lenbits, align 8, !tbaa !29
  %notmask = shl nsw i32 -1, %13
  %sub15 = xor i32 %notmask, -1
  %distbits = getelementptr inbounds i8, ptr %0, i64 124
  %14 = load i32, ptr %distbits, align 4, !tbaa !30
  %notmask670 = shl nsw i32 -1, %14
  %sub17 = xor i32 %notmask670, -1
  %conv26 = zext nneg i32 %sub15 to i64
  %conv124 = zext nneg i32 %sub17 to i64
  %sub.ptr.rhs.cast = ptrtoint ptr %add.ptr4 to i64
  %sane = getelementptr inbounds i8, ptr %0, i64 7144
  %cmp212 = icmp eq i32 %7, 0
  %add234 = add i32 %7, %5
  br label %do.body

do.body:                                          ; preds = %do.cond380, %entry
  %bits.0 = phi i32 [ %10, %entry ], [ %bits.8, %do.cond380 ]
  %hold.0 = phi i64 [ %9, %entry ], [ %hold.8, %do.cond380 ]
  %out.0 = phi ptr [ %3, %entry ], [ %out.8, %do.cond380 ]
  %in.0 = phi ptr [ %1, %entry ], [ %in.6, %do.cond380 ]
  %cmp = icmp ult i32 %bits.0, 15
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %do.body
  %incdec.ptr = getelementptr inbounds i8, ptr %in.0, i64 1
  %15 = load i8, ptr %in.0, align 1, !tbaa !31
  %conv = zext i8 %15 to i64
  %sh_prom = zext nneg i32 %bits.0 to i64
  %shl18 = shl nuw nsw i64 %conv, %sh_prom
  %add = add i64 %shl18, %hold.0
  %add19 = add nuw nsw i32 %bits.0, 8
  %incdec.ptr20 = getelementptr inbounds i8, ptr %in.0, i64 2
  %16 = load i8, ptr %incdec.ptr, align 1, !tbaa !31
  %conv21 = zext i8 %16 to i64
  %sh_prom22 = zext nneg i32 %add19 to i64
  %shl23 = shl nuw nsw i64 %conv21, %sh_prom22
  %add24 = add i64 %add, %shl23
  %add25 = or disjoint i32 %bits.0, 16
  br label %if.end

if.end:                                           ; preds = %if.then, %do.body
  %bits.1 = phi i32 [ %add25, %if.then ], [ %bits.0, %do.body ]
  %hold.1 = phi i64 [ %add24, %if.then ], [ %hold.0, %do.body ]
  %in.1 = phi ptr [ %incdec.ptr20, %if.then ], [ %in.0, %do.body ]
  %and = and i64 %hold.1, %conv26
  %here.sroa.0.0.in708 = getelementptr inbounds %struct.code, ptr %11, i64 %and
  %here.sroa.0.0709 = load i8, ptr %here.sroa.0.0.in708, align 2, !tbaa !31
  %here.sroa.8.0.in710 = getelementptr inbounds %struct.code, ptr %11, i64 %and, i32 1
  %here.sroa.8.0711 = load i8, ptr %here.sroa.8.0.in710, align 1, !tbaa !31
  %here.sroa.10.0.in712 = getelementptr inbounds %struct.code, ptr %11, i64 %and, i32 2
  %here.sroa.10.0713 = load i16, ptr %here.sroa.10.0.in712, align 2, !tbaa !32
  %conv28714 = zext i8 %here.sroa.8.0711 to i32
  %sh_prom29715 = zext nneg i8 %here.sroa.8.0711 to i64
  %shr716 = lshr i64 %hold.1, %sh_prom29715
  %sub30717 = sub i32 %bits.1, %conv28714
  %cmp33719 = icmp eq i8 %here.sroa.0.0709, 0
  br i1 %cmp33719, label %if.then35, label %if.else

if.then35:                                        ; preds = %if.then356, %if.end
  %here.sroa.10.0.lcssa = phi i16 [ %here.sroa.10.0713, %if.end ], [ %here.sroa.10.0, %if.then356 ]
  %shr.lcssa = phi i64 [ %shr716, %if.end ], [ %shr, %if.then356 ]
  %sub30.lcssa = phi i32 [ %sub30717, %if.end ], [ %sub30, %if.then356 ]
  %17 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp36 = icmp sgt i32 %17, 1
  br i1 %cmp36, label %if.then38, label %if.end48

if.then38:                                        ; preds = %if.then35
  %18 = load ptr, ptr @stderr, align 8, !tbaa !35
  %conv39 = zext i16 %here.sroa.10.0.lcssa to i32
  %19 = add i16 %here.sroa.10.0.lcssa, -32
  %20 = icmp ult i16 %19, 95
  %cond = select i1 %20, ptr @.str, ptr @.str.1
  %call = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %18, ptr noundef nonnull %cond, i32 noundef signext %conv39) #3
  br label %if.end48

if.end48:                                         ; preds = %if.then38, %if.then35
  %conv50 = trunc i16 %here.sroa.10.0.lcssa to i8
  %incdec.ptr51 = getelementptr inbounds i8, ptr %out.0, i64 1
  store i8 %conv50, ptr %out.0, align 1, !tbaa !31
  br label %do.cond380

if.else:                                          ; preds = %if.end, %if.then356
  %conv32723.in = phi i8 [ %here.sroa.0.0, %if.then356 ], [ %here.sroa.0.0709, %if.end ]
  %sub30722 = phi i32 [ %sub30, %if.then356 ], [ %sub30717, %if.end ]
  %shr721 = phi i64 [ %shr, %if.then356 ], [ %shr716, %if.end ]
  %here.sroa.10.0720 = phi i16 [ %here.sroa.10.0, %if.then356 ], [ %here.sroa.10.0713, %if.end ]
  %conv32723 = zext i8 %conv32723.in to i32
  %and52 = and i32 %conv32723, 16
  %tobool.not = icmp eq i32 %and52, 0
  br i1 %tobool.not, label %if.else352, label %if.then53

if.then53:                                        ; preds = %if.else
  %21 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp54 = icmp sgt i32 %21, 1
  br i1 %cmp54, label %if.end58, label %if.end63

if.end58:                                         ; preds = %if.then53
  %22 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call57 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %22, ptr noundef nonnull @.str.2, i64 noundef %shr721) #3
  %.pr = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp59 = icmp sgt i32 %.pr, 1
  br i1 %cmp59, label %if.then61, label %if.end63

if.then61:                                        ; preds = %if.end58
  %23 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call62 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %23, ptr noundef nonnull @.str.3, i32 noundef signext %conv32723) #3
  br label %if.end63

if.end63:                                         ; preds = %if.then53, %if.then61, %if.end58
  %conv65 = zext i16 %here.sroa.10.0720 to i32
  %and66 = and i32 %conv32723, 15
  %tobool67.not = icmp eq i32 %and66, 0
  br i1 %tobool67.not, label %if.end97, label %if.then68

if.then68:                                        ; preds = %if.end63
  %cmp69 = icmp ult i32 %sub30722, %and66
  br i1 %cmp69, label %if.then71, label %if.end78

if.then71:                                        ; preds = %if.then68
  %incdec.ptr72 = getelementptr inbounds i8, ptr %in.1, i64 1
  %24 = load i8, ptr %in.1, align 1, !tbaa !31
  %conv73 = zext i8 %24 to i64
  %sh_prom74 = zext nneg i32 %sub30722 to i64
  %shl75 = shl nuw nsw i64 %conv73, %sh_prom74
  %add76 = add i64 %shl75, %shr721
  %add77 = add nuw nsw i32 %sub30722, 8
  br label %if.end78

if.end78:                                         ; preds = %if.then71, %if.then68
  %bits.3 = phi i32 [ %add77, %if.then71 ], [ %sub30722, %if.then68 ]
  %hold.3 = phi i64 [ %add76, %if.then71 ], [ %shr721, %if.then68 ]
  %in.2 = phi ptr [ %incdec.ptr72, %if.then71 ], [ %in.1, %if.then68 ]
  %25 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp79 = icmp sgt i32 %25, 1
  br i1 %cmp79, label %if.end83, label %if.end88

if.end83:                                         ; preds = %if.end78
  %26 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call82 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %26, ptr noundef nonnull @.str.2, i64 noundef %hold.3) #3
  %.pr677 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp84 = icmp sgt i32 %.pr677, 1
  br i1 %cmp84, label %if.then86, label %if.end88

if.then86:                                        ; preds = %if.end83
  %27 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call87 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %27, ptr noundef nonnull @.str.3, i32 noundef signext %and66) #3
  br label %if.end88

if.end88:                                         ; preds = %if.end78, %if.then86, %if.end83
  %conv89 = trunc i64 %hold.3 to i32
  %notmask672 = shl nsw i32 -1, %and66
  %sub91 = xor i32 %notmask672, -1
  %and92 = and i32 %conv89, %sub91
  %add93 = add nuw nsw i32 %and92, %conv65
  %sh_prom94 = zext nneg i32 %and66 to i64
  %shr95 = lshr i64 %hold.3, %sh_prom94
  %sub96 = sub i32 %bits.3, %and66
  br label %if.end97

if.end97:                                         ; preds = %if.end88, %if.end63
  %bits.4 = phi i32 [ %sub96, %if.end88 ], [ %sub30722, %if.end63 ]
  %hold.4 = phi i64 [ %shr95, %if.end88 ], [ %shr721, %if.end63 ]
  %in.3 = phi ptr [ %in.2, %if.end88 ], [ %in.1, %if.end63 ]
  %len.0 = phi i32 [ %add93, %if.end88 ], [ %conv65, %if.end63 ]
  %28 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp98 = icmp sgt i32 %28, 1
  br i1 %cmp98, label %if.end102, label %if.end107

if.end102:                                        ; preds = %if.end97
  %29 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call101 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %29, ptr noundef nonnull @.str.4, i32 noundef signext %len.0) #3
  %.pr679 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp103 = icmp sgt i32 %.pr679, 1
  br i1 %cmp103, label %if.then105, label %if.end107

if.then105:                                       ; preds = %if.end102
  %30 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call106 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %30, ptr noundef nonnull @.str.2, i64 noundef %hold.4) #3
  br label %if.end107

if.end107:                                        ; preds = %if.end97, %if.then105, %if.end102
  %cmp108 = icmp ult i32 %bits.4, 15
  br i1 %cmp108, label %if.then110, label %if.end123

if.then110:                                       ; preds = %if.end107
  %incdec.ptr111 = getelementptr inbounds i8, ptr %in.3, i64 1
  %31 = load i8, ptr %in.3, align 1, !tbaa !31
  %conv112 = zext i8 %31 to i64
  %sh_prom113 = zext nneg i32 %bits.4 to i64
  %shl114 = shl nuw nsw i64 %conv112, %sh_prom113
  %add115 = add i64 %shl114, %hold.4
  %add116 = add nuw nsw i32 %bits.4, 8
  %incdec.ptr117 = getelementptr inbounds i8, ptr %in.3, i64 2
  %32 = load i8, ptr %incdec.ptr111, align 1, !tbaa !31
  %conv118 = zext i8 %32 to i64
  %sh_prom119 = zext nneg i32 %add116 to i64
  %shl120 = shl nuw nsw i64 %conv118, %sh_prom119
  %add121 = add i64 %add115, %shl120
  %add122 = or disjoint i32 %bits.4, 16
  br label %if.end123

if.end123:                                        ; preds = %if.then110, %if.end107
  %bits.5 = phi i32 [ %add122, %if.then110 ], [ %bits.4, %if.end107 ]
  %hold.5 = phi i64 [ %add121, %if.then110 ], [ %hold.4, %if.end107 ]
  %in.4 = phi ptr [ %incdec.ptr117, %if.then110 ], [ %in.3, %if.end107 ]
  %and125 = and i64 %hold.5, %conv124
  %arrayidx126 = getelementptr inbounds %struct.code, ptr %12, i64 %and125
  %here.sroa.0.0.copyload519 = load i8, ptr %arrayidx126, align 2, !tbaa !31
  %here.sroa.8.0.arrayidx126.sroa_idx = getelementptr inbounds i8, ptr %arrayidx126, i64 1
  %here.sroa.8.0.copyload522 = load i8, ptr %here.sroa.8.0.arrayidx126.sroa_idx, align 1, !tbaa !31
  %here.sroa.10.0.arrayidx126.sroa_idx = getelementptr inbounds i8, ptr %arrayidx126, i64 2
  %here.sroa.10.0.copyload526 = load i16, ptr %here.sroa.10.0.arrayidx126.sroa_idx, align 2, !tbaa !32
  %33 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp127 = icmp sgt i32 %33, 1
  br i1 %cmp127, label %if.end131, label %if.end141

if.end131:                                        ; preds = %if.end123
  %34 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call130 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %34, ptr noundef nonnull @.str.5, i32 noundef signext %bits.5) #3
  %.pr681 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp132 = icmp sgt i32 %.pr681, 1
  br i1 %cmp132, label %if.end136, label %if.end141

if.end136:                                        ; preds = %if.end131
  %35 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call135 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %35, ptr noundef nonnull @.str.2, i64 noundef %hold.5) #3
  %.pr683 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp137 = icmp sgt i32 %.pr683, 1
  br i1 %cmp137, label %if.then139, label %if.end141

if.then139:                                       ; preds = %if.end136
  %36 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call140 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %36, ptr noundef nonnull @.str.6, i32 noundef signext %sub17) #3
  br label %if.end141

if.end141:                                        ; preds = %if.end123, %if.end131, %if.then139, %if.end136
  %conv143726 = zext i8 %here.sroa.8.0.copyload522 to i32
  %sh_prom144727 = zext nneg i8 %here.sroa.8.0.copyload522 to i64
  %shr145728 = lshr i64 %hold.5, %sh_prom144727
  %sub146729 = sub i32 %bits.5, %conv143726
  %conv148730 = zext i8 %here.sroa.0.0.copyload519 to i32
  %and149731 = and i32 %conv148730, 16
  %tobool150.not732 = icmp eq i32 %and149731, 0
  br i1 %tobool150.not732, label %if.else335, label %if.then151

if.then151:                                       ; preds = %if.then339, %if.end141
  %here.sroa.10.1.lcssa = phi i16 [ %here.sroa.10.0.copyload526, %if.end141 ], [ %here.sroa.10.0.copyload527, %if.then339 ]
  %shr145.lcssa = phi i64 [ %shr145728, %if.end141 ], [ %shr145, %if.then339 ]
  %sub146.lcssa = phi i32 [ %sub146729, %if.end141 ], [ %sub146, %if.then339 ]
  %conv148.lcssa = phi i32 [ %conv148730, %if.end141 ], [ %conv148, %if.then339 ]
  %37 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp152 = icmp sgt i32 %37, 1
  br i1 %cmp152, label %if.end158, label %if.end158.thread

if.end158.thread:                                 ; preds = %if.then151
  %conv160686 = zext i16 %here.sroa.10.1.lcssa to i32
  br label %if.end165

if.end158:                                        ; preds = %if.then151
  %38 = load ptr, ptr @stderr, align 8, !tbaa !35
  %conv156 = zext i16 %here.sroa.10.1.lcssa to i32
  %call157 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %38, ptr noundef nonnull @.str.7, i32 noundef signext %conv156) #3
  %.pr685 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp161 = icmp sgt i32 %.pr685, 1
  br i1 %cmp161, label %if.then163, label %if.end165

if.then163:                                       ; preds = %if.end158
  %39 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call164 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %39, ptr noundef nonnull @.str.8, i32 noundef signext %conv156) #3
  br label %if.end165

if.end165:                                        ; preds = %if.end158.thread, %if.then163, %if.end158
  %conv160688 = phi i32 [ %conv160686, %if.end158.thread ], [ %conv156, %if.then163 ], [ %conv156, %if.end158 ]
  %and166 = and i32 %conv148.lcssa, 15
  %cmp167 = icmp ult i32 %sub146.lcssa, %and166
  br i1 %cmp167, label %if.then169, label %if.end186

if.then169:                                       ; preds = %if.end165
  %incdec.ptr170 = getelementptr inbounds i8, ptr %in.4, i64 1
  %40 = load i8, ptr %in.4, align 1, !tbaa !31
  %conv171 = zext i8 %40 to i64
  %sh_prom172 = zext nneg i32 %sub146.lcssa to i64
  %shl173 = shl nuw nsw i64 %conv171, %sh_prom172
  %add174 = add i64 %shl173, %shr145.lcssa
  %add175 = add nuw nsw i32 %sub146.lcssa, 8
  %cmp176 = icmp ult i32 %add175, %and166
  br i1 %cmp176, label %if.then178, label %if.end186

if.then178:                                       ; preds = %if.then169
  %incdec.ptr179 = getelementptr inbounds i8, ptr %in.4, i64 2
  %41 = load i8, ptr %incdec.ptr170, align 1, !tbaa !31
  %conv180 = zext i8 %41 to i64
  %sh_prom181 = zext nneg i32 %add175 to i64
  %shl182 = shl nuw nsw i64 %conv180, %sh_prom181
  %add183 = add i64 %shl182, %add174
  %add184 = add nuw nsw i32 %sub146.lcssa, 16
  br label %if.end186

if.end186:                                        ; preds = %if.then169, %if.then178, %if.end165
  %bits.7 = phi i32 [ %add184, %if.then178 ], [ %add175, %if.then169 ], [ %sub146.lcssa, %if.end165 ]
  %hold.7 = phi i64 [ %add183, %if.then178 ], [ %add174, %if.then169 ], [ %shr145.lcssa, %if.end165 ]
  %in.5 = phi ptr [ %incdec.ptr179, %if.then178 ], [ %incdec.ptr170, %if.then169 ], [ %in.4, %if.end165 ]
  %conv187 = trunc i64 %hold.7 to i32
  %notmask674 = shl nsw i32 -1, %and166
  %sub189 = xor i32 %notmask674, -1
  %and190 = and i32 %conv187, %sub189
  %add191 = add nuw nsw i32 %and190, %conv160688
  %sh_prom192 = zext nneg i32 %and166 to i64
  %shr193 = lshr i64 %hold.7, %sh_prom192
  %sub194 = sub i32 %bits.7, %and166
  %42 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp195 = icmp sgt i32 %42, 1
  br i1 %cmp195, label %if.then197, label %if.end199

if.then197:                                       ; preds = %if.end186
  %43 = load ptr, ptr @stderr, align 8, !tbaa !35
  %call198 = tail call signext i32 (ptr, ptr, ...) @fprintf(ptr noundef %43, ptr noundef nonnull @.str.9, i32 noundef signext %add191) #3
  br label %if.end199

if.end199:                                        ; preds = %if.then197, %if.end186
  %sub.ptr.lhs.cast = ptrtoint ptr %out.0 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %conv200 = trunc i64 %sub.ptr.sub to i32
  %cmp201 = icmp ugt i32 %add191, %conv200
  br i1 %cmp201, label %if.then203, label %if.else307

if.then203:                                       ; preds = %if.end199
  %sub204 = sub nsw i32 %add191, %conv200
  %cmp205 = icmp ugt i32 %sub204, %6
  br i1 %cmp205, label %if.then207, label %if.end211

if.then207:                                       ; preds = %if.then203
  %44 = load i32, ptr %sane, align 8, !tbaa !36
  %tobool208.not = icmp eq i32 %44, 0
  br i1 %tobool208.not, label %if.end211, label %if.then209

if.then209:                                       ; preds = %if.then207
  %msg = getelementptr inbounds i8, ptr %strm, i64 48
  store ptr @.str.10, ptr %msg, align 8, !tbaa !37
  br label %do.end387.sink.split

if.end211:                                        ; preds = %if.then207, %if.then203
  br i1 %cmp212, label %if.then214, label %if.else230

if.then214:                                       ; preds = %if.end211
  %sub215 = sub i32 %5, %sub204
  %idx.ext216 = zext i32 %sub215 to i64
  %add.ptr217 = getelementptr i8, ptr %8, i64 %idx.ext216
  %cmp218 = icmp ult i32 %sub204, %len.0
  br i1 %cmp218, label %do.body222, label %if.end286

do.body222:                                       ; preds = %if.then214, %do.body222
  %out.1 = phi ptr [ %incdec.ptr224, %do.body222 ], [ %out.0, %if.then214 ]
  %op.0 = phi i32 [ %dec, %do.body222 ], [ %sub204, %if.then214 ]
  %from.0 = phi ptr [ %incdec.ptr223, %do.body222 ], [ %add.ptr217, %if.then214 ]
  %incdec.ptr223 = getelementptr inbounds i8, ptr %from.0, i64 1
  %45 = load i8, ptr %from.0, align 1, !tbaa !31
  %incdec.ptr224 = getelementptr inbounds i8, ptr %out.1, i64 1
  store i8 %45, ptr %out.1, align 1, !tbaa !31
  %dec = add i32 %op.0, -1
  %tobool225.not = icmp eq i32 %dec, 0
  br i1 %tobool225.not, label %do.end, label %do.body222, !llvm.loop !38

do.end:                                           ; preds = %do.body222
  %sub221 = sub nsw i32 %len.0, %sub204
  %idx.ext226 = zext nneg i32 %add191 to i64
  %idx.neg227 = sub nsw i64 0, %idx.ext226
  %add.ptr228 = getelementptr inbounds i8, ptr %incdec.ptr224, i64 %idx.neg227
  br label %if.end286

if.else230:                                       ; preds = %if.end211
  %cmp231 = icmp ult i32 %7, %sub204
  br i1 %cmp231, label %if.then233, label %if.else266

if.then233:                                       ; preds = %if.else230
  %sub235 = sub i32 %add234, %sub204
  %idx.ext236 = zext i32 %sub235 to i64
  %add.ptr237 = getelementptr i8, ptr %8, i64 %idx.ext236
  %sub238 = sub i32 %sub204, %7
  %cmp239 = icmp ult i32 %sub238, %len.0
  br i1 %cmp239, label %do.body243, label %if.end286

do.body243:                                       ; preds = %if.then233, %do.body243
  %out.2 = phi ptr [ %incdec.ptr245, %do.body243 ], [ %out.0, %if.then233 ]
  %op.1 = phi i32 [ %dec247, %do.body243 ], [ %sub238, %if.then233 ]
  %from.1 = phi ptr [ %incdec.ptr244, %do.body243 ], [ %add.ptr237, %if.then233 ]
  %incdec.ptr244 = getelementptr inbounds i8, ptr %from.1, i64 1
  %46 = load i8, ptr %from.1, align 1, !tbaa !31
  %incdec.ptr245 = getelementptr i8, ptr %out.2, i64 1
  store i8 %46, ptr %out.2, align 1, !tbaa !31
  %dec247 = add i32 %op.1, -1
  %tobool248.not = icmp eq i32 %dec247, 0
  br i1 %tobool248.not, label %do.end249, label %do.body243, !llvm.loop !40

do.end249:                                        ; preds = %do.body243
  %sub242 = sub nsw i32 %len.0, %sub238
  %cmp250 = icmp ult i32 %7, %sub242
  br i1 %cmp250, label %do.body254, label %if.end286

do.body254:                                       ; preds = %do.end249, %do.body254
  %out.3 = phi ptr [ %incdec.ptr256, %do.body254 ], [ %incdec.ptr245, %do.end249 ]
  %op.2 = phi i32 [ %dec258, %do.body254 ], [ %7, %do.end249 ]
  %from.2 = phi ptr [ %incdec.ptr255, %do.body254 ], [ %8, %do.end249 ]
  %incdec.ptr255 = getelementptr inbounds i8, ptr %from.2, i64 1
  %47 = load i8, ptr %from.2, align 1, !tbaa !31
  %incdec.ptr256 = getelementptr inbounds i8, ptr %out.3, i64 1
  store i8 %47, ptr %out.3, align 1, !tbaa !31
  %dec258 = add i32 %op.2, -1
  %tobool259.not = icmp eq i32 %dec258, 0
  br i1 %tobool259.not, label %do.end260, label %do.body254, !llvm.loop !41

do.end260:                                        ; preds = %do.body254
  %sub253 = sub i32 %sub242, %7
  %idx.ext261 = zext nneg i32 %add191 to i64
  %idx.neg262 = sub nsw i64 0, %idx.ext261
  %add.ptr263 = getelementptr inbounds i8, ptr %incdec.ptr256, i64 %idx.neg262
  br label %if.end286

if.else266:                                       ; preds = %if.else230
  %sub267 = sub i32 %7, %sub204
  %idx.ext268 = zext i32 %sub267 to i64
  %add.ptr269 = getelementptr i8, ptr %8, i64 %idx.ext268
  %cmp270 = icmp ult i32 %sub204, %len.0
  br i1 %cmp270, label %do.body274, label %if.end286

do.body274:                                       ; preds = %if.else266, %do.body274
  %out.4 = phi ptr [ %incdec.ptr276, %do.body274 ], [ %out.0, %if.else266 ]
  %op.3 = phi i32 [ %dec278, %do.body274 ], [ %sub204, %if.else266 ]
  %from.3 = phi ptr [ %incdec.ptr275, %do.body274 ], [ %add.ptr269, %if.else266 ]
  %incdec.ptr275 = getelementptr inbounds i8, ptr %from.3, i64 1
  %48 = load i8, ptr %from.3, align 1, !tbaa !31
  %incdec.ptr276 = getelementptr inbounds i8, ptr %out.4, i64 1
  store i8 %48, ptr %out.4, align 1, !tbaa !31
  %dec278 = add i32 %op.3, -1
  %tobool279.not = icmp eq i32 %dec278, 0
  br i1 %tobool279.not, label %do.end280, label %do.body274, !llvm.loop !42

do.end280:                                        ; preds = %do.body274
  %sub273 = sub nsw i32 %len.0, %sub204
  %idx.ext281 = zext nneg i32 %add191 to i64
  %idx.neg282 = sub nsw i64 0, %idx.ext281
  %add.ptr283 = getelementptr inbounds i8, ptr %incdec.ptr276, i64 %idx.neg282
  br label %if.end286

if.end286:                                        ; preds = %do.end249, %do.end260, %if.then233, %do.end280, %if.else266, %if.then214, %do.end
  %out.5 = phi ptr [ %incdec.ptr224, %do.end ], [ %out.0, %if.then214 ], [ %incdec.ptr256, %do.end260 ], [ %incdec.ptr245, %do.end249 ], [ %out.0, %if.then233 ], [ %incdec.ptr276, %do.end280 ], [ %out.0, %if.else266 ]
  %len.1 = phi i32 [ %sub221, %do.end ], [ %len.0, %if.then214 ], [ %sub253, %do.end260 ], [ %sub242, %do.end249 ], [ %len.0, %if.then233 ], [ %sub273, %do.end280 ], [ %len.0, %if.else266 ]
  %from.4 = phi ptr [ %add.ptr228, %do.end ], [ %add.ptr217, %if.then214 ], [ %add.ptr263, %do.end260 ], [ %8, %do.end249 ], [ %add.ptr237, %if.then233 ], [ %add.ptr283, %do.end280 ], [ %add.ptr269, %if.else266 ]
  %cmp287741 = icmp ugt i32 %len.1, 2
  br i1 %cmp287741, label %while.body, label %while.end

while.body:                                       ; preds = %if.end286, %while.body
  %from.5744 = phi ptr [ %incdec.ptr293, %while.body ], [ %from.4, %if.end286 ]
  %len.2743 = phi i32 [ %sub295, %while.body ], [ %len.1, %if.end286 ]
  %out.6742 = phi ptr [ %incdec.ptr294, %while.body ], [ %out.5, %if.end286 ]
  %incdec.ptr289 = getelementptr inbounds i8, ptr %from.5744, i64 1
  %49 = load i8, ptr %from.5744, align 1, !tbaa !31
  %incdec.ptr290 = getelementptr inbounds i8, ptr %out.6742, i64 1
  store i8 %49, ptr %out.6742, align 1, !tbaa !31
  %incdec.ptr291 = getelementptr inbounds i8, ptr %from.5744, i64 2
  %50 = load i8, ptr %incdec.ptr289, align 1, !tbaa !31
  %incdec.ptr292 = getelementptr inbounds i8, ptr %out.6742, i64 2
  store i8 %50, ptr %incdec.ptr290, align 1, !tbaa !31
  %incdec.ptr293 = getelementptr inbounds i8, ptr %from.5744, i64 3
  %51 = load i8, ptr %incdec.ptr291, align 1, !tbaa !31
  %incdec.ptr294 = getelementptr inbounds i8, ptr %out.6742, i64 3
  store i8 %51, ptr %incdec.ptr292, align 1, !tbaa !31
  %sub295 = add i32 %len.2743, -3
  %cmp287 = icmp ugt i32 %sub295, 2
  br i1 %cmp287, label %while.body, label %while.end, !llvm.loop !43

while.end:                                        ; preds = %while.body, %if.end286
  %out.6.lcssa = phi ptr [ %out.5, %if.end286 ], [ %incdec.ptr294, %while.body ]
  %len.2.lcssa = phi i32 [ %len.1, %if.end286 ], [ %sub295, %while.body ]
  %from.5.lcssa = phi ptr [ %from.4, %if.end286 ], [ %incdec.ptr293, %while.body ]
  %tobool296.not = icmp eq i32 %len.2.lcssa, 0
  br i1 %tobool296.not, label %do.cond380, label %if.then297

if.then297:                                       ; preds = %while.end
  %52 = load i8, ptr %from.5.lcssa, align 1, !tbaa !31
  %incdec.ptr299 = getelementptr inbounds i8, ptr %out.6.lcssa, i64 1
  store i8 %52, ptr %out.6.lcssa, align 1, !tbaa !31
  %cmp300 = icmp eq i32 %len.2.lcssa, 2
  br i1 %cmp300, label %if.then302, label %do.cond380

if.then302:                                       ; preds = %if.then297
  %incdec.ptr298 = getelementptr inbounds i8, ptr %from.5.lcssa, i64 1
  %53 = load i8, ptr %incdec.ptr298, align 1, !tbaa !31
  %incdec.ptr304 = getelementptr inbounds i8, ptr %out.6.lcssa, i64 2
  store i8 %53, ptr %incdec.ptr299, align 1, !tbaa !31
  br label %do.cond380

if.else307:                                       ; preds = %if.end199
  %idx.ext308 = zext nneg i32 %add191 to i64
  %idx.neg309 = sub nsw i64 0, %idx.ext308
  %add.ptr310 = getelementptr inbounds i8, ptr %out.0, i64 %idx.neg309
  br label %do.body311

do.body311:                                       ; preds = %do.body311, %if.else307
  %out.7 = phi ptr [ %out.0, %if.else307 ], [ %incdec.ptr317, %do.body311 ]
  %len.3 = phi i32 [ %len.0, %if.else307 ], [ %sub318, %do.body311 ]
  %from.6 = phi ptr [ %add.ptr310, %if.else307 ], [ %incdec.ptr316, %do.body311 ]
  %incdec.ptr312 = getelementptr inbounds i8, ptr %from.6, i64 1
  %54 = load i8, ptr %from.6, align 1, !tbaa !31
  %incdec.ptr313 = getelementptr inbounds i8, ptr %out.7, i64 1
  store i8 %54, ptr %out.7, align 1, !tbaa !31
  %incdec.ptr314 = getelementptr inbounds i8, ptr %from.6, i64 2
  %55 = load i8, ptr %incdec.ptr312, align 1, !tbaa !31
  %incdec.ptr315 = getelementptr inbounds i8, ptr %out.7, i64 2
  store i8 %55, ptr %incdec.ptr313, align 1, !tbaa !31
  %incdec.ptr316 = getelementptr inbounds i8, ptr %from.6, i64 3
  %56 = load i8, ptr %incdec.ptr314, align 1, !tbaa !31
  %incdec.ptr317 = getelementptr inbounds i8, ptr %out.7, i64 3
  store i8 %56, ptr %incdec.ptr315, align 1, !tbaa !31
  %sub318 = add i32 %len.3, -3
  %cmp320 = icmp ugt i32 %sub318, 2
  br i1 %cmp320, label %do.body311, label %do.end322, !llvm.loop !44

do.end322:                                        ; preds = %do.body311
  %tobool323.not = icmp eq i32 %sub318, 0
  br i1 %tobool323.not, label %do.cond380, label %if.then324

if.then324:                                       ; preds = %do.end322
  %57 = load i8, ptr %incdec.ptr316, align 1, !tbaa !31
  %incdec.ptr326 = getelementptr inbounds i8, ptr %out.7, i64 4
  store i8 %57, ptr %incdec.ptr317, align 1, !tbaa !31
  %cmp327 = icmp eq i32 %sub318, 2
  br i1 %cmp327, label %if.then329, label %do.cond380

if.then329:                                       ; preds = %if.then324
  %incdec.ptr325 = getelementptr inbounds i8, ptr %from.6, i64 4
  %58 = load i8, ptr %incdec.ptr325, align 1, !tbaa !31
  %incdec.ptr331 = getelementptr inbounds i8, ptr %out.7, i64 5
  store i8 %58, ptr %incdec.ptr326, align 1, !tbaa !31
  br label %do.cond380

if.else335:                                       ; preds = %if.end141, %if.then339
  %conv148736 = phi i32 [ %conv148, %if.then339 ], [ %conv148730, %if.end141 ]
  %sub146735 = phi i32 [ %sub146, %if.then339 ], [ %sub146729, %if.end141 ]
  %shr145734 = phi i64 [ %shr145, %if.then339 ], [ %shr145728, %if.end141 ]
  %here.sroa.10.1733 = phi i16 [ %here.sroa.10.0.copyload527, %if.then339 ], [ %here.sroa.10.0.copyload526, %if.end141 ]
  %and336 = and i32 %conv148736, 64
  %cmp337 = icmp eq i32 %and336, 0
  br i1 %cmp337, label %if.then339, label %if.else348

if.then339:                                       ; preds = %if.else335
  %conv341 = zext i16 %here.sroa.10.1733 to i64
  %notmask673 = shl nsw i32 -1, %conv148736
  %sub343 = xor i32 %notmask673, -1
  %conv344 = zext nneg i32 %sub343 to i64
  %and345 = and i64 %shr145734, %conv344
  %59 = getelementptr %struct.code, ptr %12, i64 %and345
  %arrayidx347 = getelementptr %struct.code, ptr %59, i64 %conv341
  %here.sroa.0.0.copyload520 = load i8, ptr %arrayidx347, align 2, !tbaa !31
  %here.sroa.8.0.arrayidx347.sroa_idx = getelementptr inbounds i8, ptr %arrayidx347, i64 1
  %here.sroa.8.0.copyload523 = load i8, ptr %here.sroa.8.0.arrayidx347.sroa_idx, align 1, !tbaa !31
  %here.sroa.10.0.arrayidx347.sroa_idx = getelementptr inbounds i8, ptr %arrayidx347, i64 2
  %here.sroa.10.0.copyload527 = load i16, ptr %here.sroa.10.0.arrayidx347.sroa_idx, align 2, !tbaa !32
  %conv143 = zext i8 %here.sroa.8.0.copyload523 to i32
  %sh_prom144 = zext nneg i8 %here.sroa.8.0.copyload523 to i64
  %shr145 = lshr i64 %shr145734, %sh_prom144
  %sub146 = sub i32 %sub146735, %conv143
  %conv148 = zext i8 %here.sroa.0.0.copyload520 to i32
  %and149 = and i32 %conv148, 16
  %tobool150.not = icmp eq i32 %and149, 0
  br i1 %tobool150.not, label %if.else335, label %if.then151

if.else348:                                       ; preds = %if.else335
  %msg349 = getelementptr inbounds i8, ptr %strm, i64 48
  store ptr @.str.11, ptr %msg349, align 8, !tbaa !37
  br label %do.end387.sink.split

if.else352:                                       ; preds = %if.else
  %and353 = and i32 %conv32723, 64
  %cmp354 = icmp eq i32 %and353, 0
  br i1 %cmp354, label %if.then356, label %if.else365

if.then356:                                       ; preds = %if.else352
  %conv358 = zext i16 %here.sroa.10.0720 to i64
  %notmask671 = shl nsw i32 -1, %conv32723
  %sub360 = xor i32 %notmask671, -1
  %conv361 = zext nneg i32 %sub360 to i64
  %and362 = and i64 %shr721, %conv361
  %add363 = add nuw nsw i64 %and362, %conv358
  %here.sroa.0.0.in = getelementptr inbounds %struct.code, ptr %11, i64 %add363
  %here.sroa.0.0 = load i8, ptr %here.sroa.0.0.in, align 2, !tbaa !31
  %here.sroa.8.0.in = getelementptr inbounds %struct.code, ptr %11, i64 %add363, i32 1
  %here.sroa.8.0 = load i8, ptr %here.sroa.8.0.in, align 1, !tbaa !31
  %here.sroa.10.0.in = getelementptr inbounds %struct.code, ptr %11, i64 %add363, i32 2
  %here.sroa.10.0 = load i16, ptr %here.sroa.10.0.in, align 2, !tbaa !32
  %conv28 = zext i8 %here.sroa.8.0 to i32
  %sh_prom29 = zext nneg i8 %here.sroa.8.0 to i64
  %shr = lshr i64 %shr721, %sh_prom29
  %sub30 = sub i32 %sub30722, %conv28
  %cmp33 = icmp eq i8 %here.sroa.0.0, 0
  br i1 %cmp33, label %if.then35, label %if.else

if.else365:                                       ; preds = %if.else352
  %and366 = and i32 %conv32723, 32
  %tobool367.not = icmp eq i32 %and366, 0
  br i1 %tobool367.not, label %if.else375, label %if.then368

if.then368:                                       ; preds = %if.else365
  %60 = load i32, ptr @z_verbose, align 4, !tbaa !34
  %cmp369 = icmp sgt i32 %60, 1
  br i1 %cmp369, label %if.then371, label %do.end387.sink.split

if.then371:                                       ; preds = %if.then368
  %61 = load ptr, ptr @stderr, align 8, !tbaa !35
  %62 = tail call i64 @fwrite(ptr nonnull @.str.12, i64 30, i64 1, ptr %61) #4
  br label %do.end387.sink.split

if.else375:                                       ; preds = %if.else365
  %msg376 = getelementptr inbounds i8, ptr %strm, i64 48
  store ptr @.str.13, ptr %msg376, align 8, !tbaa !37
  br label %do.end387.sink.split

do.cond380:                                       ; preds = %if.end48, %do.end322, %if.then329, %if.then324, %while.end, %if.then302, %if.then297
  %bits.8 = phi i32 [ %sub30.lcssa, %if.end48 ], [ %sub194, %if.then302 ], [ %sub194, %if.then297 ], [ %sub194, %while.end ], [ %sub194, %if.then329 ], [ %sub194, %if.then324 ], [ %sub194, %do.end322 ]
  %hold.8 = phi i64 [ %shr.lcssa, %if.end48 ], [ %shr193, %if.then302 ], [ %shr193, %if.then297 ], [ %shr193, %while.end ], [ %shr193, %if.then329 ], [ %shr193, %if.then324 ], [ %shr193, %do.end322 ]
  %out.8 = phi ptr [ %incdec.ptr51, %if.end48 ], [ %incdec.ptr304, %if.then302 ], [ %incdec.ptr299, %if.then297 ], [ %out.6.lcssa, %while.end ], [ %incdec.ptr331, %if.then329 ], [ %incdec.ptr326, %if.then324 ], [ %incdec.ptr317, %do.end322 ]
  %in.6 = phi ptr [ %in.1, %if.end48 ], [ %in.5, %if.then302 ], [ %in.5, %if.then297 ], [ %in.5, %while.end ], [ %in.5, %if.then329 ], [ %in.5, %if.then324 ], [ %in.5, %do.end322 ]
  %cmp381 = icmp ult ptr %in.6, %add.ptr
  %cmp384 = icmp ult ptr %out.8, %add.ptr8
  %63 = select i1 %cmp381, i1 %cmp384, i1 false
  br i1 %63, label %do.body, label %do.end387, !llvm.loop !45

do.end387.sink.split:                             ; preds = %if.then368, %if.then371, %if.then209, %if.else348, %if.else375
  %.sink = phi i32 [ 16209, %if.else375 ], [ 16209, %if.else348 ], [ 16209, %if.then209 ], [ 16191, %if.then371 ], [ 16191, %if.then368 ]
  %bits.9.ph = phi i32 [ %sub30722, %if.else375 ], [ %sub146735, %if.else348 ], [ %sub194, %if.then209 ], [ %sub30722, %if.then371 ], [ %sub30722, %if.then368 ]
  %hold.9.ph = phi i64 [ %shr721, %if.else375 ], [ %shr145734, %if.else348 ], [ %shr193, %if.then209 ], [ %shr721, %if.then371 ], [ %shr721, %if.then368 ]
  %in.7.ph = phi ptr [ %in.1, %if.else375 ], [ %in.4, %if.else348 ], [ %in.5, %if.then209 ], [ %in.1, %if.then371 ], [ %in.1, %if.then368 ]
  %mode377 = getelementptr inbounds i8, ptr %0, i64 8
  store i32 %.sink, ptr %mode377, align 8, !tbaa !46
  br label %do.end387

do.end387:                                        ; preds = %do.cond380, %do.end387.sink.split
  %bits.9 = phi i32 [ %bits.9.ph, %do.end387.sink.split ], [ %bits.8, %do.cond380 ]
  %hold.9 = phi i64 [ %hold.9.ph, %do.end387.sink.split ], [ %hold.8, %do.cond380 ]
  %out.9 = phi ptr [ %out.0, %do.end387.sink.split ], [ %out.8, %do.cond380 ]
  %in.7 = phi ptr [ %in.7.ph, %do.end387.sink.split ], [ %in.6, %do.cond380 ]
  %shr388 = lshr i32 %bits.9, 3
  %idx.ext389 = zext nneg i32 %shr388 to i64
  %idx.neg390 = sub nsw i64 0, %idx.ext389
  %add.ptr391 = getelementptr inbounds i8, ptr %in.7, i64 %idx.neg390
  store ptr %add.ptr391, ptr %strm, align 8, !tbaa !16
  store ptr %out.9, ptr %next_out, align 8, !tbaa !18
  %sub.ptr.lhs.cast402 = ptrtoint ptr %add.ptr to i64
  %sub.ptr.rhs.cast403 = ptrtoint ptr %add.ptr391 to i64
  %sub.ptr.sub404 = sub i64 %sub.ptr.lhs.cast402, %sub.ptr.rhs.cast403
  %64 = trunc i64 %sub.ptr.sub404 to i32
  %conv411 = add i32 %64, 5
  store i32 %conv411, ptr %avail_in, align 8, !tbaa !17
  %sub.ptr.lhs.cast416 = ptrtoint ptr %add.ptr8 to i64
  %sub.ptr.rhs.cast417 = ptrtoint ptr %out.9 to i64
  %sub.ptr.sub418 = sub i64 %sub.ptr.lhs.cast416, %sub.ptr.rhs.cast417
  %sub393 = and i32 %bits.9, 7
  %notmask675 = shl nsw i32 -1, %sub393
  %sub395 = xor i32 %notmask675, -1
  %conv396 = zext nneg i32 %sub395 to i64
  %and397 = and i64 %hold.9, %conv396
  %65 = trunc i64 %sub.ptr.sub418 to i32
  %conv427 = add i32 %65, 257
  store i32 %conv427, ptr %avail_out, align 8, !tbaa !19
  store i64 %and397, ptr %hold13, align 8, !tbaa !25
  store i32 %sub393, ptr %bits14, align 8, !tbaa !26
  ret void
}

; Function Attrs: nofree nounwind
declare noundef signext i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare noundef i64 @fwrite(ptr nocapture noundef, i64 noundef, i64 noundef, ptr nocapture noundef) local_unnamed_addr #2

attributes #0 = { nofree nounwind uwtable "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+64bit,+a,+c,+d,+f,+m,+relax,+zicsr,-e,-experimental-smmpm,-experimental-smnpm,-experimental-ssnpm,-experimental-sspm,-experimental-ssqosid,-experimental-supm,-experimental-zaamo,-experimental-zabha,-experimental-zalasr,-experimental-zalrsc,-experimental-zfbfmin,-experimental-zicfilp,-experimental-zicfiss,-experimental-ztso,-experimental-zvfbfmin,-experimental-zvfbfwma,-h,-shcounterenw,-shgatpa,-shtvala,-shvsatpa,-shvstvala,-shvstvecd,-smaia,-smepmp,-ssaia,-ssccptr,-sscofpmf,-sscounterenw,-ssstateen,-ssstrict,-sstc,-sstvala,-sstvecd,-ssu64xl,-svade,-svadu,-svbare,-svinval,-svnapot,-svpbmt,-v,-xcvalu,-xcvbi,-xcvbitmanip,-xcvelw,-xcvmac,-xcvmem,-xcvsimd,-xsfcease,-xsfvcp,-xsfvfnrclipxfqf,-xsfvfwmaccqqq,-xsfvqmaccdod,-xsfvqmaccqoq,-xsifivecdiscarddlone,-xsifivecflushdlone,-xtheadba,-xtheadbb,-xtheadbs,-xtheadcmo,-xtheadcondmov,-xtheadfmemidx,-xtheadmac,-xtheadmemidx,-xtheadmempair,-xtheadsync,-xtheadvdot,-xventanacondops,-za128rs,-za64rs,-zacas,-zama16b,-zawrs,-zba,-zbb,-zbc,-zbkb,-zbkc,-zbkx,-zbs,-zca,-zcb,-zcd,-zce,-zcf,-zcmop,-zcmp,-zcmt,-zdinx,-zfa,-zfh,-zfhmin,-zfinx,-zhinx,-zhinxmin,-zic64b,-zicbom,-zicbop,-zicboz,-ziccamoa,-ziccif,-zicclsm,-ziccrse,-zicntr,-zicond,-zifencei,-zihintntl,-zihintpause,-zihpm,-zimop,-zk,-zkn,-zknd,-zkne,-zknh,-zkr,-zks,-zksed,-zksh,-zkt,-zmmul,-zvbb,-zvbc,-zve32f,-zve32x,-zve64d,-zve64f,-zve64x,-zvfh,-zvfhmin,-zvkb,-zvkg,-zvkn,-zvknc,-zvkned,-zvkng,-zvknha,-zvknhb,-zvks,-zvksc,-zvksed,-zvksg,-zvksh,-zvkt,-zvl1024b,-zvl128b,-zvl16384b,-zvl2048b,-zvl256b,-zvl32768b,-zvl32b,-zvl4096b,-zvl512b,-zvl64b,-zvl65536b,-zvl8192b" }
attributes #1 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic-rv64" "target-features"="+64bit,+a,+c,+d,+f,+m,+relax,+zicsr,-e,-experimental-smmpm,-experimental-smnpm,-experimental-ssnpm,-experimental-sspm,-experimental-ssqosid,-experimental-supm,-experimental-zaamo,-experimental-zabha,-experimental-zalasr,-experimental-zalrsc,-experimental-zfbfmin,-experimental-zicfilp,-experimental-zicfiss,-experimental-ztso,-experimental-zvfbfmin,-experimental-zvfbfwma,-h,-shcounterenw,-shgatpa,-shtvala,-shvsatpa,-shvstvala,-shvstvecd,-smaia,-smepmp,-ssaia,-ssccptr,-sscofpmf,-sscounterenw,-ssstateen,-ssstrict,-sstc,-sstvala,-sstvecd,-ssu64xl,-svade,-svadu,-svbare,-svinval,-svnapot,-svpbmt,-v,-xcvalu,-xcvbi,-xcvbitmanip,-xcvelw,-xcvmac,-xcvmem,-xcvsimd,-xsfcease,-xsfvcp,-xsfvfnrclipxfqf,-xsfvfwmaccqqq,-xsfvqmaccdod,-xsfvqmaccqoq,-xsifivecdiscarddlone,-xsifivecflushdlone,-xtheadba,-xtheadbb,-xtheadbs,-xtheadcmo,-xtheadcondmov,-xtheadfmemidx,-xtheadmac,-xtheadmemidx,-xtheadmempair,-xtheadsync,-xtheadvdot,-xventanacondops,-za128rs,-za64rs,-zacas,-zama16b,-zawrs,-zba,-zbb,-zbc,-zbkb,-zbkc,-zbkx,-zbs,-zca,-zcb,-zcd,-zce,-zcf,-zcmop,-zcmp,-zcmt,-zdinx,-zfa,-zfh,-zfhmin,-zfinx,-zhinx,-zhinxmin,-zic64b,-zicbom,-zicbop,-zicboz,-ziccamoa,-ziccif,-zicclsm,-ziccrse,-zicntr,-zicond,-zifencei,-zihintntl,-zihintpause,-zihpm,-zimop,-zk,-zkn,-zknd,-zkne,-zknh,-zkr,-zks,-zksed,-zksh,-zkt,-zmmul,-zvbb,-zvbc,-zve32f,-zve32x,-zve64d,-zve64f,-zve64x,-zvfh,-zvfhmin,-zvkb,-zvkg,-zvkn,-zvknc,-zvkned,-zvkng,-zvknha,-zvknhb,-zvks,-zvksc,-zvksed,-zvksg,-zvksh,-zvkt,-zvl1024b,-zvl128b,-zvl16384b,-zvl2048b,-zvl256b,-zvl32768b,-zvl32b,-zvl4096b,-zvl512b,-zvl64b,-zvl65536b,-zvl8192b" }
attributes #2 = { nofree nounwind }
attributes #3 = { cold nounwind }
attributes #4 = { cold }

!llvm.module.flags = !{!0, !1, !2, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"lp64d"}
!2 = !{i32 6, !"riscv-isa", !3}
!3 = !{!"rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0"}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"PIE Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 8, !"SmallDataLimit", i32 8}
!8 = !{!"clang version 19.0.0git (https://github.com/llvm/llvm-project.git e7a8dd9b0d419403fe1d8adeb177a4ec78e036cc)"}
!9 = !{!10, !11, i64 56}
!10 = !{!"z_stream_s", !11, i64 0, !14, i64 8, !15, i64 16, !11, i64 24, !14, i64 32, !15, i64 40, !11, i64 48, !11, i64 56, !11, i64 64, !11, i64 72, !11, i64 80, !14, i64 88, !15, i64 96, !15, i64 104}
!11 = !{!"any pointer", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!"int", !12, i64 0}
!15 = !{!"long", !12, i64 0}
!16 = !{!10, !11, i64 0}
!17 = !{!10, !14, i64 8}
!18 = !{!10, !11, i64 24}
!19 = !{!10, !14, i64 32}
!20 = !{!21, !14, i64 60}
!21 = !{!"inflate_state", !11, i64 0, !14, i64 8, !14, i64 12, !14, i64 16, !14, i64 20, !14, i64 24, !14, i64 28, !15, i64 32, !15, i64 40, !11, i64 48, !14, i64 56, !14, i64 60, !14, i64 64, !14, i64 68, !11, i64 72, !15, i64 80, !14, i64 88, !14, i64 92, !14, i64 96, !14, i64 100, !11, i64 104, !11, i64 112, !14, i64 120, !14, i64 124, !14, i64 128, !14, i64 132, !14, i64 136, !14, i64 140, !11, i64 144, !12, i64 152, !12, i64 792, !12, i64 1368, !14, i64 7144, !14, i64 7148, !14, i64 7152}
!22 = !{!21, !14, i64 64}
!23 = !{!21, !14, i64 68}
!24 = !{!21, !11, i64 72}
!25 = !{!21, !15, i64 80}
!26 = !{!21, !14, i64 88}
!27 = !{!21, !11, i64 104}
!28 = !{!21, !11, i64 112}
!29 = !{!21, !14, i64 120}
!30 = !{!21, !14, i64 124}
!31 = !{!12, !12, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"short", !12, i64 0}
!34 = !{!14, !14, i64 0}
!35 = !{!11, !11, i64 0}
!36 = !{!21, !14, i64 7144}
!37 = !{!10, !11, i64 48}
!38 = distinct !{!38, !39}
!39 = !{!"llvm.loop.mustprogress"}
!40 = distinct !{!40, !39}
!41 = distinct !{!41, !39}
!42 = distinct !{!42, !39}
!43 = distinct !{!43, !39}
!44 = distinct !{!44, !39}
!45 = distinct !{!45, !39}
!46 = !{!21, !14, i64 8}

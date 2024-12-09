#! /usr/bin/env perl
# Copyright 2014-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
#ifndef __ARMEB__
	rev		$ctr, $ctr
#endif
___
$code.=<<___	if ($flavour =~ /64/);
	vorr		$dat1,$dat0,$dat0
	add		$tctr1, $ctr, #1
	vorr		$dat2,$dat0,$dat0
	add		$ctr, $ctr, #2
	vorr		$ivec,$dat0,$dat0
	rev		$tctr1, $tctr1
	vmov.32		${dat1}[3],$tctr1
	b.ls		.Lctr32_tail
	rev		$tctr2, $ctr
	sub		$len,$len,#3		// bias
	vmov.32		${dat2}[3],$tctr2
___
$code.=<<___	if ($flavour !~ /64/);
	add		$tctr1, $ctr, #1
	vorr		$ivec,$dat0,$dat0
	rev		$tctr1, $tctr1
	vmov.32		${ivec}[3],$tctr1
	vorr		$dat2,$ivec,$ivec
___
$code.=<<___	if ($flavour =~ /64/);
	cmp		$len,#32
	b.lo		.Loop3x_ctr32

	add		w13,$ctr,#1
	add		w14,$ctr,#2
	aese		$dat1,q8
	aesmc		$tmp1,$dat1
	 vld1.8		{$in0},[$inp],#16
___
$code.=<<___	if ($flavour =~ /64/);
	 vorr		$dat0,$ivec,$ivec
___
$code.=<<___	if ($flavour !~ /64/);
	 add		$tctr0,$ctr,#1
___
$code.=<<___;
	aese		$dat2,q8
	aesmc		$dat2,$dat2
	 vld1.8		{$in1},[$inp],#16
___
$code.=<<___	if ($flavour =~ /64/);
	 vorr		$dat1,$ivec,$ivec
___
$code.=<<___	if ($flavour !~ /64/);
	 rev		$tctr0,$tctr0
___
$code.=<<___;
	aese		$tmp0,q9
	aesmc		$tmp0,$tmp0
	aese		$tmp1,q9
	aesmc		$tmp1,$tmp1
	 mov		$key_,$key
	aese		$dat2,q9
	aesmc		$tmp2,$dat2
___
$code.=<<___	if ($flavour =~ /64/);
	 vorr		$dat2,$ivec,$ivec
	 add		$tctr0,$ctr,#1
___
$code.=<<___;
	aese		$tmp0,q12
	aesmc		$tmp0,$tmp0
	aese		$tmp1,q12
	aesmc		$tmp1,$tmp1
	aese		$tmp1,q13
	aesmc		$tmp1,$tmp1
	 veor		$in2,$in2,$rndlast
___
$code.=<<___	if ($flavour =~ /64/);
	 rev		$tctr0,$tctr0
	aese		$tmp2,q13
	aesmc		$tmp2,$tmp2
	 vmov.32	${dat0}[3], $tctr0
___
$code.=<<___	if ($flavour !~ /64/);
	 vmov.32	${ivec}[3], $tctr0
	aese		$tmp2,q13
	aesmc		$tmp2,$tmp2
	 vorr		$dat0,$ivec,$ivec
___
$code.=<<___;
	 rev		$tctr1,$tctr1
	aese		$tmp0,q14
	aesmc		$tmp0,$tmp0
___
$code.=<<___	if ($flavour !~ /64/);
	 vmov.32	${ivec}[3], $tctr1
	 rev		$tctr2,$ctr
___
$code.=<<___;
	aese		$tmp1,q14
	aesmc		$tmp1,$tmp1
___
$code.=<<___	if ($flavour =~ /64/);
	 vmov.32	${dat1}[3], $tctr1
	 rev		$tctr2,$ctr
	aese		$tmp2,q14
	aesmc		$tmp2,$tmp2
	 vmov.32	${dat2}[3], $tctr2
___
$code.=<<___	if ($flavour !~ /64/);
	 vorr		$dat1,$ivec,$ivec
	 vmov.32	${ivec}[3], $tctr2
	aese		$tmp2,q14
	aesmc		$tmp2,$tmp2
	 vorr		$dat2,$ivec,$ivec
___
$code.=<<___;
	 subs		$len,$len,#3
	aese		$tmp0,q15
	aese		$tmp1,q15
	aese		$tmp2,q15
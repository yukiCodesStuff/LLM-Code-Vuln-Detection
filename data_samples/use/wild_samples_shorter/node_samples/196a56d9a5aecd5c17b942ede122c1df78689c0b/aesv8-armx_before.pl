#! /usr/bin/env perl
# Copyright 2014-2020 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
#ifndef __ARMEB__
	rev		$ctr, $ctr
#endif
	add		$tctr1, $ctr, #1
	vorr		$ivec,$dat0,$dat0
	rev		$tctr1, $tctr1
	vmov.32		${ivec}[3],$tctr1
	vorr		$dat2,$ivec,$ivec
___
$code.=<<___	if ($flavour =~ /64/);
	cmp		$len,#2
	b.lo		.Loop3x_ctr32

	add		w13,$ctr,#1
	add		w14,$ctr,#2
	aese		$dat1,q8
	aesmc		$tmp1,$dat1
	 vld1.8		{$in0},[$inp],#16
	 add		$tctr0,$ctr,#1
	aese		$dat2,q8
	aesmc		$dat2,$dat2
	 vld1.8		{$in1},[$inp],#16
	 rev		$tctr0,$tctr0
	aese		$tmp0,q9
	aesmc		$tmp0,$tmp0
	aese		$tmp1,q9
	aesmc		$tmp1,$tmp1
	 mov		$key_,$key
	aese		$dat2,q9
	aesmc		$tmp2,$dat2
	aese		$tmp0,q12
	aesmc		$tmp0,$tmp0
	aese		$tmp1,q12
	aesmc		$tmp1,$tmp1
	aese		$tmp1,q13
	aesmc		$tmp1,$tmp1
	 veor		$in2,$in2,$rndlast
	 vmov.32	${ivec}[3], $tctr0
	aese		$tmp2,q13
	aesmc		$tmp2,$tmp2
	 vorr		$dat0,$ivec,$ivec
	 rev		$tctr1,$tctr1
	aese		$tmp0,q14
	aesmc		$tmp0,$tmp0
	 vmov.32	${ivec}[3], $tctr1
	 rev		$tctr2,$ctr
	aese		$tmp1,q14
	aesmc		$tmp1,$tmp1
	 vorr		$dat1,$ivec,$ivec
	 vmov.32	${ivec}[3], $tctr2
	aese		$tmp2,q14
	aesmc		$tmp2,$tmp2
	 vorr		$dat2,$ivec,$ivec
	 subs		$len,$len,#3
	aese		$tmp0,q15
	aese		$tmp1,q15
	aese		$tmp2,q15
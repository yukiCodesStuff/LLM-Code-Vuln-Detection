# Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
# Copyright (c) 2020, Intel Corporation. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html
#
#
# Originally written by Ilya Albrekht, Sergey Kirillov and Andrey Matyukov
# Intel Corporation
#
# December 2020
#
# Initial release.
#
# Implementation utilizes 256-bit (ymm) registers to avoid frequency scaling issues.
#
# IceLake-Client @ 1.3GHz
# |---------+----------------------+--------------+-------------|
# |         | OpenSSL 3.0.0-alpha9 | this         | Unit        |
# |---------+----------------------+--------------+-------------|
# | rsa2048 | 2 127 659            | 1 015 625    | cycles/sign |
# |         | 611                  | 1280 / +109% | sign/s      |
# |---------+----------------------+--------------+-------------|
#

# $output is the last argument if it looks like a file (it has an extension)
# $flavour is the first argument if it doesn't look like a file
$output = $#ARGV >= 0 && $ARGV[$#ARGV] =~ m|\.\w+$| ? pop : undef;
$flavour = $#ARGV >= 0 && $ARGV[0] !~ m|\.| ? shift : undef;

$win64=0; $win64=1 if ($flavour =~ /[nm]asm|mingw64/ || $output =~ /\.asm$/);
$avx512ifma=0;

$0 =~ m/(.*[\/\\])[^\/\\]+$/; $dir=$1;
( $xlate="${dir}x86_64-xlate.pl" and -f $xlate ) or
( $xlate="${dir}../../perlasm/x86_64-xlate.pl" and -f $xlate) or
die "can't locate x86_64-xlate.pl";

if (`$ENV{CC} -Wa,-v -c -o /dev/null -x assembler /dev/null 2>&1`
        =~ /GNU assembler version ([2-9]\.[0-9]+)/) {
    $avx512ifma = ($1>=2.26);
}
       `nasm -v 2>&1` =~ /NASM version ([2-9]\.[0-9]+)(?:\.([0-9]+))?/) {
    $avx512ifma = ($1==2.11 && $2>=8) + ($1>=2.12);
}

if (!$avx512 && `$ENV{CC} -v 2>&1` =~ /((?:clang|LLVM) version|.*based on LLVM) ([0-9]+\.[0-9]+)/) {
    $avx512ifma = ($2>=7.0);
}

open OUT,"| \"$^X\" \"$xlate\" $flavour \"$output\""
    or die "can't call $xlate: $!";
*STDOUT=*OUT;

if ($avx512ifma>0) {{{
@_6_args_universal_ABI = ("%rdi","%rsi","%rdx","%rcx","%r8","%r9");

$code.=<<___;
.extern OPENSSL_ia32cap_P
.globl  ossl_rsaz_avx512ifma_eligible
.type   ossl_rsaz_avx512ifma_eligible,\@abi-omnipotent
.align  32
ossl_rsaz_avx512ifma_eligible:
    mov OPENSSL_ia32cap_P+8(%rip), %ecx
    xor %eax,%eax
    and \$`1<<31|1<<21|1<<17|1<<16`, %ecx     # avx512vl + avx512ifma + avx512dq + avx512f
    cmp \$`1<<31|1<<21|1<<17|1<<16`, %ecx
    cmove %ecx,%eax
    ret
.size   ossl_rsaz_avx512ifma_eligible, .-ossl_rsaz_avx512ifma_eligible
___

###############################################################################
# Almost Montgomery Multiplication (AMM) for 20-digit number in radix 2^52.
#
# AMM is defined as presented in the paper
# "Efficient Software Implementations of Modular Exponentiation" by Shay Gueron.
#
# The input and output are presented in 2^52 radix domain, i.e.
#   |res|, |a|, |b|, |m| are arrays of 20 64-bit qwords with 12 high bits zeroed.
#   |k0| is a Montgomery coefficient, which is here k0 = -1/m mod 2^64
#        (note, the implementation counts only 52 bits from it).
#
# NB: the AMM implementation does not perform "conditional" subtraction step as
# specified in the original algorithm as according to the paper "Enhanced Montgomery
# Multiplication" by Shay Gueron (see Lemma 1), the result will be always < 2*2^1024
# and can be used as a direct input to the next AMM iteration.
# This post-condition is true, provided the correct parameter |s| is choosen, i.e.
# s >= n + 2 * k, which matches our case: 1040 > 1024 + 2 * 1.
#
# void ossl_rsaz_amm52x20_x1_256(BN_ULONG *res,
#                           const BN_ULONG *a,
#                           const BN_ULONG *b,
#                           const BN_ULONG *m,
#                           BN_ULONG k0);
###############################################################################
{
# input parameters ("%rdi","%rsi","%rdx","%rcx","%r8")
my ($res,$a,$b,$m,$k0) = @_6_args_universal_ABI;

my $mask52     = "%rax";
my $acc0_0     = "%r9";
my $acc0_0_low = "%r9d";
my $acc0_1     = "%r15";
my $acc0_1_low = "%r15d";
my $b_ptr      = "%r11";

my $iter = "%ebx";

my $zero = "%ymm0";
my ($R0_0,$R0_0h,$R1_0,$R1_0h,$R2_0) = ("%ymm1", map("%ymm$_",(16..19)));
my ($R0_1,$R0_1h,$R1_1,$R1_1h,$R2_1) = ("%ymm2", map("%ymm$_",(20..23)));
my $Bi = "%ymm3";
my $Yi = "%ymm4";

# Registers mapping for normalization.
# We can reuse Bi, Yi registers here.
my $TMP = $Bi;
my $mask52x4 = $Yi;
my ($T0,$T0h,$T1,$T1h,$T2) = map("%ymm$_", (24..28));

sub amm52x20_x1() {
# _data_offset - offset in the |a| or |m| arrays pointing to the beginning
#                of data for corresponding AMM operation;
# _b_offset    - offset in the |b| array pointing to the next qword digit;
my ($_data_offset,$_b_offset,$_acc,$_R0,$_R0h,$_R1,$_R1h,$_R2,$_k0) = @_;
my $_R0_xmm = $_R0;
$_R0_xmm =~ s/%y/%x/;
$code.=<<___;
    movq    $_b_offset($b_ptr), %r13             # b[i]

    vpbroadcastq    %r13, $Bi                    # broadcast b[i]
    movq    $_data_offset($a), %rdx
    mulx    %r13, %r13, %r12                     # a[0]*b[i] = (t0,t2)
    addq    %r13, $_acc                          # acc += t0
    movq    %r12, %r10
    adcq    \$0, %r10                            # t2 += CF

    movq    $_k0, %r13
    imulq   $_acc, %r13                          # acc * k0
    andq    $mask52, %r13                        # yi = (acc * k0) & mask52

    vpbroadcastq    %r13, $Yi                    # broadcast y[i]
    movq    $_data_offset($m), %rdx
    mulx    %r13, %r13, %r12                     # yi * m[0] = (t0,t1)
    addq    %r13, $_acc                          # acc += t0
    adcq    %r12, %r10                           # t2 += (t1 + CF)

    shrq    \$52, $_acc
    salq    \$12, %r10
    or      %r10, $_acc                          # acc = ((acc >> 52) | (t2 << 12))

    vpmadd52luq `$_data_offset+64*0`($a), $Bi, $_R0
    vpmadd52luq `$_data_offset+64*0+32`($a), $Bi, $_R0h
    vpmadd52luq `$_data_offset+64*1`($a), $Bi, $_R1
    vpmadd52luq `$_data_offset+64*1+32`($a), $Bi, $_R1h
    vpmadd52luq `$_data_offset+64*2`($a), $Bi, $_R2

    vpmadd52luq `$_data_offset+64*0`($m), $Yi, $_R0
    vpmadd52luq `$_data_offset+64*0+32`($m), $Yi, $_R0h
    vpmadd52luq `$_data_offset+64*1`($m), $Yi, $_R1
    vpmadd52luq `$_data_offset+64*1+32`($m), $Yi, $_R1h
    vpmadd52luq `$_data_offset+64*2`($m), $Yi, $_R2

    # Shift accumulators right by 1 qword, zero extending the highest one
    valignq     \$1, $_R0, $_R0h, $_R0
    valignq     \$1, $_R0h, $_R1, $_R0h
    valignq     \$1, $_R1, $_R1h, $_R1
    valignq     \$1, $_R1h, $_R2, $_R1h
    valignq     \$1, $_R2, $zero, $_R2

    vmovq   $_R0_xmm, %r13
    addq    %r13, $_acc    # acc += R0[0]

    vpmadd52huq `$_data_offset+64*0`($a), $Bi, $_R0
    vpmadd52huq `$_data_offset+64*0+32`($a), $Bi, $_R0h
    vpmadd52huq `$_data_offset+64*1`($a), $Bi, $_R1
    vpmadd52huq `$_data_offset+64*1+32`($a), $Bi, $_R1h
    vpmadd52huq `$_data_offset+64*2`($a), $Bi, $_R2

    vpmadd52huq `$_data_offset+64*0`($m), $Yi, $_R0
    vpmadd52huq `$_data_offset+64*0+32`($m), $Yi, $_R0h
    vpmadd52huq `$_data_offset+64*1`($m), $Yi, $_R1
    vpmadd52huq `$_data_offset+64*1+32`($m), $Yi, $_R1h
    vpmadd52huq `$_data_offset+64*2`($m), $Yi, $_R2
___
}

# Normalization routine: handles carry bits in R0..R2 QWs and
# gets R0..R2 back to normalized 2^52 representation.
#
# Uses %r8-14,%e[bcd]x
sub amm52x20_x1_norm {
my ($_acc,$_R0,$_R0h,$_R1,$_R1h,$_R2) = @_;
$code.=<<___;
    # Put accumulator to low qword in R0
    vpbroadcastq    $_acc, $TMP
    vpblendd \$3, $TMP, $_R0, $_R0

    # Extract "carries" (12 high bits) from each QW of R0..R2
    # Save them to LSB of QWs in T0..T2
    vpsrlq    \$52, $_R0,   $T0
    vpsrlq    \$52, $_R0h,  $T0h
    vpsrlq    \$52, $_R1,   $T1
    vpsrlq    \$52, $_R1h,  $T1h
    vpsrlq    \$52, $_R2,   $T2

    # "Shift left" T0..T2 by 1 QW
    valignq \$3, $T1h,  $T2,  $T2
    valignq \$3, $T1,   $T1h, $T1h
    valignq \$3, $T0h,  $T1,  $T1
    valignq \$3, $T0,   $T0h, $T0h
    valignq \$3, $zero, $T0,  $T0

    # Drop "carries" from R0..R2 QWs
    vpandq    $mask52x4, $_R0,  $_R0
    vpandq    $mask52x4, $_R0h, $_R0h
    vpandq    $mask52x4, $_R1,  $_R1
    vpandq    $mask52x4, $_R1h, $_R1h
    vpandq    $mask52x4, $_R2,  $_R2

    # Sum R0..R2 with corresponding adjusted carries
    vpaddq  $T0,  $_R0,  $_R0
    vpaddq  $T0h, $_R0h, $_R0h
    vpaddq  $T1,  $_R1,  $_R1
    vpaddq  $T1h, $_R1h, $_R1h
    vpaddq  $T2,  $_R2,  $_R2

    # Now handle carry bits from this addition
    # Get mask of QWs which 52-bit parts overflow...
    vpcmpuq   \$1, $_R0,  $mask52x4, %k1 # OP=lt
    vpcmpuq   \$1, $_R0h, $mask52x4, %k2
    vpcmpuq   \$1, $_R1,  $mask52x4, %k3
    vpcmpuq   \$1, $_R1h, $mask52x4, %k4
    vpcmpuq   \$1, $_R2,  $mask52x4, %k5
    kmovb   %k1, %r14d                   # k1
    kmovb   %k2, %r13d                   # k1h
    kmovb   %k3, %r12d                   # k2
    kmovb   %k4, %r11d                   # k2h
    kmovb   %k5, %r10d                   # k3

    # ...or saturated
    vpcmpuq   \$0, $_R0,  $mask52x4, %k1 # OP=eq
    vpcmpuq   \$0, $_R0h, $mask52x4, %k2
    vpcmpuq   \$0, $_R1,  $mask52x4, %k3
    vpcmpuq   \$0, $_R1h, $mask52x4, %k4
    vpcmpuq   \$0, $_R2,  $mask52x4, %k5
    kmovb   %k1, %r9d                    # k4
    kmovb   %k2, %r8d                    # k4h
    kmovb   %k3, %ebx                    # k5
    kmovb   %k4, %ecx                    # k5h
    kmovb   %k5, %edx                    # k6

    # Get mask of QWs where carries shall be propagated to.
    # Merge 4-bit masks to 8-bit values to use add with carry.
    shl   \$4, %r13b
    or    %r13b, %r14b
    shl   \$4, %r11b
    or    %r11b, %r12b

    add   %r14b, %r14b
    adc   %r12b, %r12b
    adc   %r10b, %r10b

    shl   \$4, %r8b
    or    %r8b,%r9b
    shl   \$4, %cl
    or    %cl, %bl

    add   %r9b, %r14b
    adc   %bl, %r12b
    adc   %dl, %r10b

    xor   %r9b, %r14b
    xor   %bl, %r12b
    xor   %dl, %r10b

    kmovb   %r14d, %k1
    shr     \$4, %r14b
    kmovb   %r14d, %k2
    kmovb   %r12d, %k3
    shr     \$4, %r12b
    kmovb   %r12d, %k4
    kmovb   %r10d, %k5

    # Add carries according to the obtained mask
    vpsubq  $mask52x4, $_R0,  ${_R0}{%k1}
    vpsubq  $mask52x4, $_R0h, ${_R0h}{%k2}
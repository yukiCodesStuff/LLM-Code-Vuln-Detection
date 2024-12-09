#! /usr/bin/env perl
#
# Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
# Copyright Siemens AG 2019-2020
#
# Licensed under the Apache License 2.0 (the "License").
# You may not use this file except in compliance with the License.
# You can obtain a copy in the file LICENSE in the source distribution
#   except within if ... else constructs where some branch contains more than one
#   statement. Since the exception is hard to recognize when such branches occur
#   after the current position (such that false positives would be reported)
#   the tool by checks for this rule by defaul only for do/while/for bodies.
#   Yet with the --1-stmt option false positives are preferred over negatives.
#   False negatives occur if the braces are more than two non-empty lines apart.
#
# * The presence of multiple consecutive spaces is regarded a coding style nit
#   except when this is before end-of-line comments (unless the --eol-comment is given) and
#   except when done in order to align certain columns over multiple lines, e.g.:
#   # define CDE 22
#   # define F   3333
#   This pattern is recognized - and consequently extra space not reported -
#   for a given line if in the nonempty line before or after (if existing)
#   for each occurrence of "  \S" (where \S means non-space) in the given line
#   there is " \S" in the other line in the respective column position.
#   This may lead to both false negatives (in case of coincidental " \S")
#   and false positives (in case of more complex multi-column alignment).
# status variables
my $self_test;             # whether the current input file is regarded to contain (positive/negative) self-tests
my $line;                  # current line number
my $line_before;           # number of previous not essentially empty line (containing at most whitespace and '\')
my $line_before2;          # number of not essentially empty line before previous not essentially empty line
my $contents;              # contents of current line (without blinding)
#  $_                      # current line, where comments etc. get blinded
my $contents_before;       # contents of $line_before (without blinding), if $line_before > 0
my $contents_before_;      # contents of $line_before after blinding comments etc., if $line_before > 0
my $contents_before2;      # contents of $line_before2  (without blinding), if $line_before2 > 0
my $contents_before_2;     # contents of $line_before2 after blinding comments etc., if $line_before2 > 0
my @nested_conds_indents;  # stack of hanging indents due to conditionals ('?' ... ':')
my $expr_indent;           # resulting hanging indent within (multi-line) expressions including type exprs, else 0
my $hanging_symbol;        # character ('(', '{', '[', not: '?') responsible for $expr_indent, if $expr_indent != 0
my $in_expr;               # in expression after if/while/for/switch/return/enum/LHS of assignment
my $in_paren_expr;         # in parenthesized if/while/for condition and switch expression, if $expr_indent != 0
my $in_typedecl;           # nesting level of typedef/struct/union/enum
my $in_directive;          # number of lines so far within preprocessor directive, e.g., macro definition
    $line = 0;
    $line_before = 0;
    $line_before2 = 0;
    @nested_block_indents = ();
    @nested_hanging_offsets = ();
    @nested_in_typedecl = ();
    @nested_symbols = ();
    @nested_indents = ();
    @nested_conds_indents = ();
    $expr_indent = 0;
    $in_paren_expr = 0;
    $in_expr = 0;
    $hanging_offset = 0;
    @in_do_hanging_offsets = ();
    @in_if_hanging_offsets = ();
    $if_maybe_terminated = 0;
            $contents_before) if !$sloppy_cmt && $count_before != $count;
    }
    # ... but allow normal indentation for the current line, else above check will be done for the line before
    if (($in_comment == 0 || $in_comment < 0) # (no commment,) intra-line comment or end of multi-line comment
        && m/^(\s*)@[\s@]*$/) { # line begins with '@', no code follows (except '\')
        if ($count == $ref_indent) { # indentation is like for (normal) code in this line
            s/^(\s*)@/$1*/; # blind first '@' as '*' to prevent above delayed check for the line before
            return;
        my $in_stmt = $in_expr || @nested_symbols != 0; # not: || $in_typedecl != 0
        if ($c =~ m/[{([?]/) { # $c is '{', '(', '[', or '?'
            if ($c eq "{") { # '{' in any context
                # cancel newly hanging_offset if opening brace '{' is after non-whitespace non-comment:
                $hanging_offset -= INDENT_LEVEL if $hanging_offset > 0 && $head =~ m/[^\s\@]/;
                push @nested_block_indents, $block_indent;
                push @nested_hanging_offsets, $in_expr ? $hanging_offset : 0;

while (<>) { # loop over all lines of all input files
    $self_test = $ARGV =~ m/check-format-test/;
    $line++;
    s/\r$//; # strip any trailing CR '\r' (which are typical on Windows systems)
    $contents = $_;


    # do/prepare checks within multi-line comments
    my $self_test_exception = $self_test ? "@" : "";
    if ($in_comment > 0) { # this still includes the last line of multi-line commment
        my ($head, $any_symbol, $cmt_text) = m/^(\s*)(.?)(.*)$/;
        if ($any_symbol eq "*") {
            report("no space after leading '*' in multi-line comment") if $cmt_text =~ m|^[^/\s$self_test_exception]|;
        } else {
            report("no leading '*' in multi-line comment");
        }
        $in_comment++;
    }

    # detect end of comment, must be within multi-line comment, check if it is preceded by non-whitespace text
    if ((my ($head, $tail) = m|^(.*?)\*/(.*)$|) && $1 ne '/') { # ending comment: '*/'
        report("neither space nor '*' before '*/'") if $head =~ m/[^*\s]$/;
        report("no space after '*/'") if $tail =~ m/^[^\s,;)}\]]/; # no space or ,;)}] after '*/'
        if (!($head =~ m|/\*|)) { # not begin of comment '/*', which is is handled below
            if ($in_comment == 0) {
                report("unexpected '*/' outside comment");
                $_ = "$head@@".$tail; # blind the "*/"
            } else {
                report("text before '*/' in multi-line comment") if ($head =~ m/\S/); # non-SPC before '*/'
                $in_comment = -1; # indicate that multi-line comment ends on current line
                if ($count > 0) {
                    # make indentation of end of multi-line comment appear like of leading intra-line comment
                    $head =~ s/^(\s*)\s/$1@/; # replace the last leading space by '@'
    # detect begin of comment, check if it is followed by non-space text
  MATCH_COMMENT:
    if (my ($head, $opt_minus, $tail) = m|^(.*?)/\*(-?)(.*)$|) { # begin of comment: '/*'
        report("no space before '/*'")
            if $head =~ m/[^\s(\*]$/; # not space, '(', or or '*' (needed to allow '*/') before comment delimiter
        report("neither space nor '*' after '/*' or '/*-'") if $tail =~ m/^[^\s*$self_test_exception]/;
        my $cmt_text = $opt_minus.$tail; # preliminary
        if ($in_comment > 0) {
            report("unexpected '/*' inside multi-line comment");
        } elsif ($tail =~ m|^(.*?)\*/(.*)$|) { # comment end: */ on same line
        } else { # begin of multi-line comment
            my $self_test_exception = $self_test ? "(@\d?)?" : "";
            report("text after '/*' in multi-line comment")
                unless $tail =~ m/^$self_test_exception.?\s*$/;
            # tail not essentially empty, first char already checked
            # adapt to actual indentation of first line
            $comment_indent = length($head) + 1;
            $_ = "$head@@".blind_nonspace($cmt_text);
            $in_comment = 1;
            $leading_comment = $head =~ m/^\s*$/; # there is code before beginning delimiter
            $formatted_comment = $opt_minus eq "-";
        }
    }

    if ($in_comment > 1) { # still inside multi-line comment (not at its begin or end)
        m/^(\s*)\*?(\s*)(.*)$/;

    # at this point all non-space portions of any types of comments have been blinded as @

    goto LINE_FINISHED if m/^\s*$/; # essentially empty line: just whitespace (and maybe a trailing '\')

    # intra-line whitespace nits @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    my $in_multiline_comment = ($in_comment > 1 || $in_comment < 0); # $in_multiline_comment refers to line before
        $intra_line =~ s/\s+$//;                     # strip any (resulting) space at EOL
        $intra_line =~ s/(for\s*\([^;]*);;(\))/"$1$2"/eg; # strip trailing ';;' in for (;;)
        $intra_line =~ s/(for\s*\([^;]+;[^;]+);(\))/"$1$2"/eg; # strip trailing ';' in for (;;)
        $intra_line =~ s/(=\s*)\{ /"$1@ "/eg;        # do not report {SPC in initializers such as ' = { 0, };'
        $intra_line =~ s/, \};/, @;/g;               # do not report SPC} in initializers such as ' = { 0, };'
        report("space before '$1'") if $intra_line =~ m/[\w)\]]\s+(\+\+|--)/;  # postfix ++/-- with preceding space
        report("space after '$1'")  if $intra_line =~ m/(\+\+|--)\s+[a-zA-Z_(]/; # prefix ++/-- with following space
        report("space before '$1'") if $intra_line =~ m/\s(\.|->)/;            # '.' or '->' with preceding space
        report("space after '$1'")  if $intra_line =~ m/(\.|->)\s/;            # '.' or '->' with following space
        $intra_line =~ s/\-\>|\+\+|\-\-/@/g;         # blind '->,', '++', and '--'
        report("space before '$2'")     if $intra_line =~ m/[^:]\s+(;)/;       # space before ';' but not after ':'
        report("space before '$1'")     if $intra_line =~ m/\s([,)\]])/;       # space before ,)]
        report("space after '$1'")      if $intra_line =~ m/([(\[~!])\s/;      # space after ([~!
        report("space after '$1'")      if $intra_line =~ m/(defined)\s/;      # space after 'defined'
        report("no space before '=' or '<op>='") if $intra_line =~ m/\S(=)/;   # '=' etc. without preceding space
        report("no space before '$1'")  if $intra_line =~ m/\S([|\/%<>^\?])/;  # |/%<>^? without preceding space
        # TODO ternary ':' without preceding SPC, while allowing no SPC before ':' after 'case'
        report("no space before binary '$1'")  if $intra_line =~ m/[^\s{()\[]([+\-])/;# +/- without preceding space or {()[
                                                                             # or ')' (which is used f type casts)
        report("no space before binary '$1'")  if $intra_line =~ m/[^\s{()\[*!]([*])/; # '*' without preceding space or {()[*!
        report("no space before binary '$1'")  if $intra_line =~ m/[^\s{()\[]([&])/;  # '&' without preceding space or {()[
        report("no space after ternary '$1'") if $intra_line =~ m/(:)[^\s\d]/; # ':' without following space or digit
        report("no space after '$1'")   if $intra_line =~ m/([,;=|\/%<>^\?])\S/; # ,;=|/%<>^? without following space
        report("no space after binary '$1'") if $intra_line=~m/[^{(\[]([*])[^\sa-zA-Z_(),*]/;# '*' w/o space or \w(),* after
        # TODO unary '*' must not be followed by SPC
        report("no space after binary '$1'") if $intra_line=~m/([&])[^\sa-zA-Z_(]/;  # '&' w/o following space or \w(
        # TODO unary '&' must not be followed by SPC
        report("no space after binary '$1'") if $intra_line=~m/[^{(\[]([+\-])[^\s\d(]/;  # +/- w/o following space or \d(
        # TODO unary '+' and '-' must not be followed by SPC
        report("no space after '$2'")   if $intra_line =~ m/(^|\W)(if|while|for|switch|case)[^\w\s]/; # kw w/o SPC
        report("no space after '$2'")   if $intra_line =~ m/(^|\W)(return)[^\w\s;]/;  # return w/o SPC or ';'
        report("space after function/macro name")
                                      if $intra_line =~ m/(\w+)\s+\(/        # fn/macro name with space before '('
       && !($1 =~ m/^(if|while|for|switch|return|typedef|void|char|unsigned|int|long|float|double)$/) # not keyword
                                    && !(m/^\s*#\s*define\s/); # we skip macro definitions here because macros
                                    # without parameters but with body beginning with '(', e.g., '#define X (1)',
                                    # would lead to false positives - TODO also check for macros with parameters
        report("no space before '{'")   if $intra_line =~ m/[^\s{(\[]\{/;      # '{' without preceding space or {([
        report("no space after '}'")    if $intra_line =~ m/\}[^\s,;\])}]/;    # '}' without following space or ,;])}
    }

    # preprocessor directives @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    # update indents according to leading closing brace(s) '}' or label or switch case
    my $in_stmt = $in_expr || @nested_symbols != 0 || $in_typedecl != 0;
    if ($in_stmt) { # expr/stmt/type decl/var def/fn hdr, i.e., not at block level
        if (m/^([\s@]*\})/) { # leading '}', any preceding blinded comment must not be matched
            my $head = $1;
            update_nested_indents($head);
            $nested_indents_position = length($head);
            if (@nested_symbols >= 1) {
            }
            if ($before ne "") { # non-whitespace non-'{' before '}'
                report("code before '}'");
            } else { # leading '}', any preceding blinded comment must not be matched
                $local_offset = $block_indent + $hanging_offset - INDENT_LEVEL;
                update_nested_indents($head);
                $nested_indents_position = length($head);
                $local_offset -= ($block_indent + $hanging_offset);

    check_indent() if $count >= 0; # not for #define and not if multi-line string literal is continued

    $in_comment = 0 if $in_comment < 0; # multi-line comment has ended

    # do some further checks @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

             $line_opening_brace == $line_before)
            && $contents_before =~ m/;/) { # there is at least one terminator ';', so there is some stmt
            # TODO do not report cases where a further else branch
            # follows with a block containg more than one line/statement
            report_flexibly($line_before, "'$keyword_opening_brace' { 1 stmt }", $contents_before);
        }
    }

    report("single-letter name '$2'") if (m/(^|.*\W)([IO])(\W.*|$)/); # single-letter name 'I' or 'O' # maybe re-add 'l'?
    # constant on LHS of comparison or assignment, e.g., NULL != x or 'a' < c, but not a + 1 == b
    report("constant on LHS of '$2'")
        if (m/(['"]|([\+\-\*\/\/%\&\|\^<>]\s*)?\W[0-9]+L?|NULL)\s*([\!<>=]=|[<=>][^<>])/ && $2 eq "");

    # TODO report #if 0 and #if 1

    # TODO report empty line within local variable definitions

    # TODO report missing empty line after local variable definitions

    # TODO report needless use of parentheses, while
    #      macro parameters should always be in parens (except when passed on), e.g., '#define ID(x) (x)'

    # adapt required indentation for following lines @@@@@@@@@@@@@@@@@@@@@@@@@@@

    # set $in_typedecl and potentially $hanging_offset for type declaration
    if (!$in_expr && @nested_indents == 0 # not in expression
        && m/(^|^.*\W)(typedef|struct|union|enum)(\W.*|$)$/
        && parens_balance($1) == 0 # not in newly started expression or function arg list
        && ($2 eq "typedef" || !($3 =~ m/\s*\w++\s*(.)/ && $1 ne "{")) # 'struct'/'union'/'enum' <name> not followed by '{'
        # not needed: && $keyword_opening_brace = $2 if $3 =~ m/\{/;
        ) {
                    !($keyword_opening_brace eq "else" && $line_opening_brace < $line_before2);
            }
            report("code after '{'") if $tail=~ m/[^\s\@]/ && # trailing non-whitespace non-comment (non-'\')
                                      !($tail=~ m/\}/);  # no '}' after last '{'
        }
    }

    # check for opening brace after if/while/for/switch/do not on same line
    # note that "no '{' on same line after '} else'" is handled further below
    if (/^[\s@]*{/ && # leading '{'
        $line_before > 0 && !($contents_before_ =~ m/^\s*#/) && # not preprocessor directive '#if
        (my ($head, $mid, $tail) = ($contents_before_ =~ m/(^|^.*\W)(if|while|for|switch|do)(\W.*$|$)/))) {
        my $brace_after  = $tail =~ /^[\s@]*{/; # any whitespace or comments then '{'
    # check for closing brace on line before 'else' not followed by leading '{'
    elsif (my ($head, $tail) = m/(^|^.*\W)else(\W.*$|$)/) {
        if (parens_balance($tail) == 0 &&  # avoid false positive due to unfinished expr on current line
            !($tail =~ m/{/) && # after 'else' no '{' on same line
            !($head =~ m/}[\s@]*$/) && # not: '}' then any whitespace or comments before 'else'
            $line_before > 0 && $contents_before_ =~ /}[\s@]*$/) { # trailing '}' on line before
            report("no '{' after '} else'");
        }
    }

    # check for closing brace before 'while' not on same line
            if ($line_before > 0 && $contents_before_ =~ /}[\s@]*$/) {
                report("'else' not on same line as preceding '}'");
            } elsif (parens_balance($tail) == 0) { # avoid false positive due to unfinished expr on current line
                report("no '}' on same line before 'else ... {'") if $brace_after;
            }
        } elsif (parens_balance($tail) == 0) { # avoid false positive due to unfinished expr on current line
            report("no '{' on same line after '} else'") if $brace_before && !$brace_after;
        }
    }

  POSTPROCESS_DIRECTIVE:
    # post-processing at end of line @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  LINE_FINISHED:
    # on end of multi-line preprocessor directive, adapt indent
    if ($in_directive > 0 &&
        # need to use original line contents because trailing \ may have been stripped
        !($contents =~ m/^(.*?)[\s@]*\\[\s@]*$/)) { # no trailing '\'
        $hanging_offset = 0; # compensate for this in case macro ends, e.g., as 'while (0)'
    }

    if (m/^\s*$/) { # at begin of file essentially empty line: just whitespace (and maybe a '\')
            report("leading ".($1 eq "" ? "empty" :"whitespace")." line") if $line == 1 && !$sloppy_SPC;
    } else {
        if ($line_before > 0) {
            my $linediff = $line - $line_before - 1;
            report("$linediff empty lines before") if $linediff > 1 && !$sloppy_SPC;
        }
        $line_before2      = $line_before;
        $contents_before2  = $contents_before;
        $contents_before_2 = $contents_before_;
    # post-processing at end of file @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    if (eof) {
        # check for essentially empty line (which may include a '\') just before EOF
        report(($1 eq "\n" ? "empty line" : $2 ne "" ? "'\\'" : "whitespace")." at EOF")
            if $contents =~ m/^(\s*(\\?)\s*)$/ && !$sloppy_SPC;

        # report unclosed expression-level nesting
        check_nested_nonblock_indents("expr at EOF"); # also adapts @nested_block_indents
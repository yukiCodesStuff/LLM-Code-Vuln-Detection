
import re

def IS_LINE_JUNK(line, pat=re.compile(r"\s*(?:#\s*)?$").match):
    r"""
    Return 1 for ignorable line: iff `line` is blank or contains a single '#'.

    Examples:
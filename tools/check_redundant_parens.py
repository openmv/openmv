#!/usr/bin/env python3
# Report parentheses that are redundant given C operator precedence.
#
# For every parenthesized expression, the inner expression's binding strength
# is compared against what its surrounding context requires; parentheses are
# reported only when their removal cannot change the parse. The check is
# deliberate about staying quiet where removing parentheses could change
# meaning: it does not touch unary, cast or sizeof operands, it keeps the
# parentheses on the right of a same-precedence operator (a - (b - c), and
# float re-association like a + (b + c)), and it ignores parentheses produced
# by macro expansion.
#
# Requires libclang (pip install libclang).
#
# Usage:
#   check_redundant_parens.py [--std gnu11] [-I dir]... [--arg X]... file.c...
#   check_redundant_parens.py --diff-base origin/master file.c...   # only new
#
# With --diff-base, only parentheses on lines added relative to that git ref
# are reported, so it gates new code without demanding a repo-wide cleanup.
# Exits non-zero if anything is reported.

import sys
import argparse
import subprocess

import clang.cindex as ci

# C precedence, higher binds tighter.
COMMA = 1
ASSIGN = 2
TERNARY = 3
LOGICAL_OR = 4
PREC = {
    "||": 4, "&&": 5, "|": 6, "^": 7, "&": 8,
    "==": 9, "!=": 9,
    "<": 10, "<=": 10, ">": 10, ">=": 10,
    "<<": 11, ">>": 11,
    "+": 12, "-": 12,
    "*": 13, "/": 13, "%": 13,
    ",": COMMA,
}
ASSIGN_OPS = {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="}
PRIMARY = 16

K = ci.CursorKind


def unwrap(cur):
    # See through the implicit nodes libclang inserts (casts, lvalue->rvalue).
    while cur is not None and cur.kind == K.UNEXPOSED_EXPR:
        kids = list(cur.get_children())
        if len(kids) != 1:
            break
        cur = kids[0]
    return cur


def binary_op(cur):
    # The operator token sits between the two operand subtrees.
    kids = list(cur.get_children())
    if len(kids) != 2:
        return None
    left_end = kids[0].extent.end.offset
    right_start = kids[1].extent.start.offset
    for tok in cur.get_tokens():
        s = tok.extent.start.offset
        if left_end <= s < right_start and (tok.spelling in PREC or tok.spelling in ASSIGN_OPS):
            return tok.spelling
    return None


def top_prec(cur):
    cur = unwrap(cur)
    if cur is None:
        return PRIMARY
    if cur.kind == K.BINARY_OPERATOR:
        return PREC.get(binary_op(cur), ASSIGN)
    if cur.kind == K.COMPOUND_ASSIGNMENT_OPERATOR:
        return ASSIGN
    if cur.kind == K.CONDITIONAL_OPERATOR:
        return TERNARY
    if cur.kind == K.UNARY_OPERATOR:
        return 14
    return PRIMARY


def is_paren(cur):
    return unwrap(cur).kind == K.PAREN_EXPR


def paren_of(cur):
    return unwrap(cur)


class Checker:
    def __init__(self, path, lines):
        self.path = path
        self.lines = lines
        self.hits = []

    def report(self, paren):
        loc = paren.extent.start
        if loc.file is None or loc.file.name != self.path:
            return   # parenthesis came from a macro body in another file
        # Only report where a real '(' sits at the column. A macro expanding to
        # a parenthesized value (#define ID (0x54)) reports at the macro name,
        # not a literal paren; this drops those.
        if 1 <= loc.line <= len(self.lines):
            text = self.lines[loc.line - 1]
            if loc.column - 1 < len(text) and text[loc.column - 1] != "(":
                return
        self.hits.append((loc.line, loc.column))

    def flag(self, child, required_min, strict=False):
        # Redundant when the inner expression binds at least as tightly as the
        # context needs (strictly tighter on the right of a binary operator).
        if not is_paren(child):
            return
        paren = paren_of(child)
        kids = list(paren.get_children())
        p = top_prec(kids[0]) if kids else PRIMARY
        if (p > required_min) if strict else (p >= required_min):
            self.report(paren)

    def visit(self, cur):
        k = cur.kind
        kids = list(cur.get_children())
        if k == K.RETURN_STMT and kids:
            self.flag(kids[0], COMMA)                       # return (x)
        elif k in (K.IF_STMT, K.WHILE_STMT, K.SWITCH_STMT, K.DO_STMT) and kids:
            self.flag(kids[0], COMMA)                       # if ((x))
        elif k == K.VAR_DECL and kids:
            self.flag(kids[-1], ASSIGN)                     # int x = (e)
        elif k == K.BINARY_OPERATOR and len(kids) == 2:
            op = binary_op(cur)
            if op in ASSIGN_OPS:
                self.flag(kids[1], ASSIGN)                  # a = (e)
            elif op in PREC:
                self.flag(kids[0], PREC[op])                # left:  inner >= op
                self.flag(kids[1], PREC[op], strict=True)   # right: inner >  op
        elif k == K.COMPOUND_ASSIGNMENT_OPERATOR and len(kids) == 2:
            self.flag(kids[1], ASSIGN)
        elif k == K.CONDITIONAL_OPERATOR and len(kids) == 3:
            self.flag(kids[0], LOGICAL_OR)                  # cond
            self.flag(kids[1], COMMA)                       # then
            self.flag(kids[2], TERNARY)                     # else
        for ch in kids:
            self.visit(ch)


def added_lines(base, path):
    try:
        out = subprocess.check_output(
            ["git", "diff", "--unified=0", base, "--", path],
            text=True, stderr=subprocess.DEVNULL)
    except (subprocess.CalledProcessError, OSError):
        return None
    lines = set()
    for ln in out.splitlines():
        if ln.startswith("@@"):
            # @@ -a,b +c,d @@
            plus = ln.split("+", 1)[1].split(" ", 1)[0]
            start = int(plus.split(",")[0])
            count = int(plus.split(",")[1]) if "," in plus else 1
            lines.update(range(start, start + count))
    return lines


def check(path, cargs, base):
    idx = ci.Index.create()
    try:
        tu = idx.parse(path, args=cargs)
    except ci.TranslationUnitLoadError:
        print("%s: parse failed" % path, file=sys.stderr)
        return []
    try:
        with open(path, "r", errors="replace") as fh:
            lines = fh.read().splitlines()
    except OSError:
        lines = []
    c = Checker(path, lines)
    c.visit(tu.cursor)
    hits = sorted(set(c.hits))
    if base is not None:
        keep = added_lines(base, path)
        if keep is not None:
            hits = [(l, col) for (l, col) in hits if l in keep]
    return hits


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--std", default="gnu11")
    ap.add_argument("-I", action="append", dest="incs", default=[])
    ap.add_argument("--arg", action="append", dest="cargs", default=[],
                    help="extra argument passed to clang (repeatable)")
    ap.add_argument("--diff-base", default=None,
                    help="only report parentheses on lines added vs this git ref")
    a = ap.parse_args()
    cargs = ["-std=" + a.std] + ["-I" + i for i in a.incs] + a.cargs
    total = 0
    for f in a.files:
        for line, col in check(f, cargs, a.diff_base):
            print("%s:%d:%d: redundant parentheses" % (f, line, col))
            total += 1
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())

    def test_literal_eval(self):
        self.assertEqual(ast.literal_eval('[1, 2, 3]'), [1, 2, 3])
        self.assertEqual(ast.literal_eval('{"foo": 42}'), {"foo": 42})
        self.assertEqual(ast.literal_eval('(True, False, None)'), (True, False, None))
        self.assertEqual(ast.literal_eval('{1, 2, 3}'), {1, 2, 3})
        self.assertEqual(ast.literal_eval('b"hi"'), b"hi")
        self.assertEqual(ast.literal_eval('set()'), set())
        self.assertRaises(ValueError, ast.literal_eval, 'foo()')
        self.assertEqual(ast.literal_eval('6'), 6)
        self.assertEqual(ast.literal_eval('+6'), 6)
        self.assertEqual(ast.literal_eval('-6'), -6)
        self.assertEqual(ast.literal_eval('3.25'), 3.25)
        self.assertEqual(ast.literal_eval('+3.25'), 3.25)
        self.assertEqual(ast.literal_eval('-3.25'), -3.25)
        self.assertEqual(repr(ast.literal_eval('-0.0')), '-0.0')
        self.assertRaises(ValueError, ast.literal_eval, '++6')
        self.assertRaises(ValueError, ast.literal_eval, '+True')
        self.assertRaises(ValueError, ast.literal_eval, '2+3')

    def test_literal_eval_str_int_limit(self):
        with support.adjust_int_max_str_digits(4000):
            ast.literal_eval('3'*4000)  # no error
            with self.assertRaises(SyntaxError) as err_ctx:
                ast.literal_eval('3'*4001)
            self.assertIn('Exceeds the limit ', str(err_ctx.exception))
            self.assertIn(' Consider hexadecimal ', str(err_ctx.exception))

    def test_literal_eval_complex(self):
        # Issue #4907
        self.assertEqual(ast.literal_eval('6j'), 6j)
        self.assertEqual(ast.literal_eval('-6j'), -6j)
        self.assertEqual(ast.literal_eval('6.75j'), 6.75j)
        self.assertEqual(ast.literal_eval('-6.75j'), -6.75j)
        self.assertEqual(ast.literal_eval('3+6j'), 3+6j)
        self.assertEqual(ast.literal_eval('-3+6j'), -3+6j)
        self.assertEqual(ast.literal_eval('3-6j'), 3-6j)
        self.assertEqual(ast.literal_eval('-3-6j'), -3-6j)
        self.assertEqual(ast.literal_eval('3.25+6.75j'), 3.25+6.75j)
        self.assertEqual(ast.literal_eval('-3.25+6.75j'), -3.25+6.75j)
        self.assertEqual(ast.literal_eval('3.25-6.75j'), 3.25-6.75j)
        self.assertEqual(ast.literal_eval('-3.25-6.75j'), -3.25-6.75j)
        self.assertEqual(ast.literal_eval('(3+6j)'), 3+6j)
        self.assertRaises(ValueError, ast.literal_eval, '-6j+3')
        self.assertRaises(ValueError, ast.literal_eval, '-6j+3j')
        self.assertRaises(ValueError, ast.literal_eval, '3+-6j')
        self.assertRaises(ValueError, ast.literal_eval, '3+(0+6j)')
        self.assertRaises(ValueError, ast.literal_eval, '-(3+6j)')

    def test_literal_eval_malformed_dict_nodes(self):
        malformed = ast.Dict(keys=[ast.Constant(1), ast.Constant(2)], values=[ast.Constant(3)])
        self.assertRaises(ValueError, ast.literal_eval, malformed)
        malformed = ast.Dict(keys=[ast.Constant(1)], values=[ast.Constant(2), ast.Constant(3)])
        self.assertRaises(ValueError, ast.literal_eval, malformed)

    def test_literal_eval_trailing_ws(self):
        self.assertEqual(ast.literal_eval("    -1"), -1)
        self.assertEqual(ast.literal_eval("\t\t-1"), -1)
        self.assertEqual(ast.literal_eval(" \t -1"), -1)
        self.assertRaises(IndentationError, ast.literal_eval, "\n -1")

    def test_literal_eval_malformed_lineno(self):
        msg = r'malformed node or string on line 3:'
        with self.assertRaisesRegex(ValueError, msg):
            ast.literal_eval("{'a': 1,\n'b':2,\n'c':++3,\n'd':4}")

        node = ast.UnaryOp(
            ast.UAdd(), ast.UnaryOp(ast.UAdd(), ast.Constant(6)))
        self.assertIsNone(getattr(node, 'lineno', None))
        msg = r'malformed node or string:'
        with self.assertRaisesRegex(ValueError, msg):
            ast.literal_eval(node)

    def test_literal_eval_syntax_errors(self):
        with self.assertRaisesRegex(SyntaxError, "unexpected indent"):
            ast.literal_eval(r'''
                \
                (\
            \ ''')

    def test_bad_integer(self):
        # issue13436: Bad error message with invalid numeric values
        body = [ast.ImportFrom(module='time',
                               names=[ast.alias(name='sleep')],
                               level=None,
                               lineno=None, col_offset=None)]
        mod = ast.Module(body, [])
        with self.assertRaises(ValueError) as cm:
            compile(mod, 'test', 'exec')
        self.assertIn("invalid integer value: None", str(cm.exception))

    def test_level_as_none(self):
        body = [ast.ImportFrom(module='time',
                               names=[ast.alias(name='sleep',
                                                lineno=0, col_offset=0)],
                               level=None,
                               lineno=0, col_offset=0)]
        mod = ast.Module(body, [])
        code = compile(mod, 'test', 'exec')
        ns = {}
        exec(code, ns)
        self.assertIn('sleep', ns)

    def test_recursion_direct(self):
        e = ast.UnaryOp(op=ast.Not(), lineno=0, col_offset=0)
        e.operand = e
        with self.assertRaises(RecursionError):
            with support.infinite_recursion():
                compile(ast.Expression(e), "<test>", "eval")

    def test_recursion_indirect(self):
        e = ast.UnaryOp(op=ast.Not(), lineno=0, col_offset=0)
        f = ast.UnaryOp(op=ast.Not(), lineno=0, col_offset=0)
        e.operand = f
        f.operand = e
        with self.assertRaises(RecursionError):
            with support.infinite_recursion():
                compile(ast.Expression(e), "<test>", "eval")


class ASTValidatorTests(unittest.TestCase):

    def mod(self, mod, msg=None, mode="exec", *, exc=ValueError):
        mod.lineno = mod.col_offset = 0
        ast.fix_missing_locations(mod)
        if msg is None:
            compile(mod, "<test>", mode)
        else:
            with self.assertRaises(exc) as cm:
                compile(mod, "<test>", mode)
            self.assertIn(msg, str(cm.exception))

    def expr(self, node, msg=None, *, exc=ValueError):
        mod = ast.Module([ast.Expr(node)], [])
        self.mod(mod, msg, exc=exc)

    def stmt(self, stmt, msg=None):
        mod = ast.Module([stmt], [])
        self.mod(mod, msg)

    def test_module(self):
        m = ast.Interactive([ast.Expr(ast.Name("x", ast.Store()))])
        self.mod(m, "must have Load context", "single")
        m = ast.Expression(ast.Name("x", ast.Store()))
        self.mod(m, "must have Load context", "eval")

    def _check_arguments(self, fac, check):
        def arguments(args=None, posonlyargs=None, vararg=None,
                      kwonlyargs=None, kwarg=None,
                      defaults=None, kw_defaults=None):
            if args is None:
                args = []
            if posonlyargs is None:
                posonlyargs = []
            if kwonlyargs is None:
                kwonlyargs = []
            if defaults is None:
                defaults = []
            if kw_defaults is None:
                kw_defaults = []
            args = ast.arguments(args, posonlyargs, vararg, kwonlyargs,
                                 kw_defaults, kwarg, defaults)
            return fac(args)
        args = [ast.arg("x", ast.Name("x", ast.Store()))]
        check(arguments(args=args), "must have Load context")
        check(arguments(posonlyargs=args), "must have Load context")
        check(arguments(kwonlyargs=args), "must have Load context")
        check(arguments(defaults=[ast.Num(3)]),
                       "more positional defaults than args")
        check(arguments(kw_defaults=[ast.Num(4)]),
                       "length of kwonlyargs is not the same as kw_defaults")
        args = [ast.arg("x", ast.Name("x", ast.Load()))]
        check(arguments(args=args, defaults=[ast.Name("x", ast.Store())]),
                       "must have Load context")
        args = [ast.arg("a", ast.Name("x", ast.Load())),
                ast.arg("b", ast.Name("y", ast.Load()))]
        check(arguments(kwonlyargs=args,
                          kw_defaults=[None, ast.Name("x", ast.Store())]),
                          "must have Load context")

    def test_funcdef(self):
        a = ast.arguments([], [], None, [], [], None, [])
        f = ast.FunctionDef("x", a, [], [], None)
        self.stmt(f, "empty body on FunctionDef")
        f = ast.FunctionDef("x", a, [ast.Pass()], [ast.Name("x", ast.Store())],
                            None)
        self.stmt(f, "must have Load context")
        f = ast.FunctionDef("x", a, [ast.Pass()], [],
                            ast.Name("x", ast.Store()))
        self.stmt(f, "must have Load context")
        def fac(args):
            return ast.FunctionDef("x", args, [ast.Pass()], [], None)
        self._check_arguments(fac, self.stmt)

    def test_classdef(self):
        def cls(bases=None, keywords=None, body=None, decorator_list=None):
            if bases is None:
                bases = []
            if keywords is None:
                keywords = []
            if body is None:
                body = [ast.Pass()]
            if decorator_list is None:
                decorator_list = []
            return ast.ClassDef("myclass", bases, keywords,
                                body, decorator_list)
        self.stmt(cls(bases=[ast.Name("x", ast.Store())]),
                  "must have Load context")
        self.stmt(cls(keywords=[ast.keyword("x", ast.Name("x", ast.Store()))]),
                  "must have Load context")
        self.stmt(cls(body=[]), "empty body on ClassDef")
        self.stmt(cls(body=[None]), "None disallowed")
        self.stmt(cls(decorator_list=[ast.Name("x", ast.Store())]),
                  "must have Load context")

    def test_delete(self):
        self.stmt(ast.Delete([]), "empty targets on Delete")
        self.stmt(ast.Delete([None]), "None disallowed")
        self.stmt(ast.Delete([ast.Name("x", ast.Load())]),
                  "must have Del context")

    def test_assign(self):
        self.stmt(ast.Assign([], ast.Num(3)), "empty targets on Assign")
        self.stmt(ast.Assign([None], ast.Num(3)), "None disallowed")
        self.stmt(ast.Assign([ast.Name("x", ast.Load())], ast.Num(3)),
                  "must have Store context")
        self.stmt(ast.Assign([ast.Name("x", ast.Store())],
                                ast.Name("y", ast.Store())),
                  "must have Load context")

    def test_augassign(self):
        aug = ast.AugAssign(ast.Name("x", ast.Load()), ast.Add(),
                            ast.Name("y", ast.Load()))
        self.stmt(aug, "must have Store context")
        aug = ast.AugAssign(ast.Name("x", ast.Store()), ast.Add(),
                            ast.Name("y", ast.Store()))
        self.stmt(aug, "must have Load context")

    def test_for(self):
        x = ast.Name("x", ast.Store())
        y = ast.Name("y", ast.Load())
        p = ast.Pass()
        self.stmt(ast.For(x, y, [], []), "empty body on For")
        self.stmt(ast.For(ast.Name("x", ast.Load()), y, [p], []),
                  "must have Store context")
        self.stmt(ast.For(x, ast.Name("y", ast.Store()), [p], []),
                  "must have Load context")
        e = ast.Expr(ast.Name("x", ast.Store()))
        self.stmt(ast.For(x, y, [e], []), "must have Load context")
        self.stmt(ast.For(x, y, [p], [e]), "must have Load context")

    def test_while(self):
        self.stmt(ast.While(ast.Num(3), [], []), "empty body on While")
        self.stmt(ast.While(ast.Name("x", ast.Store()), [ast.Pass()], []),
                  "must have Load context")
        self.stmt(ast.While(ast.Num(3), [ast.Pass()],
                             [ast.Expr(ast.Name("x", ast.Store()))]),
                             "must have Load context")

    def test_if(self):
        self.stmt(ast.If(ast.Num(3), [], []), "empty body on If")
        i = ast.If(ast.Name("x", ast.Store()), [ast.Pass()], [])
        self.stmt(i, "must have Load context")
        i = ast.If(ast.Num(3), [ast.Expr(ast.Name("x", ast.Store()))], [])
        self.stmt(i, "must have Load context")
        i = ast.If(ast.Num(3), [ast.Pass()],
                   [ast.Expr(ast.Name("x", ast.Store()))])
        self.stmt(i, "must have Load context")

    def test_with(self):
        p = ast.Pass()
        self.stmt(ast.With([], [p]), "empty items on With")
        i = ast.withitem(ast.Num(3), None)
        self.stmt(ast.With([i], []), "empty body on With")
        i = ast.withitem(ast.Name("x", ast.Store()), None)
        self.stmt(ast.With([i], [p]), "must have Load context")
        i = ast.withitem(ast.Num(3), ast.Name("x", ast.Load()))
        self.stmt(ast.With([i], [p]), "must have Store context")

    def test_raise(self):
        r = ast.Raise(None, ast.Num(3))
        self.stmt(r, "Raise with cause but no exception")
        r = ast.Raise(ast.Name("x", ast.Store()), None)
        self.stmt(r, "must have Load context")
        r = ast.Raise(ast.Num(4), ast.Name("x", ast.Store()))
        self.stmt(r, "must have Load context")

    def test_try(self):
        p = ast.Pass()
        t = ast.Try([], [], [], [p])
        self.stmt(t, "empty body on Try")
        t = ast.Try([ast.Expr(ast.Name("x", ast.Store()))], [], [], [p])
        self.stmt(t, "must have Load context")
        t = ast.Try([p], [], [], [])
        self.stmt(t, "Try has neither except handlers nor finalbody")
        t = ast.Try([p], [], [p], [p])
        self.stmt(t, "Try has orelse but no except handlers")
        t = ast.Try([p], [ast.ExceptHandler(None, "x", [])], [], [])
        self.stmt(t, "empty body on ExceptHandler")
        e = [ast.ExceptHandler(ast.Name("x", ast.Store()), "y", [p])]
        self.stmt(ast.Try([p], e, [], []), "must have Load context")
        e = [ast.ExceptHandler(None, "x", [p])]
        t = ast.Try([p], e, [ast.Expr(ast.Name("x", ast.Store()))], [p])
        self.stmt(t, "must have Load context")
        t = ast.Try([p], e, [p], [ast.Expr(ast.Name("x", ast.Store()))])
        self.stmt(t, "must have Load context")

    def test_try_star(self):
        p = ast.Pass()
        t = ast.TryStar([], [], [], [p])
        self.stmt(t, "empty body on TryStar")
        t = ast.TryStar([ast.Expr(ast.Name("x", ast.Store()))], [], [], [p])
        self.stmt(t, "must have Load context")
        t = ast.TryStar([p], [], [], [])
        self.stmt(t, "TryStar has neither except handlers nor finalbody")
        t = ast.TryStar([p], [], [p], [p])
        self.stmt(t, "TryStar has orelse but no except handlers")
        t = ast.TryStar([p], [ast.ExceptHandler(None, "x", [])], [], [])
        self.stmt(t, "empty body on ExceptHandler")
        e = [ast.ExceptHandler(ast.Name("x", ast.Store()), "y", [p])]
        self.stmt(ast.TryStar([p], e, [], []), "must have Load context")
        e = [ast.ExceptHandler(None, "x", [p])]
        t = ast.TryStar([p], e, [ast.Expr(ast.Name("x", ast.Store()))], [p])
        self.stmt(t, "must have Load context")
        t = ast.TryStar([p], e, [p], [ast.Expr(ast.Name("x", ast.Store()))])
        self.stmt(t, "must have Load context")

    def test_assert(self):
        self.stmt(ast.Assert(ast.Name("x", ast.Store()), None),
                  "must have Load context")
        assrt = ast.Assert(ast.Name("x", ast.Load()),
                           ast.Name("y", ast.Store()))
        self.stmt(assrt, "must have Load context")

    def test_import(self):
        self.stmt(ast.Import([]), "empty names on Import")

    def test_importfrom(self):
        imp = ast.ImportFrom(None, [ast.alias("x", None)], -42)
        self.stmt(imp, "Negative ImportFrom level")
        self.stmt(ast.ImportFrom(None, [], 0), "empty names on ImportFrom")

    def test_global(self):
        self.stmt(ast.Global([]), "empty names on Global")

    def test_nonlocal(self):
        self.stmt(ast.Nonlocal([]), "empty names on Nonlocal")

    def test_expr(self):
        e = ast.Expr(ast.Name("x", ast.Store()))
        self.stmt(e, "must have Load context")

    def test_boolop(self):
        b = ast.BoolOp(ast.And(), [])
        self.expr(b, "less than 2 values")
        b = ast.BoolOp(ast.And(), [ast.Num(3)])
        self.expr(b, "less than 2 values")
        b = ast.BoolOp(ast.And(), [ast.Num(4), None])
        self.expr(b, "None disallowed")
        b = ast.BoolOp(ast.And(), [ast.Num(4), ast.Name("x", ast.Store())])
        self.expr(b, "must have Load context")

    def test_unaryop(self):
        u = ast.UnaryOp(ast.Not(), ast.Name("x", ast.Store()))
        self.expr(u, "must have Load context")

    def test_lambda(self):
        a = ast.arguments([], [], None, [], [], None, [])
        self.expr(ast.Lambda(a, ast.Name("x", ast.Store())),
                  "must have Load context")
        def fac(args):
            return ast.Lambda(args, ast.Name("x", ast.Load()))
        self._check_arguments(fac, self.expr)

    def test_ifexp(self):
        l = ast.Name("x", ast.Load())
        s = ast.Name("y", ast.Store())
        for args in (s, l, l), (l, s, l), (l, l, s):
            self.expr(ast.IfExp(*args), "must have Load context")

    def test_dict(self):
        d = ast.Dict([], [ast.Name("x", ast.Load())])
        self.expr(d, "same number of keys as values")
        d = ast.Dict([ast.Name("x", ast.Load())], [None])
        self.expr(d, "None disallowed")

    def test_set(self):
        self.expr(ast.Set([None]), "None disallowed")
        s = ast.Set([ast.Name("x", ast.Store())])
        self.expr(s, "must have Load context")

    def _check_comprehension(self, fac):
        self.expr(fac([]), "comprehension with no generators")
        g = ast.comprehension(ast.Name("x", ast.Load()),
                              ast.Name("x", ast.Load()), [], 0)
        self.expr(fac([g]), "must have Store context")
        g = ast.comprehension(ast.Name("x", ast.Store()),
                              ast.Name("x", ast.Store()), [], 0)
        self.expr(fac([g]), "must have Load context")
        x = ast.Name("x", ast.Store())
        y = ast.Name("y", ast.Load())
        g = ast.comprehension(x, y, [None], 0)
        self.expr(fac([g]), "None disallowed")
        g = ast.comprehension(x, y, [ast.Name("x", ast.Store())], 0)
        self.expr(fac([g]), "must have Load context")

    def _simple_comp(self, fac):
        g = ast.comprehension(ast.Name("x", ast.Store()),
                              ast.Name("x", ast.Load()), [], 0)
        self.expr(fac(ast.Name("x", ast.Store()), [g]),
                  "must have Load context")
        def wrap(gens):
            return fac(ast.Name("x", ast.Store()), gens)
        self._check_comprehension(wrap)

    def test_listcomp(self):
        self._simple_comp(ast.ListComp)

    def test_setcomp(self):
        self._simple_comp(ast.SetComp)

    def test_generatorexp(self):
        self._simple_comp(ast.GeneratorExp)

    def test_dictcomp(self):
        g = ast.comprehension(ast.Name("y", ast.Store()),
                              ast.Name("p", ast.Load()), [], 0)
        c = ast.DictComp(ast.Name("x", ast.Store()),
                         ast.Name("y", ast.Load()), [g])
        self.expr(c, "must have Load context")
        c = ast.DictComp(ast.Name("x", ast.Load()),
                         ast.Name("y", ast.Store()), [g])
        self.expr(c, "must have Load context")
        def factory(comps):
            k = ast.Name("x", ast.Load())
            v = ast.Name("y", ast.Load())
            return ast.DictComp(k, v, comps)
        self._check_comprehension(factory)

    def test_yield(self):
        self.expr(ast.Yield(ast.Name("x", ast.Store())), "must have Load")
        self.expr(ast.YieldFrom(ast.Name("x", ast.Store())), "must have Load")

    def test_compare(self):
        left = ast.Name("x", ast.Load())
        comp = ast.Compare(left, [ast.In()], [])
        self.expr(comp, "no comparators")
        comp = ast.Compare(left, [ast.In()], [ast.Num(4), ast.Num(5)])
        self.expr(comp, "different number of comparators and operands")
        comp = ast.Compare(ast.Num("blah"), [ast.In()], [left])
        self.expr(comp)
        comp = ast.Compare(left, [ast.In()], [ast.Num("blah")])
        self.expr(comp)

    def test_call(self):
        func = ast.Name("x", ast.Load())
        args = [ast.Name("y", ast.Load())]
        keywords = [ast.keyword("w", ast.Name("z", ast.Load()))]
        call = ast.Call(ast.Name("x", ast.Store()), args, keywords)
        self.expr(call, "must have Load context")
        call = ast.Call(func, [None], keywords)
        self.expr(call, "None disallowed")
        bad_keywords = [ast.keyword("w", ast.Name("z", ast.Store()))]
        call = ast.Call(func, args, bad_keywords)
        self.expr(call, "must have Load context")

    def test_num(self):
        class subint(int):
            pass
        class subfloat(float):
            pass
        class subcomplex(complex):
            pass
        for obj in "0", "hello":
            self.expr(ast.Num(obj))
        for obj in subint(), subfloat(), subcomplex():
            self.expr(ast.Num(obj), "invalid type", exc=TypeError)

    def test_attribute(self):
        attr = ast.Attribute(ast.Name("x", ast.Store()), "y", ast.Load())
        self.expr(attr, "must have Load context")

    def test_subscript(self):
        sub = ast.Subscript(ast.Name("x", ast.Store()), ast.Num(3),
                            ast.Load())
        self.expr(sub, "must have Load context")
        x = ast.Name("x", ast.Load())
        sub = ast.Subscript(x, ast.Name("y", ast.Store()),
                            ast.Load())
        self.expr(sub, "must have Load context")
        s = ast.Name("x", ast.Store())
        for args in (s, None, None), (None, s, None), (None, None, s):
            sl = ast.Slice(*args)
            self.expr(ast.Subscript(x, sl, ast.Load()),
                      "must have Load context")
        sl = ast.Tuple([], ast.Load())
        self.expr(ast.Subscript(x, sl, ast.Load()))
        sl = ast.Tuple([s], ast.Load())
        self.expr(ast.Subscript(x, sl, ast.Load()), "must have Load context")

    def test_starred(self):
        left = ast.List([ast.Starred(ast.Name("x", ast.Load()), ast.Store())],
                        ast.Store())
        assign = ast.Assign([left], ast.Num(4))
        self.stmt(assign, "must have Store context")

    def _sequence(self, fac):
        self.expr(fac([None], ast.Load()), "None disallowed")
        self.expr(fac([ast.Name("x", ast.Store())], ast.Load()),
                  "must have Load context")

    def test_list(self):
        self._sequence(ast.List)

    def test_tuple(self):
        self._sequence(ast.Tuple)

    def test_nameconstant(self):
        self.expr(ast.NameConstant(4))

    def test_stdlib_validates(self):
        stdlib = os.path.dirname(ast.__file__)
        tests = [fn for fn in os.listdir(stdlib) if fn.endswith(".py")]
        tests.extend(["test/test_grammar.py", "test/test_unpack_ex.py"])
        for module in tests:
            with self.subTest(module):
                fn = os.path.join(stdlib, module)
                with open(fn, "r", encoding="utf-8") as fp:
                    source = fp.read()
                mod = ast.parse(source, fn)
                compile(mod, fn, "exec")

    constant_1 = ast.Constant(1)
    pattern_1 = ast.MatchValue(constant_1)

    constant_x = ast.Constant('x')
    pattern_x = ast.MatchValue(constant_x)

    constant_true = ast.Constant(True)
    pattern_true = ast.MatchSingleton(True)

    name_carter = ast.Name('carter', ast.Load())

    _MATCH_PATTERNS = [
        ast.MatchValue(
            ast.Attribute(
                ast.Attribute(
                    ast.Name('x', ast.Store()),
                    'y', ast.Load()
                ),
                'z', ast.Load()
            )
        ),
        ast.MatchValue(
            ast.Attribute(
                ast.Attribute(
                    ast.Name('x', ast.Load()),
                    'y', ast.Store()
                ),
                'z', ast.Load()
            )
        ),
        ast.MatchValue(
            ast.Constant(...)
        ),
        ast.MatchValue(
            ast.Constant(True)
        ),
        ast.MatchValue(
            ast.Constant((1,2,3))
        ),
        ast.MatchSingleton('string'),
        ast.MatchSequence([
          ast.MatchSingleton('string')
        ]),
        ast.MatchSequence(
            [
                ast.MatchSequence(
                    [
                        ast.MatchSingleton('string')
                    ]
                )
            ]
        ),
        ast.MatchMapping(
            [constant_1, constant_true],
            [pattern_x]
        ),
        ast.MatchMapping(
            [constant_true, constant_1],
            [pattern_x, pattern_1],
            rest='True'
        ),
        ast.MatchMapping(
            [constant_true, ast.Starred(ast.Name('lol', ast.Load()), ast.Load())],
            [pattern_x, pattern_1],
            rest='legit'
        ),
        ast.MatchClass(
            ast.Attribute(
                ast.Attribute(
                    constant_x,
                    'y', ast.Load()),
                'z', ast.Load()),
            patterns=[], kwd_attrs=[], kwd_patterns=[]
        ),
        ast.MatchClass(
            name_carter,
            patterns=[],
            kwd_attrs=['True'],
            kwd_patterns=[pattern_1]
        ),
        ast.MatchClass(
            name_carter,
            patterns=[],
            kwd_attrs=[],
            kwd_patterns=[pattern_1]
        ),
        ast.MatchClass(
            name_carter,
            patterns=[ast.MatchSingleton('string')],
            kwd_attrs=[],
            kwd_patterns=[]
        ),
        ast.MatchClass(
            name_carter,
            patterns=[ast.MatchStar()],
            kwd_attrs=[],
            kwd_patterns=[]
        ),
        ast.MatchClass(
            name_carter,
            patterns=[],
            kwd_attrs=[],
            kwd_patterns=[ast.MatchStar()]
        ),
        ast.MatchSequence(
            [
                ast.MatchStar("True")
            ]
        ),
        ast.MatchAs(
            name='False'
        ),
        ast.MatchOr(
            []
        ),
        ast.MatchOr(
            [pattern_1]
        ),
        ast.MatchOr(
            [pattern_1, pattern_x, ast.MatchSingleton('xxx')]
        ),
        ast.MatchAs(name="_"),
        ast.MatchStar(name="x"),
        ast.MatchSequence([ast.MatchStar("_")]),
        ast.MatchMapping([], [], rest="_"),
    ]

    def test_match_validation_pattern(self):
        name_x = ast.Name('x', ast.Load())
        for pattern in self._MATCH_PATTERNS:
            with self.subTest(ast.dump(pattern, indent=4)):
                node = ast.Match(
                    subject=name_x,
                    cases = [
                        ast.match_case(
                            pattern=pattern,
                            body = [ast.Pass()]
                        )
                    ]
                )
                node = ast.fix_missing_locations(node)
                module = ast.Module([node], [])
                with self.assertRaises(ValueError):
                    compile(module, "<test>", "exec")


class ConstantTests(unittest.TestCase):
    """Tests on the ast.Constant node type."""

    def compile_constant(self, value):
        tree = ast.parse("x = 123")

        node = tree.body[0].value
        new_node = ast.Constant(value=value)
        ast.copy_location(new_node, node)
        tree.body[0].value = new_node

        code = compile(tree, "<string>", "exec")

        ns = {}
        exec(code, ns)
        return ns['x']

    def test_validation(self):
        with self.assertRaises(TypeError) as cm:
            self.compile_constant([1, 2, 3])
        self.assertEqual(str(cm.exception),
                         "got an invalid type in Constant: list")

    def test_singletons(self):
        for const in (None, False, True, Ellipsis, b'', frozenset()):
            with self.subTest(const=const):
                value = self.compile_constant(const)
                self.assertIs(value, const)

    def test_values(self):
        nested_tuple = (1,)
        nested_frozenset = frozenset({1})
        for level in range(3):
            nested_tuple = (nested_tuple, 2)
            nested_frozenset = frozenset({nested_frozenset, 2})
        values = (123, 123.0, 123j,
                  "unicode", b'bytes',
                  tuple("tuple"), frozenset("frozenset"),
                  nested_tuple, nested_frozenset)
        for value in values:
            with self.subTest(value=value):
                result = self.compile_constant(value)
                self.assertEqual(result, value)

    def test_assign_to_constant(self):
        tree = ast.parse("x = 1")

        target = tree.body[0].targets[0]
        new_target = ast.Constant(value=1)
        ast.copy_location(new_target, target)
        tree.body[0].targets[0] = new_target

        with self.assertRaises(ValueError) as cm:
            compile(tree, "string", "exec")
        self.assertEqual(str(cm.exception),
                         "expression which can't be assigned "
                         "to in Store context")

    def test_get_docstring(self):
        tree = ast.parse("'docstring'\nx = 1")
        self.assertEqual(ast.get_docstring(tree), 'docstring')

    def get_load_const(self, tree):
        # Compile to bytecode, disassemble and get parameter of LOAD_CONST
        # instructions
        co = compile(tree, '<string>', 'exec')
        consts = []
        for instr in dis.get_instructions(co):
            if instr.opname == 'LOAD_CONST':
                consts.append(instr.argval)
        return consts

    @support.cpython_only
    def test_load_const(self):
        consts = [None,
                  True, False,
                  124,
                  2.0,
                  3j,
                  "unicode",
                  b'bytes',
                  (1, 2, 3)]

        code = '\n'.join(['x={!r}'.format(const) for const in consts])
        code += '\nx = ...'
        consts.extend((Ellipsis, None))

        tree = ast.parse(code)
        self.assertEqual(self.get_load_const(tree),
                         consts)

        # Replace expression nodes with constants
        for assign, const in zip(tree.body, consts):
            assert isinstance(assign, ast.Assign), ast.dump(assign)
            new_node = ast.Constant(value=const)
            ast.copy_location(new_node, assign.value)
            assign.value = new_node

        self.assertEqual(self.get_load_const(tree),
                         consts)

    def test_literal_eval(self):
        tree = ast.parse("1 + 2")
        binop = tree.body[0].value

        new_left = ast.Constant(value=10)
        ast.copy_location(new_left, binop.left)
        binop.left = new_left

        new_right = ast.Constant(value=20j)
        ast.copy_location(new_right, binop.right)
        binop.right = new_right

        self.assertEqual(ast.literal_eval(binop), 10+20j)

    def test_string_kind(self):
        c = ast.parse('"x"', mode='eval').body
        self.assertEqual(c.value, "x")
        self.assertEqual(c.kind, None)

        c = ast.parse('u"x"', mode='eval').body
        self.assertEqual(c.value, "x")
        self.assertEqual(c.kind, "u")

        c = ast.parse('r"x"', mode='eval').body
        self.assertEqual(c.value, "x")
        self.assertEqual(c.kind, None)

        c = ast.parse('b"x"', mode='eval').body
        self.assertEqual(c.value, b"x")
        self.assertEqual(c.kind, None)


class EndPositionTests(unittest.TestCase):
    """Tests for end position of AST nodes.

    Testing end positions of nodes requires a bit of extra care
    because of how LL parsers work.
    """
    def _check_end_pos(self, ast_node, end_lineno, end_col_offset):
        self.assertEqual(ast_node.end_lineno, end_lineno)
        self.assertEqual(ast_node.end_col_offset, end_col_offset)

    def _check_content(self, source, ast_node, content):
        self.assertEqual(ast.get_source_segment(source, ast_node), content)

    def _parse_value(self, s):
        # Use duck-typing to support both single expression
        # and a right hand side of an assignment statement.
        return ast.parse(s).body[0].value

    def test_lambda(self):
        s = 'lambda x, *y: None'
        lam = self._parse_value(s)
        self._check_content(s, lam.body, 'None')
        self._check_content(s, lam.args.args[0], 'x')
        self._check_content(s, lam.args.vararg, 'y')

    def test_func_def(self):
        s = dedent('''
            def func(x: int,
                     *args: str,
                     z: float = 0,
                     **kwargs: Any) -> bool:
                return True
            ''').strip()
        fdef = ast.parse(s).body[0]
        self._check_end_pos(fdef, 5, 15)
        self._check_content(s, fdef.body[0], 'return True')
        self._check_content(s, fdef.args.args[0], 'x: int')
        self._check_content(s, fdef.args.args[0].annotation, 'int')
        self._check_content(s, fdef.args.kwarg, 'kwargs: Any')
        self._check_content(s, fdef.args.kwarg.annotation, 'Any')

    def test_call(self):
        s = 'func(x, y=2, **kw)'
        call = self._parse_value(s)
        self._check_content(s, call.func, 'func')
        self._check_content(s, call.keywords[0].value, '2')
        self._check_content(s, call.keywords[1].value, 'kw')

    def test_call_noargs(self):
        s = 'x[0]()'
        call = self._parse_value(s)
        self._check_content(s, call.func, 'x[0]')
        self._check_end_pos(call, 1, 6)

    def test_class_def(self):
        s = dedent('''
            class C(A, B):
                x: int = 0
        ''').strip()
        cdef = ast.parse(s).body[0]
        self._check_end_pos(cdef, 2, 14)
        self._check_content(s, cdef.bases[1], 'B')
        self._check_content(s, cdef.body[0], 'x: int = 0')

    def test_class_kw(self):
        s = 'class S(metaclass=abc.ABCMeta): pass'
        cdef = ast.parse(s).body[0]
        self._check_content(s, cdef.keywords[0].value, 'abc.ABCMeta')

    def test_multi_line_str(self):
        s = dedent('''
            x = """Some multi-line text.

            It goes on starting from same indent."""
        ''').strip()
        assign = ast.parse(s).body[0]
        self._check_end_pos(assign, 3, 40)
        self._check_end_pos(assign.value, 3, 40)

    def test_continued_str(self):
        s = dedent('''
            x = "first part" \\
            "second part"
        ''').strip()
        assign = ast.parse(s).body[0]
        self._check_end_pos(assign, 2, 13)
        self._check_end_pos(assign.value, 2, 13)

    def test_suites(self):
        # We intentionally put these into the same string to check
        # that empty lines are not part of the suite.
        s = dedent('''
            while True:
                pass

            if one():
                x = None
            elif other():
                y = None
            else:
                z = None

            for x, y in stuff:
                assert True

            try:
                raise RuntimeError
            except TypeError as e:
                pass

            pass
        ''').strip()
        mod = ast.parse(s)
        while_loop = mod.body[0]
        if_stmt = mod.body[1]
        for_loop = mod.body[2]
        try_stmt = mod.body[3]
        pass_stmt = mod.body[4]

        self._check_end_pos(while_loop, 2, 8)
        self._check_end_pos(if_stmt, 9, 12)
        self._check_end_pos(for_loop, 12, 15)
        self._check_end_pos(try_stmt, 17, 8)
        self._check_end_pos(pass_stmt, 19, 4)

        self._check_content(s, while_loop.test, 'True')
        self._check_content(s, if_stmt.body[0], 'x = None')
        self._check_content(s, if_stmt.orelse[0].test, 'other()')
        self._check_content(s, for_loop.target, 'x, y')
        self._check_content(s, try_stmt.body[0], 'raise RuntimeError')
        self._check_content(s, try_stmt.handlers[0].type, 'TypeError')

    def test_fstring(self):
        s = 'x = f"abc {x + y} abc"'
        fstr = self._parse_value(s)
        binop = fstr.values[1].value
        self._check_content(s, binop, 'x + y')

    def test_fstring_multi_line(self):
        s = dedent('''
            f"""Some multi-line text.
            {
            arg_one
            +
            arg_two
            }
            It goes on..."""
        ''').strip()
        fstr = self._parse_value(s)
        binop = fstr.values[1].value
        self._check_end_pos(binop, 5, 7)
        self._check_content(s, binop.left, 'arg_one')
        self._check_content(s, binop.right, 'arg_two')

    def test_import_from_multi_line(self):
        s = dedent('''
            from x.y.z import (
                a, b, c as c
            )
        ''').strip()
        imp = ast.parse(s).body[0]
        self._check_end_pos(imp, 3, 1)
        self._check_end_pos(imp.names[2], 2, 16)

    def test_slices(self):
        s1 = 'f()[1, 2] [0]'
        s2 = 'x[ a.b: c.d]'
        sm = dedent('''
            x[ a.b: f () ,
               g () : c.d
              ]
        ''').strip()
        i1, i2, im = map(self._parse_value, (s1, s2, sm))
        self._check_content(s1, i1.value, 'f()[1, 2]')
        self._check_content(s1, i1.value.slice, '1, 2')
        self._check_content(s2, i2.slice.lower, 'a.b')
        self._check_content(s2, i2.slice.upper, 'c.d')
        self._check_content(sm, im.slice.elts[0].upper, 'f ()')
        self._check_content(sm, im.slice.elts[1].lower, 'g ()')
        self._check_end_pos(im, 3, 3)

    def test_binop(self):
        s = dedent('''
            (1 * 2 + (3 ) +
                 4
            )
        ''').strip()
        binop = self._parse_value(s)
        self._check_end_pos(binop, 2, 6)
        self._check_content(s, binop.right, '4')
        self._check_content(s, binop.left, '1 * 2 + (3 )')
        self._check_content(s, binop.left.right, '3')

    def test_boolop(self):
        s = dedent('''
            if (one_condition and
                    (other_condition or yet_another_one)):
                pass
        ''').strip()
        bop = ast.parse(s).body[0].test
        self._check_end_pos(bop, 2, 44)
        self._check_content(s, bop.values[1],
                            'other_condition or yet_another_one')

    def test_tuples(self):
        s1 = 'x = () ;'
        s2 = 'x = 1 , ;'
        s3 = 'x = (1 , 2 ) ;'
        sm = dedent('''
            x = (
                a, b,
            )
        ''').strip()
        t1, t2, t3, tm = map(self._parse_value, (s1, s2, s3, sm))
        self._check_content(s1, t1, '()')
        self._check_content(s2, t2, '1 ,')
        self._check_content(s3, t3, '(1 , 2 )')
        self._check_end_pos(tm, 3, 1)

    def test_attribute_spaces(self):
        s = 'func(x. y .z)'
        call = self._parse_value(s)
        self._check_content(s, call, s)
        self._check_content(s, call.args[0], 'x. y .z')

    def test_redundant_parenthesis(self):
        s = '( ( ( a + b ) ) )'
        v = ast.parse(s).body[0].value
        self.assertEqual(type(v).__name__, 'BinOp')
        self._check_content(s, v, 'a + b')
        s2 = 'await ' + s
        v = ast.parse(s2).body[0].value.value
        self.assertEqual(type(v).__name__, 'BinOp')
        self._check_content(s2, v, 'a + b')

    def test_trailers_with_redundant_parenthesis(self):
        tests = (
            ('( ( ( a ) ) ) ( )', 'Call'),
            ('( ( ( a ) ) ) ( b )', 'Call'),
            ('( ( ( a ) ) ) [ b ]', 'Subscript'),
            ('( ( ( a ) ) ) . b', 'Attribute'),
        )
        for s, t in tests:
            with self.subTest(s):
                v = ast.parse(s).body[0].value
                self.assertEqual(type(v).__name__, t)
                self._check_content(s, v, s)
                s2 = 'await ' + s
                v = ast.parse(s2).body[0].value.value
                self.assertEqual(type(v).__name__, t)
                self._check_content(s2, v, s)

    def test_displays(self):
        s1 = '[{}, {1, }, {1, 2,} ]'
        s2 = '{a: b, f (): g () ,}'
        c1 = self._parse_value(s1)
        c2 = self._parse_value(s2)
        self._check_content(s1, c1.elts[0], '{}')
        self._check_content(s1, c1.elts[1], '{1, }')
        self._check_content(s1, c1.elts[2], '{1, 2,}')
        self._check_content(s2, c2.keys[1], 'f ()')
        self._check_content(s2, c2.values[1], 'g ()')

    def test_comprehensions(self):
        s = dedent('''
            x = [{x for x, y in stuff
                  if cond.x} for stuff in things]
        ''').strip()
        cmp = self._parse_value(s)
        self._check_end_pos(cmp, 2, 37)
        self._check_content(s, cmp.generators[0].iter, 'things')
        self._check_content(s, cmp.elt.generators[0].iter, 'stuff')
        self._check_content(s, cmp.elt.generators[0].ifs[0], 'cond.x')
        self._check_content(s, cmp.elt.generators[0].target, 'x, y')

    def test_yield_await(self):
        s = dedent('''
            async def f():
                yield x
                await y
        ''').strip()
        fdef = ast.parse(s).body[0]
        self._check_content(s, fdef.body[0].value, 'yield x')
        self._check_content(s, fdef.body[1].value, 'await y')

    def test_source_segment_multi(self):
        s_orig = dedent('''
            x = (
                a, b,
            ) + ()
        ''').strip()
        s_tuple = dedent('''
            (
                a, b,
            )
        ''').strip()
        binop = self._parse_value(s_orig)
        self.assertEqual(ast.get_source_segment(s_orig, binop.left), s_tuple)

    def test_source_segment_padded(self):
        s_orig = dedent('''
            class C:
                def fun(self) -> None:
                    "ЖЖЖЖЖ"
        ''').strip()
        s_method = '    def fun(self) -> None:\n' \
                   '        "ЖЖЖЖЖ"'
        cdef = ast.parse(s_orig).body[0]
        self.assertEqual(ast.get_source_segment(s_orig, cdef.body[0], padded=True),
                         s_method)

    def test_source_segment_endings(self):
        s = 'v = 1\r\nw = 1\nx = 1\n\ry = 1\rz = 1\r\n'
        v, w, x, y, z = ast.parse(s).body
        self._check_content(s, v, 'v = 1')
        self._check_content(s, w, 'w = 1')
        self._check_content(s, x, 'x = 1')
        self._check_content(s, y, 'y = 1')
        self._check_content(s, z, 'z = 1')

    def test_source_segment_tabs(self):
        s = dedent('''
            class C:
              \t\f  def fun(self) -> None:
              \t\f      pass
        ''').strip()
        s_method = '  \t\f  def fun(self) -> None:\n' \
                   '  \t\f      pass'

        cdef = ast.parse(s).body[0]
        self.assertEqual(ast.get_source_segment(s, cdef.body[0], padded=True), s_method)

    def test_source_segment_missing_info(self):
        s = 'v = 1\r\nw = 1\nx = 1\n\ry = 1\r\n'
        v, w, x, y = ast.parse(s).body
        del v.lineno
        del w.end_lineno
        del x.col_offset
        del y.end_col_offset
        self.assertIsNone(ast.get_source_segment(s, v))
        self.assertIsNone(ast.get_source_segment(s, w))
        self.assertIsNone(ast.get_source_segment(s, x))
        self.assertIsNone(ast.get_source_segment(s, y))

class NodeVisitorTests(unittest.TestCase):
    def test_old_constant_nodes(self):
        class Visitor(ast.NodeVisitor):
            def visit_Num(self, node):
                log.append((node.lineno, 'Num', node.n))
            def visit_Str(self, node):
                log.append((node.lineno, 'Str', node.s))
            def visit_Bytes(self, node):
                log.append((node.lineno, 'Bytes', node.s))
            def visit_NameConstant(self, node):
                log.append((node.lineno, 'NameConstant', node.value))
            def visit_Ellipsis(self, node):
                log.append((node.lineno, 'Ellipsis', ...))
        mod = ast.parse(dedent('''\
            i = 42
            f = 4.25
            c = 4.25j
            s = 'string'
            b = b'bytes'
            t = True
            n = None
            e = ...
            '''))
        visitor = Visitor()
        log = []
        with warnings.catch_warnings(record=True) as wlog:
            warnings.filterwarnings('always', '', DeprecationWarning)
            visitor.visit(mod)
        self.assertEqual(log, [
            (1, 'Num', 42),
            (2, 'Num', 4.25),
            (3, 'Num', 4.25j),
            (4, 'Str', 'string'),
            (5, 'Bytes', b'bytes'),
            (6, 'NameConstant', True),
            (7, 'NameConstant', None),
            (8, 'Ellipsis', ...),
        ])
        self.assertEqual([str(w.message) for w in wlog], [
            'visit_Num is deprecated; add visit_Constant',
            'visit_Num is deprecated; add visit_Constant',
            'visit_Num is deprecated; add visit_Constant',
            'visit_Str is deprecated; add visit_Constant',
            'visit_Bytes is deprecated; add visit_Constant',
            'visit_NameConstant is deprecated; add visit_Constant',
            'visit_NameConstant is deprecated; add visit_Constant',
            'visit_Ellipsis is deprecated; add visit_Constant',
        ])


@support.cpython_only
class ModuleStateTests(unittest.TestCase):
    # bpo-41194, bpo-41261, bpo-41631: The _ast module uses a global state.

    def check_ast_module(self):
        # Check that the _ast module still works as expected
        code = 'x + 1'
        filename = '<string>'
        mode = 'eval'

        # Create _ast.AST subclasses instances
        ast_tree = compile(code, filename, mode, flags=ast.PyCF_ONLY_AST)

        # Call PyAST_Check()
        code = compile(ast_tree, filename, mode)
        self.assertIsInstance(code, types.CodeType)

    def test_reload_module(self):
        # bpo-41194: Importing the _ast module twice must not crash.
        with support.swap_item(sys.modules, '_ast', None):
            del sys.modules['_ast']
            import _ast as ast1

            del sys.modules['_ast']
            import _ast as ast2

            self.check_ast_module()

        # Unloading the two _ast module instances must not crash.
        del ast1
        del ast2
        support.gc_collect()

        self.check_ast_module()

    def test_sys_modules(self):
        # bpo-41631: Test reproducing a Mercurial crash when PyAST_Check()
        # imported the _ast module internally.
        lazy_mod = object()

        def my_import(name, *args, **kw):
            sys.modules[name] = lazy_mod
            return lazy_mod

        with support.swap_item(sys.modules, '_ast', None):
            del sys.modules['_ast']

            with support.swap_attr(builtins, '__import__', my_import):
                # Test that compile() does not import the _ast module
                self.check_ast_module()
                self.assertNotIn('_ast', sys.modules)

                # Sanity check of the test itself
                import _ast
                self.assertIs(_ast, lazy_mod)

    def test_subinterpreter(self):
        # bpo-41631: Importing and using the _ast module in a subinterpreter
        # must not crash.
        code = dedent('''
            import _ast
            import ast
            import gc
            import sys
            import types

            # Create _ast.AST subclasses instances and call PyAST_Check()
            ast_tree = compile('x+1', '<string>', 'eval',
                               flags=ast.PyCF_ONLY_AST)
            code = compile(ast_tree, 'string', 'eval')
            if not isinstance(code, types.CodeType):
                raise AssertionError

            # Unloading the _ast module must not crash.
            del ast, _ast
            del sys.modules['ast'], sys.modules['_ast']
            gc.collect()
        ''')
        res = support.run_in_subinterp(code)
        self.assertEqual(res, 0)


def main():
    if __name__ != '__main__':
        return
    if sys.argv[1:] == ['-g']:
        for statements, kind in ((exec_tests, "exec"), (single_tests, "single"),
                                 (eval_tests, "eval")):
            print(kind+"_results = [")
            for statement in statements:
                tree = ast.parse(statement, "?", kind)
                print("%r," % (to_tuple(tree),))
            print("]")
        print("main()")
        raise SystemExit
    unittest.main()

#### EVERYTHING BELOW IS GENERATED BY python Lib/test/test_ast.py -g  #####
exec_results = [
('Module', [('Expr', (1, 0, 1, 4), ('Constant', (1, 0, 1, 4), None, None))], []),
('Module', [('Expr', (1, 0, 1, 18), ('Constant', (1, 0, 1, 18), 'module docstring', None))], []),
('Module', [('FunctionDef', (1, 0, 1, 13), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (1, 9, 1, 13))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 29), 'f', ('arguments', [], [], None, [], [], None, []), [('Expr', (1, 9, 1, 29), ('Constant', (1, 9, 1, 29), 'function docstring', None))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 14), 'f', ('arguments', [], [('arg', (1, 6, 1, 7), 'a', None, None)], None, [], [], None, []), [('Pass', (1, 10, 1, 14))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 16), 'f', ('arguments', [], [('arg', (1, 6, 1, 7), 'a', None, None)], None, [], [], None, [('Constant', (1, 8, 1, 9), 0, None)]), [('Pass', (1, 12, 1, 16))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 18), 'f', ('arguments', [], [], ('arg', (1, 7, 1, 11), 'args', None, None), [], [], None, []), [('Pass', (1, 14, 1, 18))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 23), 'f', ('arguments', [], [], ('arg', (1, 7, 1, 16), 'args', ('Starred', (1, 13, 1, 16), ('Name', (1, 14, 1, 16), 'Ts', ('Load',)), ('Load',)), None), [], [], None, []), [('Pass', (1, 19, 1, 23))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 36), 'f', ('arguments', [], [], ('arg', (1, 7, 1, 29), 'args', ('Starred', (1, 13, 1, 29), ('Subscript', (1, 14, 1, 29), ('Name', (1, 14, 1, 19), 'tuple', ('Load',)), ('Tuple', (1, 20, 1, 28), [('Name', (1, 20, 1, 23), 'int', ('Load',)), ('Constant', (1, 25, 1, 28), Ellipsis, None)], ('Load',)), ('Load',)), ('Load',)), None), [], [], None, []), [('Pass', (1, 32, 1, 36))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 36), 'f', ('arguments', [], [], ('arg', (1, 7, 1, 29), 'args', ('Starred', (1, 13, 1, 29), ('Subscript', (1, 14, 1, 29), ('Name', (1, 14, 1, 19), 'tuple', ('Load',)), ('Tuple', (1, 20, 1, 28), [('Name', (1, 20, 1, 23), 'int', ('Load',)), ('Starred', (1, 25, 1, 28), ('Name', (1, 26, 1, 28), 'Ts', ('Load',)), ('Load',))], ('Load',)), ('Load',)), ('Load',)), None), [], [], None, []), [('Pass', (1, 32, 1, 36))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 21), 'f', ('arguments', [], [], None, [], [], ('arg', (1, 8, 1, 14), 'kwargs', None, None), []), [('Pass', (1, 17, 1, 21))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 71), 'f', ('arguments', [], [('arg', (1, 6, 1, 7), 'a', None, None), ('arg', (1, 9, 1, 10), 'b', None, None), ('arg', (1, 14, 1, 15), 'c', None, None), ('arg', (1, 22, 1, 23), 'd', None, None), ('arg', (1, 28, 1, 29), 'e', None, None)], ('arg', (1, 35, 1, 39), 'args', None, None), [('arg', (1, 41, 1, 42), 'f', None, None)], [('Constant', (1, 43, 1, 45), 42, None)], ('arg', (1, 49, 1, 55), 'kwargs', None, None), [('Constant', (1, 11, 1, 12), 1, None), ('Constant', (1, 16, 1, 20), None, None), ('List', (1, 24, 1, 26), [], ('Load',)), ('Dict', (1, 30, 1, 32), [], [])]), [('Expr', (1, 58, 1, 71), ('Constant', (1, 58, 1, 71), 'doc for f()', None))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 27), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (1, 23, 1, 27))], [], ('Subscript', (1, 11, 1, 21), ('Name', (1, 11, 1, 16), 'tuple', ('Load',)), ('Tuple', (1, 17, 1, 20), [('Starred', (1, 17, 1, 20), ('Name', (1, 18, 1, 20), 'Ts', ('Load',)), ('Load',))], ('Load',)), ('Load',)), None)], []),
('Module', [('FunctionDef', (1, 0, 1, 32), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (1, 28, 1, 32))], [], ('Subscript', (1, 11, 1, 26), ('Name', (1, 11, 1, 16), 'tuple', ('Load',)), ('Tuple', (1, 17, 1, 25), [('Name', (1, 17, 1, 20), 'int', ('Load',)), ('Starred', (1, 22, 1, 25), ('Name', (1, 23, 1, 25), 'Ts', ('Load',)), ('Load',))], ('Load',)), ('Load',)), None)], []),
('Module', [('FunctionDef', (1, 0, 1, 45), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (1, 41, 1, 45))], [], ('Subscript', (1, 11, 1, 39), ('Name', (1, 11, 1, 16), 'tuple', ('Load',)), ('Tuple', (1, 17, 1, 38), [('Name', (1, 17, 1, 20), 'int', ('Load',)), ('Starred', (1, 22, 1, 38), ('Subscript', (1, 23, 1, 38), ('Name', (1, 23, 1, 28), 'tuple', ('Load',)), ('Tuple', (1, 29, 1, 37), [('Name', (1, 29, 1, 32), 'int', ('Load',)), ('Constant', (1, 34, 1, 37), Ellipsis, None)], ('Load',)), ('Load',)), ('Load',))], ('Load',)), ('Load',)), None)], []),
('Module', [('ClassDef', (1, 0, 1, 12), 'C', [], [], [('Pass', (1, 8, 1, 12))], [])], []),
('Module', [('ClassDef', (1, 0, 1, 32), 'C', [], [], [('Expr', (1, 9, 1, 32), ('Constant', (1, 9, 1, 32), 'docstring for class C', None))], [])], []),
('Module', [('ClassDef', (1, 0, 1, 21), 'C', [('Name', (1, 8, 1, 14), 'object', ('Load',))], [], [('Pass', (1, 17, 1, 21))], [])], []),
('Module', [('FunctionDef', (1, 0, 1, 16), 'f', ('arguments', [], [], None, [], [], None, []), [('Return', (1, 8, 1, 16), ('Constant', (1, 15, 1, 16), 1, None))], [], None, None)], []),
('Module', [('Delete', (1, 0, 1, 5), [('Name', (1, 4, 1, 5), 'v', ('Del',))])], []),
('Module', [('Assign', (1, 0, 1, 5), [('Name', (1, 0, 1, 1), 'v', ('Store',))], ('Constant', (1, 4, 1, 5), 1, None), None)], []),
('Module', [('Assign', (1, 0, 1, 7), [('Tuple', (1, 0, 1, 3), [('Name', (1, 0, 1, 1), 'a', ('Store',)), ('Name', (1, 2, 1, 3), 'b', ('Store',))], ('Store',))], ('Name', (1, 6, 1, 7), 'c', ('Load',)), None)], []),
('Module', [('Assign', (1, 0, 1, 9), [('Tuple', (1, 0, 1, 5), [('Name', (1, 1, 1, 2), 'a', ('Store',)), ('Name', (1, 3, 1, 4), 'b', ('Store',))], ('Store',))], ('Name', (1, 8, 1, 9), 'c', ('Load',)), None)], []),
('Module', [('Assign', (1, 0, 1, 9), [('List', (1, 0, 1, 5), [('Name', (1, 1, 1, 2), 'a', ('Store',)), ('Name', (1, 3, 1, 4), 'b', ('Store',))], ('Store',))], ('Name', (1, 8, 1, 9), 'c', ('Load',)), None)], []),
('Module', [('AnnAssign', (1, 0, 1, 13), ('Name', (1, 0, 1, 1), 'x', ('Store',)), ('Subscript', (1, 3, 1, 13), ('Name', (1, 3, 1, 8), 'tuple', ('Load',)), ('Tuple', (1, 9, 1, 12), [('Starred', (1, 9, 1, 12), ('Name', (1, 10, 1, 12), 'Ts', ('Load',)), ('Load',))], ('Load',)), ('Load',)), None, 1)], []),
('Module', [('AnnAssign', (1, 0, 1, 18), ('Name', (1, 0, 1, 1), 'x', ('Store',)), ('Subscript', (1, 3, 1, 18), ('Name', (1, 3, 1, 8), 'tuple', ('Load',)), ('Tuple', (1, 9, 1, 17), [('Name', (1, 9, 1, 12), 'int', ('Load',)), ('Starred', (1, 14, 1, 17), ('Name', (1, 15, 1, 17), 'Ts', ('Load',)), ('Load',))], ('Load',)), ('Load',)), None, 1)], []),
('Module', [('AnnAssign', (1, 0, 1, 31), ('Name', (1, 0, 1, 1), 'x', ('Store',)), ('Subscript', (1, 3, 1, 31), ('Name', (1, 3, 1, 8), 'tuple', ('Load',)), ('Tuple', (1, 9, 1, 30), [('Name', (1, 9, 1, 12), 'int', ('Load',)), ('Starred', (1, 14, 1, 30), ('Subscript', (1, 15, 1, 30), ('Name', (1, 15, 1, 20), 'tuple', ('Load',)), ('Tuple', (1, 21, 1, 29), [('Name', (1, 21, 1, 24), 'str', ('Load',)), ('Constant', (1, 26, 1, 29), Ellipsis, None)], ('Load',)), ('Load',)), ('Load',))], ('Load',)), ('Load',)), None, 1)], []),
('Module', [('AugAssign', (1, 0, 1, 6), ('Name', (1, 0, 1, 1), 'v', ('Store',)), ('Add',), ('Constant', (1, 5, 1, 6), 1, None))], []),
('Module', [('For', (1, 0, 1, 15), ('Name', (1, 4, 1, 5), 'v', ('Store',)), ('Name', (1, 9, 1, 10), 'v', ('Load',)), [('Pass', (1, 11, 1, 15))], [], None)], []),
('Module', [('While', (1, 0, 1, 12), ('Name', (1, 6, 1, 7), 'v', ('Load',)), [('Pass', (1, 8, 1, 12))], [])], []),
('Module', [('If', (1, 0, 1, 9), ('Name', (1, 3, 1, 4), 'v', ('Load',)), [('Pass', (1, 5, 1, 9))], [])], []),
('Module', [('If', (1, 0, 4, 6), ('Name', (1, 3, 1, 4), 'a', ('Load',)), [('Pass', (2, 2, 2, 6))], [('If', (3, 0, 4, 6), ('Name', (3, 5, 3, 6), 'b', ('Load',)), [('Pass', (4, 2, 4, 6))], [])])], []),
('Module', [('If', (1, 0, 6, 6), ('Name', (1, 3, 1, 4), 'a', ('Load',)), [('Pass', (2, 2, 2, 6))], [('If', (3, 0, 6, 6), ('Name', (3, 5, 3, 6), 'b', ('Load',)), [('Pass', (4, 2, 4, 6))], [('Pass', (6, 2, 6, 6))])])], []),
('Module', [('With', (1, 0, 1, 17), [('withitem', ('Name', (1, 5, 1, 6), 'x', ('Load',)), ('Name', (1, 10, 1, 11), 'y', ('Store',)))], [('Pass', (1, 13, 1, 17))], None)], []),
('Module', [('With', (1, 0, 1, 25), [('withitem', ('Name', (1, 5, 1, 6), 'x', ('Load',)), ('Name', (1, 10, 1, 11), 'y', ('Store',))), ('withitem', ('Name', (1, 13, 1, 14), 'z', ('Load',)), ('Name', (1, 18, 1, 19), 'q', ('Store',)))], [('Pass', (1, 21, 1, 25))], None)], []),
('Module', [('Raise', (1, 0, 1, 25), ('Call', (1, 6, 1, 25), ('Name', (1, 6, 1, 15), 'Exception', ('Load',)), [('Constant', (1, 16, 1, 24), 'string', None)], []), None)], []),
('Module', [('Try', (1, 0, 4, 6), [('Pass', (2, 2, 2, 6))], [('ExceptHandler', (3, 0, 4, 6), ('Name', (3, 7, 3, 16), 'Exception', ('Load',)), None, [('Pass', (4, 2, 4, 6))])], [], [])], []),
('Module', [('Try', (1, 0, 4, 6), [('Pass', (2, 2, 2, 6))], [], [], [('Pass', (4, 2, 4, 6))])], []),
('Module', [('TryStar', (1, 0, 4, 6), [('Pass', (2, 2, 2, 6))], [('ExceptHandler', (3, 0, 4, 6), ('Name', (3, 8, 3, 17), 'Exception', ('Load',)), None, [('Pass', (4, 2, 4, 6))])], [], [])], []),
('Module', [('Assert', (1, 0, 1, 8), ('Name', (1, 7, 1, 8), 'v', ('Load',)), None)], []),
('Module', [('Import', (1, 0, 1, 10), [('alias', (1, 7, 1, 10), 'sys', None)])], []),
('Module', [('ImportFrom', (1, 0, 1, 17), 'sys', [('alias', (1, 16, 1, 17), 'v', None)], 0)], []),
('Module', [('Global', (1, 0, 1, 8), ['v'])], []),
('Module', [('Expr', (1, 0, 1, 1), ('Constant', (1, 0, 1, 1), 1, None))], []),
('Module', [('Pass', (1, 0, 1, 4))], []),
('Module', [('For', (1, 0, 1, 16), ('Name', (1, 4, 1, 5), 'v', ('Store',)), ('Name', (1, 9, 1, 10), 'v', ('Load',)), [('Break', (1, 11, 1, 16))], [], None)], []),
('Module', [('For', (1, 0, 1, 19), ('Name', (1, 4, 1, 5), 'v', ('Store',)), ('Name', (1, 9, 1, 10), 'v', ('Load',)), [('Continue', (1, 11, 1, 19))], [], None)], []),
('Module', [('For', (1, 0, 1, 18), ('Tuple', (1, 4, 1, 7), [('Name', (1, 4, 1, 5), 'a', ('Store',)), ('Name', (1, 6, 1, 7), 'b', ('Store',))], ('Store',)), ('Name', (1, 11, 1, 12), 'c', ('Load',)), [('Pass', (1, 14, 1, 18))], [], None)], []),
('Module', [('For', (1, 0, 1, 20), ('Tuple', (1, 4, 1, 9), [('Name', (1, 5, 1, 6), 'a', ('Store',)), ('Name', (1, 7, 1, 8), 'b', ('Store',))], ('Store',)), ('Name', (1, 13, 1, 14), 'c', ('Load',)), [('Pass', (1, 16, 1, 20))], [], None)], []),
('Module', [('For', (1, 0, 1, 20), ('List', (1, 4, 1, 9), [('Name', (1, 5, 1, 6), 'a', ('Store',)), ('Name', (1, 7, 1, 8), 'b', ('Store',))], ('Store',)), ('Name', (1, 13, 1, 14), 'c', ('Load',)), [('Pass', (1, 16, 1, 20))], [], None)], []),
('Module', [('Expr', (1, 0, 11, 5), ('GeneratorExp', (1, 0, 11, 5), ('Tuple', (2, 4, 6, 5), [('Name', (3, 4, 3, 6), 'Aa', ('Load',)), ('Name', (5, 7, 5, 9), 'Bb', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (8, 4, 10, 6), [('Name', (8, 4, 8, 6), 'Aa', ('Store',)), ('Name', (10, 4, 10, 6), 'Bb', ('Store',))], ('Store',)), ('Name', (10, 10, 10, 12), 'Cc', ('Load',)), [], 0)]))], []),
('Module', [('Expr', (1, 0, 1, 34), ('DictComp', (1, 0, 1, 34), ('Name', (1, 1, 1, 2), 'a', ('Load',)), ('Name', (1, 5, 1, 6), 'b', ('Load',)), [('comprehension', ('Name', (1, 11, 1, 12), 'w', ('Store',)), ('Name', (1, 16, 1, 17), 'x', ('Load',)), [], 0), ('comprehension', ('Name', (1, 22, 1, 23), 'm', ('Store',)), ('Name', (1, 27, 1, 28), 'p', ('Load',)), [('Name', (1, 32, 1, 33), 'g', ('Load',))], 0)]))], []),
('Module', [('Expr', (1, 0, 1, 20), ('DictComp', (1, 0, 1, 20), ('Name', (1, 1, 1, 2), 'a', ('Load',)), ('Name', (1, 5, 1, 6), 'b', ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 14), [('Name', (1, 11, 1, 12), 'v', ('Store',)), ('Name', (1, 13, 1, 14), 'w', ('Store',))], ('Store',)), ('Name', (1, 18, 1, 19), 'x', ('Load',)), [], 0)]))], []),
('Module', [('Expr', (1, 0, 1, 19), ('SetComp', (1, 0, 1, 19), ('Name', (1, 1, 1, 2), 'r', ('Load',)), [('comprehension', ('Name', (1, 7, 1, 8), 'l', ('Store',)), ('Name', (1, 12, 1, 13), 'x', ('Load',)), [('Name', (1, 17, 1, 18), 'g', ('Load',))], 0)]))], []),
('Module', [('Expr', (1, 0, 1, 16), ('SetComp', (1, 0, 1, 16), ('Name', (1, 1, 1, 2), 'r', ('Load',)), [('comprehension', ('Tuple', (1, 7, 1, 10), [('Name', (1, 7, 1, 8), 'l', ('Store',)), ('Name', (1, 9, 1, 10), 'm', ('Store',))], ('Store',)), ('Name', (1, 14, 1, 15), 'x', ('Load',)), [], 0)]))], []),
('Module', [('AsyncFunctionDef', (1, 0, 3, 18), 'f', ('arguments', [], [], None, [], [], None, []), [('Expr', (2, 1, 2, 17), ('Constant', (2, 1, 2, 17), 'async function', None)), ('Expr', (3, 1, 3, 18), ('Await', (3, 1, 3, 18), ('Call', (3, 7, 3, 18), ('Name', (3, 7, 3, 16), 'something', ('Load',)), [], [])))], [], None, None)], []),
('Module', [('AsyncFunctionDef', (1, 0, 3, 8), 'f', ('arguments', [], [], None, [], [], None, []), [('AsyncFor', (2, 1, 3, 8), ('Name', (2, 11, 2, 12), 'e', ('Store',)), ('Name', (2, 16, 2, 17), 'i', ('Load',)), [('Expr', (2, 19, 2, 20), ('Constant', (2, 19, 2, 20), 1, None))], [('Expr', (3, 7, 3, 8), ('Constant', (3, 7, 3, 8), 2, None))], None)], [], None, None)], []),
('Module', [('AsyncFunctionDef', (1, 0, 2, 21), 'f', ('arguments', [], [], None, [], [], None, []), [('AsyncWith', (2, 1, 2, 21), [('withitem', ('Name', (2, 12, 2, 13), 'a', ('Load',)), ('Name', (2, 17, 2, 18), 'b', ('Store',)))], [('Expr', (2, 20, 2, 21), ('Constant', (2, 20, 2, 21), 1, None))], None)], [], None, None)], []),
('Module', [('Expr', (1, 0, 1, 14), ('Dict', (1, 0, 1, 14), [None, ('Constant', (1, 10, 1, 11), 2, None)], [('Dict', (1, 3, 1, 8), [('Constant', (1, 4, 1, 5), 1, None)], [('Constant', (1, 6, 1, 7), 2, None)]), ('Constant', (1, 12, 1, 13), 3, None)]))], []),
('Module', [('Expr', (1, 0, 1, 12), ('Set', (1, 0, 1, 12), [('Starred', (1, 1, 1, 8), ('Set', (1, 2, 1, 8), [('Constant', (1, 3, 1, 4), 1, None), ('Constant', (1, 6, 1, 7), 2, None)]), ('Load',)), ('Constant', (1, 10, 1, 11), 3, None)]))], []),
('Module', [('AsyncFunctionDef', (1, 0, 2, 21), 'f', ('arguments', [], [], None, [], [], None, []), [('Expr', (2, 1, 2, 21), ('ListComp', (2, 1, 2, 21), ('Name', (2, 2, 2, 3), 'i', ('Load',)), [('comprehension', ('Name', (2, 14, 2, 15), 'b', ('Store',)), ('Name', (2, 19, 2, 20), 'c', ('Load',)), [], 1)]))], [], None, None)], []),
('Module', [('FunctionDef', (4, 0, 4, 13), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (4, 9, 4, 13))], [('Name', (1, 1, 1, 6), 'deco1', ('Load',)), ('Call', (2, 1, 2, 8), ('Name', (2, 1, 2, 6), 'deco2', ('Load',)), [], []), ('Call', (3, 1, 3, 9), ('Name', (3, 1, 3, 6), 'deco3', ('Load',)), [('Constant', (3, 7, 3, 8), 1, None)], [])], None, None)], []),
('Module', [('AsyncFunctionDef', (4, 0, 4, 19), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (4, 15, 4, 19))], [('Name', (1, 1, 1, 6), 'deco1', ('Load',)), ('Call', (2, 1, 2, 8), ('Name', (2, 1, 2, 6), 'deco2', ('Load',)), [], []), ('Call', (3, 1, 3, 9), ('Name', (3, 1, 3, 6), 'deco3', ('Load',)), [('Constant', (3, 7, 3, 8), 1, None)], [])], None, None)], []),
('Module', [('ClassDef', (4, 0, 4, 13), 'C', [], [], [('Pass', (4, 9, 4, 13))], [('Name', (1, 1, 1, 6), 'deco1', ('Load',)), ('Call', (2, 1, 2, 8), ('Name', (2, 1, 2, 6), 'deco2', ('Load',)), [], []), ('Call', (3, 1, 3, 9), ('Name', (3, 1, 3, 6), 'deco3', ('Load',)), [('Constant', (3, 7, 3, 8), 1, None)], [])])], []),
('Module', [('FunctionDef', (2, 0, 2, 13), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (2, 9, 2, 13))], [('Call', (1, 1, 1, 19), ('Name', (1, 1, 1, 5), 'deco', ('Load',)), [('GeneratorExp', (1, 5, 1, 19), ('Name', (1, 6, 1, 7), 'a', ('Load',)), [('comprehension', ('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 17, 1, 18), 'b', ('Load',)), [], 0)])], [])], None, None)], []),
('Module', [('FunctionDef', (2, 0, 2, 13), 'f', ('arguments', [], [], None, [], [], None, []), [('Pass', (2, 9, 2, 13))], [('Attribute', (1, 1, 1, 6), ('Attribute', (1, 1, 1, 4), ('Name', (1, 1, 1, 2), 'a', ('Load',)), 'b', ('Load',)), 'c', ('Load',))], None, None)], []),
('Module', [('Expr', (1, 0, 1, 8), ('NamedExpr', (1, 1, 1, 7), ('Name', (1, 1, 1, 2), 'a', ('Store',)), ('Constant', (1, 6, 1, 7), 1, None)))], []),
('Module', [('FunctionDef', (1, 0, 1, 18), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [], None, [], [], None, []), [('Pass', (1, 14, 1, 18))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 26), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 12, 1, 13), 'c', None, None), ('arg', (1, 15, 1, 16), 'd', None, None), ('arg', (1, 18, 1, 19), 'e', None, None)], None, [], [], None, []), [('Pass', (1, 22, 1, 26))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 29), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 12, 1, 13), 'c', None, None)], None, [('arg', (1, 18, 1, 19), 'd', None, None), ('arg', (1, 21, 1, 22), 'e', None, None)], [None, None], None, []), [('Pass', (1, 25, 1, 29))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 39), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 12, 1, 13), 'c', None, None)], None, [('arg', (1, 18, 1, 19), 'd', None, None), ('arg', (1, 21, 1, 22), 'e', None, None)], [None, None], ('arg', (1, 26, 1, 32), 'kwargs', None, None), []), [('Pass', (1, 35, 1, 39))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 20), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [], None, [], [], None, [('Constant', (1, 8, 1, 9), 1, None)]), [('Pass', (1, 16, 1, 20))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 29), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 14, 1, 15), 'b', None, None), ('arg', (1, 19, 1, 20), 'c', None, None)], None, [], [], None, [('Constant', (1, 8, 1, 9), 1, None), ('Constant', (1, 16, 1, 17), 2, None), ('Constant', (1, 21, 1, 22), 4, None)]), [('Pass', (1, 25, 1, 29))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 32), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 14, 1, 15), 'b', None, None)], None, [('arg', (1, 22, 1, 23), 'c', None, None)], [('Constant', (1, 24, 1, 25), 4, None)], None, [('Constant', (1, 8, 1, 9), 1, None), ('Constant', (1, 16, 1, 17), 2, None)]), [('Pass', (1, 28, 1, 32))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 30), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 14, 1, 15), 'b', None, None)], None, [('arg', (1, 22, 1, 23), 'c', None, None)], [None], None, [('Constant', (1, 8, 1, 9), 1, None), ('Constant', (1, 16, 1, 17), 2, None)]), [('Pass', (1, 26, 1, 30))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 42), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 14, 1, 15), 'b', None, None)], None, [('arg', (1, 22, 1, 23), 'c', None, None)], [('Constant', (1, 24, 1, 25), 4, None)], ('arg', (1, 29, 1, 35), 'kwargs', None, None), [('Constant', (1, 8, 1, 9), 1, None), ('Constant', (1, 16, 1, 17), 2, None)]), [('Pass', (1, 38, 1, 42))], [], None, None)], []),
('Module', [('FunctionDef', (1, 0, 1, 40), 'f', ('arguments', [('arg', (1, 6, 1, 7), 'a', None, None)], [('arg', (1, 14, 1, 15), 'b', None, None)], None, [('arg', (1, 22, 1, 23), 'c', None, None)], [None], ('arg', (1, 27, 1, 33), 'kwargs', None, None), [('Constant', (1, 8, 1, 9), 1, None), ('Constant', (1, 16, 1, 17), 2, None)]), [('Pass', (1, 36, 1, 40))], [], None, None)], []),
]
single_results = [
('Interactive', [('Expr', (1, 0, 1, 3), ('BinOp', (1, 0, 1, 3), ('Constant', (1, 0, 1, 1), 1, None), ('Add',), ('Constant', (1, 2, 1, 3), 2, None)))]),
]
eval_results = [
('Expression', ('Constant', (1, 0, 1, 4), None, None)),
('Expression', ('BoolOp', (1, 0, 1, 7), ('And',), [('Name', (1, 0, 1, 1), 'a', ('Load',)), ('Name', (1, 6, 1, 7), 'b', ('Load',))])),
('Expression', ('BinOp', (1, 0, 1, 5), ('Name', (1, 0, 1, 1), 'a', ('Load',)), ('Add',), ('Name', (1, 4, 1, 5), 'b', ('Load',)))),
('Expression', ('UnaryOp', (1, 0, 1, 5), ('Not',), ('Name', (1, 4, 1, 5), 'v', ('Load',)))),
('Expression', ('Lambda', (1, 0, 1, 11), ('arguments', [], [], None, [], [], None, []), ('Constant', (1, 7, 1, 11), None, None))),
('Expression', ('Dict', (1, 0, 1, 7), [('Constant', (1, 2, 1, 3), 1, None)], [('Constant', (1, 4, 1, 5), 2, None)])),
('Expression', ('Dict', (1, 0, 1, 2), [], [])),
('Expression', ('Set', (1, 0, 1, 7), [('Constant', (1, 1, 1, 5), None, None)])),
('Expression', ('Dict', (1, 0, 5, 6), [('Constant', (2, 6, 2, 7), 1, None)], [('Constant', (4, 10, 4, 11), 2, None)])),
('Expression', ('ListComp', (1, 0, 1, 19), ('Name', (1, 1, 1, 2), 'a', ('Load',)), [('comprehension', ('Name', (1, 7, 1, 8), 'b', ('Store',)), ('Name', (1, 12, 1, 13), 'c', ('Load',)), [('Name', (1, 17, 1, 18), 'd', ('Load',))], 0)])),
('Expression', ('GeneratorExp', (1, 0, 1, 19), ('Name', (1, 1, 1, 2), 'a', ('Load',)), [('comprehension', ('Name', (1, 7, 1, 8), 'b', ('Store',)), ('Name', (1, 12, 1, 13), 'c', ('Load',)), [('Name', (1, 17, 1, 18), 'd', ('Load',))], 0)])),
('Expression', ('ListComp', (1, 0, 1, 20), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 14), [('Name', (1, 11, 1, 12), 'a', ('Store',)), ('Name', (1, 13, 1, 14), 'b', ('Store',))], ('Store',)), ('Name', (1, 18, 1, 19), 'c', ('Load',)), [], 0)])),
('Expression', ('ListComp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('ListComp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('List', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('SetComp', (1, 0, 1, 20), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 14), [('Name', (1, 11, 1, 12), 'a', ('Store',)), ('Name', (1, 13, 1, 14), 'b', ('Store',))], ('Store',)), ('Name', (1, 18, 1, 19), 'c', ('Load',)), [], 0)])),
('Expression', ('SetComp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('SetComp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('List', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('GeneratorExp', (1, 0, 1, 20), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 14), [('Name', (1, 11, 1, 12), 'a', ('Store',)), ('Name', (1, 13, 1, 14), 'b', ('Store',))], ('Store',)), ('Name', (1, 18, 1, 19), 'c', ('Load',)), [], 0)])),
('Expression', ('GeneratorExp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('Tuple', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('GeneratorExp', (1, 0, 1, 22), ('Tuple', (1, 1, 1, 6), [('Name', (1, 2, 1, 3), 'a', ('Load',)), ('Name', (1, 4, 1, 5), 'b', ('Load',))], ('Load',)), [('comprehension', ('List', (1, 11, 1, 16), [('Name', (1, 12, 1, 13), 'a', ('Store',)), ('Name', (1, 14, 1, 15), 'b', ('Store',))], ('Store',)), ('Name', (1, 20, 1, 21), 'c', ('Load',)), [], 0)])),
('Expression', ('Compare', (1, 0, 1, 9), ('Constant', (1, 0, 1, 1), 1, None), [('Lt',), ('Lt',)], [('Constant', (1, 4, 1, 5), 2, None), ('Constant', (1, 8, 1, 9), 3, None)])),
('Expression', ('Call', (1, 0, 1, 17), ('Name', (1, 0, 1, 1), 'f', ('Load',)), [('Constant', (1, 2, 1, 3), 1, None), ('Constant', (1, 4, 1, 5), 2, None), ('Starred', (1, 10, 1, 12), ('Name', (1, 11, 1, 12), 'd', ('Load',)), ('Load',))], [('keyword', (1, 6, 1, 9), 'c', ('Constant', (1, 8, 1, 9), 3, None)), ('keyword', (1, 13, 1, 16), None, ('Name', (1, 15, 1, 16), 'e', ('Load',)))])),
('Expression', ('Call', (1, 0, 1, 10), ('Name', (1, 0, 1, 1), 'f', ('Load',)), [('Starred', (1, 2, 1, 9), ('List', (1, 3, 1, 9), [('Constant', (1, 4, 1, 5), 0, None), ('Constant', (1, 7, 1, 8), 1, None)], ('Load',)), ('Load',))], [])),
('Expression', ('Call', (1, 0, 1, 15), ('Name', (1, 0, 1, 1), 'f', ('Load',)), [('GeneratorExp', (1, 1, 1, 15), ('Name', (1, 2, 1, 3), 'a', ('Load',)), [('comprehension', ('Name', (1, 8, 1, 9), 'a', ('Store',)), ('Name', (1, 13, 1, 14), 'b', ('Load',)), [], 0)])], [])),
('Expression', ('Constant', (1, 0, 1, 2), 10, None)),
('Expression', ('Constant', (1, 0, 1, 8), 'string', None)),
('Expression', ('Attribute', (1, 0, 1, 3), ('Name', (1, 0, 1, 1), 'a', ('Load',)), 'b', ('Load',))),
('Expression', ('Subscript', (1, 0, 1, 6), ('Name', (1, 0, 1, 1), 'a', ('Load',)), ('Slice', (1, 2, 1, 5), ('Name', (1, 2, 1, 3), 'b', ('Load',)), ('Name', (1, 4, 1, 5), 'c', ('Load',)), None), ('Load',))),
('Expression', ('Name', (1, 0, 1, 1), 'v', ('Load',))),
('Expression', ('List', (1, 0, 1, 7), [('Constant', (1, 1, 1, 2), 1, None), ('Constant', (1, 3, 1, 4), 2, None), ('Constant', (1, 5, 1, 6), 3, None)], ('Load',))),
('Expression', ('List', (1, 0, 1, 2), [], ('Load',))),
('Expression', ('Tuple', (1, 0, 1, 5), [('Constant', (1, 0, 1, 1), 1, None), ('Constant', (1, 2, 1, 3), 2, None), ('Constant', (1, 4, 1, 5), 3, None)], ('Load',))),
('Expression', ('Tuple', (1, 0, 1, 7), [('Constant', (1, 1, 1, 2), 1, None), ('Constant', (1, 3, 1, 4), 2, None), ('Constant', (1, 5, 1, 6), 3, None)], ('Load',))),
('Expression', ('Tuple', (1, 0, 1, 2), [], ('Load',))),
('Expression', ('Call', (1, 0, 1, 17), ('Attribute', (1, 0, 1, 7), ('Attribute', (1, 0, 1, 5), ('Attribute', (1, 0, 1, 3), ('Name', (1, 0, 1, 1), 'a', ('Load',)), 'b', ('Load',)), 'c', ('Load',)), 'd', ('Load',)), [('Subscript', (1, 8, 1, 16), ('Attribute', (1, 8, 1, 11), ('Name', (1, 8, 1, 9), 'a', ('Load',)), 'b', ('Load',)), ('Slice', (1, 12, 1, 15), ('Constant', (1, 12, 1, 13), 1, None), ('Constant', (1, 14, 1, 15), 2, None), None), ('Load',))], [])),
]
main()
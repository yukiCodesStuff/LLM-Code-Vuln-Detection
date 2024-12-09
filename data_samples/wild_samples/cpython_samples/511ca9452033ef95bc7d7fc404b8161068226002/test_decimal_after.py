class PyUsabilityTest(UsabilityTest):
    decimal = P

    def setUp(self):
        super().setUp()
        self._previous_int_limit = sys.get_int_max_str_digits()
        sys.set_int_max_str_digits(7000)

    def tearDown(self):
        sys.set_int_max_str_digits(self._previous_int_limit)
        super().tearDown()

class PythonAPItests(unittest.TestCase):

    def test_abc(self):
        Decimal = self.decimal.Decimal

        self.assertTrue(issubclass(Decimal, numbers.Number))
        self.assertFalse(issubclass(Decimal, numbers.Real))
        self.assertIsInstance(Decimal(0), numbers.Number)
        self.assertNotIsInstance(Decimal(0), numbers.Real)

    def test_pickle(self):
        for proto in range(pickle.HIGHEST_PROTOCOL + 1):
            Decimal = self.decimal.Decimal

            savedecimal = sys.modules['decimal']

            # Round trip
            sys.modules['decimal'] = self.decimal
            d = Decimal('-3.141590000')
            p = pickle.dumps(d, proto)
            e = pickle.loads(p)
            self.assertEqual(d, e)

            if C:
                # Test interchangeability
                x = C.Decimal('-3.123e81723')
                y = P.Decimal('-3.123e81723')

                sys.modules['decimal'] = C
                sx = pickle.dumps(x, proto)
                sys.modules['decimal'] = P
                r = pickle.loads(sx)
                self.assertIsInstance(r, P.Decimal)
                self.assertEqual(r, y)

                sys.modules['decimal'] = P
                sy = pickle.dumps(y, proto)
                sys.modules['decimal'] = C
                r = pickle.loads(sy)
                self.assertIsInstance(r, C.Decimal)
                self.assertEqual(r, x)

                x = C.Decimal('-3.123e81723').as_tuple()
                y = P.Decimal('-3.123e81723').as_tuple()

                sys.modules['decimal'] = C
                sx = pickle.dumps(x, proto)
                sys.modules['decimal'] = P
                r = pickle.loads(sx)
                self.assertIsInstance(r, P.DecimalTuple)
                self.assertEqual(r, y)

                sys.modules['decimal'] = P
                sy = pickle.dumps(y, proto)
                sys.modules['decimal'] = C
                r = pickle.loads(sy)
                self.assertIsInstance(r, C.DecimalTuple)
                self.assertEqual(r, x)

            sys.modules['decimal'] = savedecimal

    def test_int(self):
        Decimal = self.decimal.Decimal

        for x in range(-250, 250):
            s = '%0.2f' % (x / 100.0)
            # should work the same as for floats
            self.assertEqual(int(Decimal(s)), int(float(s)))
            # should work the same as to_integral in the ROUND_DOWN mode
            d = Decimal(s)
            r = d.to_integral(ROUND_DOWN)
            self.assertEqual(Decimal(int(d)), r)

        self.assertRaises(ValueError, int, Decimal('-nan'))
        self.assertRaises(ValueError, int, Decimal('snan'))
        self.assertRaises(OverflowError, int, Decimal('inf'))
        self.assertRaises(OverflowError, int, Decimal('-inf'))

    @cpython_only
    def test_small_ints(self):
        Decimal = self.decimal.Decimal
        # bpo-46361
        for x in range(-5, 257):
            self.assertIs(int(Decimal(x)), x)

    def test_trunc(self):
        Decimal = self.decimal.Decimal

        for x in range(-250, 250):
            s = '%0.2f' % (x / 100.0)
            # should work the same as for floats
            self.assertEqual(int(Decimal(s)), int(float(s)))
            # should work the same as to_integral in the ROUND_DOWN mode
            d = Decimal(s)
            r = d.to_integral(ROUND_DOWN)
            self.assertEqual(Decimal(math.trunc(d)), r)

    def test_from_float(self):

        Decimal = self.decimal.Decimal

        class MyDecimal(Decimal):
            def __init__(self, _):
                self.x = 'y'

        self.assertTrue(issubclass(MyDecimal, Decimal))

        r = MyDecimal.from_float(0.1)
        self.assertEqual(type(r), MyDecimal)
        self.assertEqual(str(r),
                '0.1000000000000000055511151231257827021181583404541015625')
        self.assertEqual(r.x, 'y')

        bigint = 12345678901234567890123456789
        self.assertEqual(MyDecimal.from_float(bigint), MyDecimal(bigint))
        self.assertTrue(MyDecimal.from_float(float('nan')).is_qnan())
        self.assertTrue(MyDecimal.from_float(float('inf')).is_infinite())
        self.assertTrue(MyDecimal.from_float(float('-inf')).is_infinite())
        self.assertEqual(str(MyDecimal.from_float(float('nan'))),
                         str(Decimal('NaN')))
        self.assertEqual(str(MyDecimal.from_float(float('inf'))),
                         str(Decimal('Infinity')))
        self.assertEqual(str(MyDecimal.from_float(float('-inf'))),
                         str(Decimal('-Infinity')))
        self.assertRaises(TypeError, MyDecimal.from_float, 'abc')
        for i in range(200):
            x = random.expovariate(0.01) * (random.random() * 2.0 - 1.0)
            self.assertEqual(x, float(MyDecimal.from_float(x))) # roundtrip

    def test_create_decimal_from_float(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        Inexact = self.decimal.Inexact

        context = Context(prec=5, rounding=ROUND_DOWN)
        self.assertEqual(
            context.create_decimal_from_float(math.pi),
            Decimal('3.1415')
        )
        context = Context(prec=5, rounding=ROUND_UP)
        self.assertEqual(
            context.create_decimal_from_float(math.pi),
            Decimal('3.1416')
        )
        context = Context(prec=5, traps=[Inexact])
        self.assertRaises(
            Inexact,
            context.create_decimal_from_float,
            math.pi
        )
        self.assertEqual(repr(context.create_decimal_from_float(-0.0)),
                         "Decimal('-0')")
        self.assertEqual(repr(context.create_decimal_from_float(1.0)),
                         "Decimal('1')")
        self.assertEqual(repr(context.create_decimal_from_float(10)),
                         "Decimal('10')")

    def test_quantize(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        InvalidOperation = self.decimal.InvalidOperation

        c = Context(Emax=99999, Emin=-99999)
        self.assertEqual(
            Decimal('7.335').quantize(Decimal('.01')),
            Decimal('7.34')
        )
        self.assertEqual(
            Decimal('7.335').quantize(Decimal('.01'), rounding=ROUND_DOWN),
            Decimal('7.33')
        )
        self.assertRaises(
            InvalidOperation,
            Decimal("10e99999").quantize, Decimal('1e100000'), context=c
        )

        c = Context()
        d = Decimal("0.871831e800")
        x = d.quantize(context=c, exp=Decimal("1e797"), rounding=ROUND_DOWN)
        self.assertEqual(x, Decimal('8.71E+799'))

    def test_complex(self):
        Decimal = self.decimal.Decimal

        x = Decimal("9.8182731e181273")
        self.assertEqual(x.real, x)
        self.assertEqual(x.imag, 0)
        self.assertEqual(x.conjugate(), x)

        x = Decimal("1")
        self.assertEqual(complex(x), complex(float(1)))

        self.assertRaises(AttributeError, setattr, x, 'real', 100)
        self.assertRaises(AttributeError, setattr, x, 'imag', 100)
        self.assertRaises(AttributeError, setattr, x, 'conjugate', 100)
        self.assertRaises(AttributeError, setattr, x, '__complex__', 100)

    def test_named_parameters(self):
        D = self.decimal.Decimal
        Context = self.decimal.Context
        localcontext = self.decimal.localcontext
        InvalidOperation = self.decimal.InvalidOperation
        Overflow = self.decimal.Overflow

        xc = Context()
        xc.prec = 1
        xc.Emax = 1
        xc.Emin = -1

        with localcontext() as c:
            c.clear_flags()

            self.assertEqual(D(9, xc), 9)
            self.assertEqual(D(9, context=xc), 9)
            self.assertEqual(D(context=xc, value=9), 9)
            self.assertEqual(D(context=xc), 0)
            xc.clear_flags()
            self.assertRaises(InvalidOperation, D, "xyz", context=xc)
            self.assertTrue(xc.flags[InvalidOperation])
            self.assertFalse(c.flags[InvalidOperation])

            xc.clear_flags()
            self.assertEqual(D(2).exp(context=xc), 7)
            self.assertRaises(Overflow, D(8).exp, context=xc)
            self.assertTrue(xc.flags[Overflow])
            self.assertFalse(c.flags[Overflow])

            xc.clear_flags()
            self.assertEqual(D(2).ln(context=xc), D('0.7'))
            self.assertRaises(InvalidOperation, D(-1).ln, context=xc)
            self.assertTrue(xc.flags[InvalidOperation])
            self.assertFalse(c.flags[InvalidOperation])

            self.assertEqual(D(0).log10(context=xc), D('-inf'))
            self.assertEqual(D(-1).next_minus(context=xc), -2)
            self.assertEqual(D(-1).next_plus(context=xc), D('-0.9'))
            self.assertEqual(D("9.73").normalize(context=xc), D('1E+1'))
            self.assertEqual(D("9999").to_integral(context=xc), 9999)
            self.assertEqual(D("-2000").to_integral_exact(context=xc), -2000)
            self.assertEqual(D("123").to_integral_value(context=xc), 123)
            self.assertEqual(D("0.0625").sqrt(context=xc), D('0.2'))

            self.assertEqual(D("0.0625").compare(context=xc, other=3), -1)
            xc.clear_flags()
            self.assertRaises(InvalidOperation,
                              D("0").compare_signal, D('nan'), context=xc)
            self.assertTrue(xc.flags[InvalidOperation])
            self.assertFalse(c.flags[InvalidOperation])
            self.assertEqual(D("0.01").max(D('0.0101'), context=xc), D('0.0'))
            self.assertEqual(D("0.01").max(D('0.0101'), context=xc), D('0.0'))
            self.assertEqual(D("0.2").max_mag(D('-0.3'), context=xc),
                             D('-0.3'))
            self.assertEqual(D("0.02").min(D('-0.03'), context=xc), D('-0.0'))
            self.assertEqual(D("0.02").min_mag(D('-0.03'), context=xc),
                             D('0.0'))
            self.assertEqual(D("0.2").next_toward(D('-1'), context=xc), D('0.1'))
            xc.clear_flags()
            self.assertRaises(InvalidOperation,
                              D("0.2").quantize, D('1e10'), context=xc)
            self.assertTrue(xc.flags[InvalidOperation])
            self.assertFalse(c.flags[InvalidOperation])
            self.assertEqual(D("9.99").remainder_near(D('1.5'), context=xc),
                             D('-0.5'))

            self.assertEqual(D("9.9").fma(third=D('0.9'), context=xc, other=7),
                             D('7E+1'))

            self.assertRaises(TypeError, D(1).is_canonical, context=xc)
            self.assertRaises(TypeError, D(1).is_finite, context=xc)
            self.assertRaises(TypeError, D(1).is_infinite, context=xc)
            self.assertRaises(TypeError, D(1).is_nan, context=xc)
            self.assertRaises(TypeError, D(1).is_qnan, context=xc)
            self.assertRaises(TypeError, D(1).is_snan, context=xc)
            self.assertRaises(TypeError, D(1).is_signed, context=xc)
            self.assertRaises(TypeError, D(1).is_zero, context=xc)

            self.assertFalse(D("0.01").is_normal(context=xc))
            self.assertTrue(D("0.01").is_subnormal(context=xc))

            self.assertRaises(TypeError, D(1).adjusted, context=xc)
            self.assertRaises(TypeError, D(1).conjugate, context=xc)
            self.assertRaises(TypeError, D(1).radix, context=xc)

            self.assertEqual(D(-111).logb(context=xc), 2)
            self.assertEqual(D(0).logical_invert(context=xc), 1)
            self.assertEqual(D('0.01').number_class(context=xc), '+Subnormal')
            self.assertEqual(D('0.21').to_eng_string(context=xc), '0.21')

            self.assertEqual(D('11').logical_and(D('10'), context=xc), 0)
            self.assertEqual(D('11').logical_or(D('10'), context=xc), 1)
            self.assertEqual(D('01').logical_xor(D('10'), context=xc), 1)
            self.assertEqual(D('23').rotate(1, context=xc), 3)
            self.assertEqual(D('23').rotate(1, context=xc), 3)
            xc.clear_flags()
            self.assertRaises(Overflow,
                              D('23').scaleb, 1, context=xc)
            self.assertTrue(xc.flags[Overflow])
            self.assertFalse(c.flags[Overflow])
            self.assertEqual(D('23').shift(-1, context=xc), 0)

            self.assertRaises(TypeError, D.from_float, 1.1, context=xc)
            self.assertRaises(TypeError, D(0).as_tuple, context=xc)

            self.assertEqual(D(1).canonical(), 1)
            self.assertRaises(TypeError, D("-1").copy_abs, context=xc)
            self.assertRaises(TypeError, D("-1").copy_negate, context=xc)
            self.assertRaises(TypeError, D(1).canonical, context="x")
            self.assertRaises(TypeError, D(1).canonical, xyz="x")

    def test_exception_hierarchy(self):

        decimal = self.decimal
        DecimalException = decimal.DecimalException
        InvalidOperation = decimal.InvalidOperation
        FloatOperation = decimal.FloatOperation
        DivisionByZero = decimal.DivisionByZero
        Overflow = decimal.Overflow
        Underflow = decimal.Underflow
        Subnormal = decimal.Subnormal
        Inexact = decimal.Inexact
        Rounded = decimal.Rounded
        Clamped = decimal.Clamped

        self.assertTrue(issubclass(DecimalException, ArithmeticError))

        self.assertTrue(issubclass(InvalidOperation, DecimalException))
        self.assertTrue(issubclass(FloatOperation, DecimalException))
        self.assertTrue(issubclass(FloatOperation, TypeError))
        self.assertTrue(issubclass(DivisionByZero, DecimalException))
        self.assertTrue(issubclass(DivisionByZero, ZeroDivisionError))
        self.assertTrue(issubclass(Overflow, Rounded))
        self.assertTrue(issubclass(Overflow, Inexact))
        self.assertTrue(issubclass(Overflow, DecimalException))
        self.assertTrue(issubclass(Underflow, Inexact))
        self.assertTrue(issubclass(Underflow, Rounded))
        self.assertTrue(issubclass(Underflow, Subnormal))
        self.assertTrue(issubclass(Underflow, DecimalException))

        self.assertTrue(issubclass(Subnormal, DecimalException))
        self.assertTrue(issubclass(Inexact, DecimalException))
        self.assertTrue(issubclass(Rounded, DecimalException))
        self.assertTrue(issubclass(Clamped, DecimalException))

        self.assertTrue(issubclass(decimal.ConversionSyntax, InvalidOperation))
        self.assertTrue(issubclass(decimal.DivisionImpossible, InvalidOperation))
        self.assertTrue(issubclass(decimal.DivisionUndefined, InvalidOperation))
        self.assertTrue(issubclass(decimal.DivisionUndefined, ZeroDivisionError))
        self.assertTrue(issubclass(decimal.InvalidContext, InvalidOperation))

class CPythonAPItests(PythonAPItests):
    decimal = C
class PyPythonAPItests(PythonAPItests):
    decimal = P

class ContextAPItests(unittest.TestCase):

    def test_none_args(self):
        Context = self.decimal.Context
        InvalidOperation = self.decimal.InvalidOperation
        DivisionByZero = self.decimal.DivisionByZero
        Overflow = self.decimal.Overflow

        c1 = Context()
        c2 = Context(prec=None, rounding=None, Emax=None, Emin=None,
                     capitals=None, clamp=None, flags=None, traps=None)
        for c in [c1, c2]:
            self.assertEqual(c.prec, 28)
            self.assertEqual(c.rounding, ROUND_HALF_EVEN)
            self.assertEqual(c.Emax, 999999)
            self.assertEqual(c.Emin, -999999)
            self.assertEqual(c.capitals, 1)
            self.assertEqual(c.clamp, 0)
            assert_signals(self, c, 'flags', [])
            assert_signals(self, c, 'traps', [InvalidOperation, DivisionByZero,
                                              Overflow])

    @cpython_only
    @requires_legacy_unicode_capi
    @warnings_helper.ignore_warnings(category=DeprecationWarning)
    def test_from_legacy_strings(self):
        import _testcapi
        c = self.decimal.Context()

        for rnd in RoundingModes:
            c.rounding = _testcapi.unicode_legacy_string(rnd)
            self.assertEqual(c.rounding, rnd)

        s = _testcapi.unicode_legacy_string('')
        self.assertRaises(TypeError, setattr, c, 'rounding', s)

        s = _testcapi.unicode_legacy_string('ROUND_\x00UP')
        self.assertRaises(TypeError, setattr, c, 'rounding', s)

    def test_pickle(self):

        for proto in range(pickle.HIGHEST_PROTOCOL + 1):
            Context = self.decimal.Context

            savedecimal = sys.modules['decimal']

            # Round trip
            sys.modules['decimal'] = self.decimal
            c = Context()
            e = pickle.loads(pickle.dumps(c, proto))

            self.assertEqual(c.prec, e.prec)
            self.assertEqual(c.Emin, e.Emin)
            self.assertEqual(c.Emax, e.Emax)
            self.assertEqual(c.rounding, e.rounding)
            self.assertEqual(c.capitals, e.capitals)
            self.assertEqual(c.clamp, e.clamp)
            self.assertEqual(c.flags, e.flags)
            self.assertEqual(c.traps, e.traps)

            # Test interchangeability
            combinations = [(C, P), (P, C)] if C else [(P, P)]
            for dumper, loader in combinations:
                for ri, _ in enumerate(RoundingModes):
                    for fi, _ in enumerate(OrderedSignals[dumper]):
                        for ti, _ in enumerate(OrderedSignals[dumper]):

                            prec = random.randrange(1, 100)
                            emin = random.randrange(-100, 0)
                            emax = random.randrange(1, 100)
                            caps = random.randrange(2)
                            clamp = random.randrange(2)

                            # One module dumps
                            sys.modules['decimal'] = dumper
                            c = dumper.Context(
                                  prec=prec, Emin=emin, Emax=emax,
                                  rounding=RoundingModes[ri],
                                  capitals=caps, clamp=clamp,
                                  flags=OrderedSignals[dumper][:fi],
                                  traps=OrderedSignals[dumper][:ti]
                            )
                            s = pickle.dumps(c, proto)

                            # The other module loads
                            sys.modules['decimal'] = loader
                            d = pickle.loads(s)
                            self.assertIsInstance(d, loader.Context)

                            self.assertEqual(d.prec, prec)
                            self.assertEqual(d.Emin, emin)
                            self.assertEqual(d.Emax, emax)
                            self.assertEqual(d.rounding, RoundingModes[ri])
                            self.assertEqual(d.capitals, caps)
                            self.assertEqual(d.clamp, clamp)
                            assert_signals(self, d, 'flags', OrderedSignals[loader][:fi])
                            assert_signals(self, d, 'traps', OrderedSignals[loader][:ti])

            sys.modules['decimal'] = savedecimal

    def test_equality_with_other_types(self):
        Decimal = self.decimal.Decimal

        self.assertIn(Decimal(10), ['a', 1.0, Decimal(10), (1,2), {}])
        self.assertNotIn(Decimal(10), ['a', 1.0, (1,2), {}])

    def test_copy(self):
        # All copies should be deep
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.copy()
        self.assertNotEqual(id(c), id(d))
        self.assertNotEqual(id(c.flags), id(d.flags))
        self.assertNotEqual(id(c.traps), id(d.traps))
        k1 = set(c.flags.keys())
        k2 = set(d.flags.keys())
        self.assertEqual(k1, k2)
        self.assertEqual(c.flags, d.flags)

    def test__clamp(self):
        # In Python 3.2, the private attribute `_clamp` was made
        # public (issue 8540), with the old `_clamp` becoming a
        # property wrapping `clamp`.  For the duration of Python 3.2
        # only, the attribute should be gettable/settable via both
        # `clamp` and `_clamp`; in Python 3.3, `_clamp` should be
        # removed.
        Context = self.decimal.Context
        c = Context()
        self.assertRaises(AttributeError, getattr, c, '_clamp')

    def test_abs(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.abs(Decimal(-1))
        self.assertEqual(c.abs(-1), d)
        self.assertRaises(TypeError, c.abs, '-1')

    def test_add(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.add(Decimal(1), Decimal(1))
        self.assertEqual(c.add(1, 1), d)
        self.assertEqual(c.add(Decimal(1), 1), d)
        self.assertEqual(c.add(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.add, '1', 1)
        self.assertRaises(TypeError, c.add, 1, '1')

    def test_compare(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.compare(Decimal(1), Decimal(1))
        self.assertEqual(c.compare(1, 1), d)
        self.assertEqual(c.compare(Decimal(1), 1), d)
        self.assertEqual(c.compare(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.compare, '1', 1)
        self.assertRaises(TypeError, c.compare, 1, '1')

    def test_compare_signal(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.compare_signal(Decimal(1), Decimal(1))
        self.assertEqual(c.compare_signal(1, 1), d)
        self.assertEqual(c.compare_signal(Decimal(1), 1), d)
        self.assertEqual(c.compare_signal(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.compare_signal, '1', 1)
        self.assertRaises(TypeError, c.compare_signal, 1, '1')

    def test_compare_total(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.compare_total(Decimal(1), Decimal(1))
        self.assertEqual(c.compare_total(1, 1), d)
        self.assertEqual(c.compare_total(Decimal(1), 1), d)
        self.assertEqual(c.compare_total(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.compare_total, '1', 1)
        self.assertRaises(TypeError, c.compare_total, 1, '1')

    def test_compare_total_mag(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.compare_total_mag(Decimal(1), Decimal(1))
        self.assertEqual(c.compare_total_mag(1, 1), d)
        self.assertEqual(c.compare_total_mag(Decimal(1), 1), d)
        self.assertEqual(c.compare_total_mag(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.compare_total_mag, '1', 1)
        self.assertRaises(TypeError, c.compare_total_mag, 1, '1')

    def test_copy_abs(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.copy_abs(Decimal(-1))
        self.assertEqual(c.copy_abs(-1), d)
        self.assertRaises(TypeError, c.copy_abs, '-1')

    def test_copy_decimal(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.copy_decimal(Decimal(-1))
        self.assertEqual(c.copy_decimal(-1), d)
        self.assertRaises(TypeError, c.copy_decimal, '-1')

    def test_copy_negate(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.copy_negate(Decimal(-1))
        self.assertEqual(c.copy_negate(-1), d)
        self.assertRaises(TypeError, c.copy_negate, '-1')

    def test_copy_sign(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.copy_sign(Decimal(1), Decimal(-2))
        self.assertEqual(c.copy_sign(1, -2), d)
        self.assertEqual(c.copy_sign(Decimal(1), -2), d)
        self.assertEqual(c.copy_sign(1, Decimal(-2)), d)
        self.assertRaises(TypeError, c.copy_sign, '1', -2)
        self.assertRaises(TypeError, c.copy_sign, 1, '-2')

    def test_divide(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.divide(Decimal(1), Decimal(2))
        self.assertEqual(c.divide(1, 2), d)
        self.assertEqual(c.divide(Decimal(1), 2), d)
        self.assertEqual(c.divide(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.divide, '1', 2)
        self.assertRaises(TypeError, c.divide, 1, '2')

    def test_divide_int(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.divide_int(Decimal(1), Decimal(2))
        self.assertEqual(c.divide_int(1, 2), d)
        self.assertEqual(c.divide_int(Decimal(1), 2), d)
        self.assertEqual(c.divide_int(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.divide_int, '1', 2)
        self.assertRaises(TypeError, c.divide_int, 1, '2')

    def test_divmod(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.divmod(Decimal(1), Decimal(2))
        self.assertEqual(c.divmod(1, 2), d)
        self.assertEqual(c.divmod(Decimal(1), 2), d)
        self.assertEqual(c.divmod(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.divmod, '1', 2)
        self.assertRaises(TypeError, c.divmod, 1, '2')

    def test_exp(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.exp(Decimal(10))
        self.assertEqual(c.exp(10), d)
        self.assertRaises(TypeError, c.exp, '10')

    def test_fma(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.fma(Decimal(2), Decimal(3), Decimal(4))
        self.assertEqual(c.fma(2, 3, 4), d)
        self.assertEqual(c.fma(Decimal(2), 3, 4), d)
        self.assertEqual(c.fma(2, Decimal(3), 4), d)
        self.assertEqual(c.fma(2, 3, Decimal(4)), d)
        self.assertEqual(c.fma(Decimal(2), Decimal(3), 4), d)
        self.assertRaises(TypeError, c.fma, '2', 3, 4)
        self.assertRaises(TypeError, c.fma, 2, '3', 4)
        self.assertRaises(TypeError, c.fma, 2, 3, '4')

        # Issue 12079 for Context.fma ...
        self.assertRaises(TypeError, c.fma,
                          Decimal('Infinity'), Decimal(0), "not a decimal")
        self.assertRaises(TypeError, c.fma,
                          Decimal(1), Decimal('snan'), 1.222)
        # ... and for Decimal.fma.
        self.assertRaises(TypeError, Decimal('Infinity').fma,
                          Decimal(0), "not a decimal")
        self.assertRaises(TypeError, Decimal(1).fma,
                          Decimal('snan'), 1.222)

    def test_is_finite(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_finite(Decimal(10))
        self.assertEqual(c.is_finite(10), d)
        self.assertRaises(TypeError, c.is_finite, '10')

    def test_is_infinite(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_infinite(Decimal(10))
        self.assertEqual(c.is_infinite(10), d)
        self.assertRaises(TypeError, c.is_infinite, '10')

    def test_is_nan(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_nan(Decimal(10))
        self.assertEqual(c.is_nan(10), d)
        self.assertRaises(TypeError, c.is_nan, '10')

    def test_is_normal(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_normal(Decimal(10))
        self.assertEqual(c.is_normal(10), d)
        self.assertRaises(TypeError, c.is_normal, '10')

    def test_is_qnan(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_qnan(Decimal(10))
        self.assertEqual(c.is_qnan(10), d)
        self.assertRaises(TypeError, c.is_qnan, '10')

    def test_is_signed(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_signed(Decimal(10))
        self.assertEqual(c.is_signed(10), d)
        self.assertRaises(TypeError, c.is_signed, '10')

    def test_is_snan(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_snan(Decimal(10))
        self.assertEqual(c.is_snan(10), d)
        self.assertRaises(TypeError, c.is_snan, '10')

    def test_is_subnormal(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_subnormal(Decimal(10))
        self.assertEqual(c.is_subnormal(10), d)
        self.assertRaises(TypeError, c.is_subnormal, '10')

    def test_is_zero(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.is_zero(Decimal(10))
        self.assertEqual(c.is_zero(10), d)
        self.assertRaises(TypeError, c.is_zero, '10')

    def test_ln(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.ln(Decimal(10))
        self.assertEqual(c.ln(10), d)
        self.assertRaises(TypeError, c.ln, '10')

    def test_log10(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.log10(Decimal(10))
        self.assertEqual(c.log10(10), d)
        self.assertRaises(TypeError, c.log10, '10')

    def test_logb(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.logb(Decimal(10))
        self.assertEqual(c.logb(10), d)
        self.assertRaises(TypeError, c.logb, '10')

    def test_logical_and(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.logical_and(Decimal(1), Decimal(1))
        self.assertEqual(c.logical_and(1, 1), d)
        self.assertEqual(c.logical_and(Decimal(1), 1), d)
        self.assertEqual(c.logical_and(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.logical_and, '1', 1)
        self.assertRaises(TypeError, c.logical_and, 1, '1')

    def test_logical_invert(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.logical_invert(Decimal(1000))
        self.assertEqual(c.logical_invert(1000), d)
        self.assertRaises(TypeError, c.logical_invert, '1000')

    def test_logical_or(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.logical_or(Decimal(1), Decimal(1))
        self.assertEqual(c.logical_or(1, 1), d)
        self.assertEqual(c.logical_or(Decimal(1), 1), d)
        self.assertEqual(c.logical_or(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.logical_or, '1', 1)
        self.assertRaises(TypeError, c.logical_or, 1, '1')

    def test_logical_xor(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.logical_xor(Decimal(1), Decimal(1))
        self.assertEqual(c.logical_xor(1, 1), d)
        self.assertEqual(c.logical_xor(Decimal(1), 1), d)
        self.assertEqual(c.logical_xor(1, Decimal(1)), d)
        self.assertRaises(TypeError, c.logical_xor, '1', 1)
        self.assertRaises(TypeError, c.logical_xor, 1, '1')

    def test_max(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.max(Decimal(1), Decimal(2))
        self.assertEqual(c.max(1, 2), d)
        self.assertEqual(c.max(Decimal(1), 2), d)
        self.assertEqual(c.max(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.max, '1', 2)
        self.assertRaises(TypeError, c.max, 1, '2')

    def test_max_mag(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.max_mag(Decimal(1), Decimal(2))
        self.assertEqual(c.max_mag(1, 2), d)
        self.assertEqual(c.max_mag(Decimal(1), 2), d)
        self.assertEqual(c.max_mag(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.max_mag, '1', 2)
        self.assertRaises(TypeError, c.max_mag, 1, '2')

    def test_min(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.min(Decimal(1), Decimal(2))
        self.assertEqual(c.min(1, 2), d)
        self.assertEqual(c.min(Decimal(1), 2), d)
        self.assertEqual(c.min(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.min, '1', 2)
        self.assertRaises(TypeError, c.min, 1, '2')

    def test_min_mag(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.min_mag(Decimal(1), Decimal(2))
        self.assertEqual(c.min_mag(1, 2), d)
        self.assertEqual(c.min_mag(Decimal(1), 2), d)
        self.assertEqual(c.min_mag(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.min_mag, '1', 2)
        self.assertRaises(TypeError, c.min_mag, 1, '2')

    def test_minus(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.minus(Decimal(10))
        self.assertEqual(c.minus(10), d)
        self.assertRaises(TypeError, c.minus, '10')

    def test_multiply(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.multiply(Decimal(1), Decimal(2))
        self.assertEqual(c.multiply(1, 2), d)
        self.assertEqual(c.multiply(Decimal(1), 2), d)
        self.assertEqual(c.multiply(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.multiply, '1', 2)
        self.assertRaises(TypeError, c.multiply, 1, '2')

    def test_next_minus(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.next_minus(Decimal(10))
        self.assertEqual(c.next_minus(10), d)
        self.assertRaises(TypeError, c.next_minus, '10')

    def test_next_plus(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.next_plus(Decimal(10))
        self.assertEqual(c.next_plus(10), d)
        self.assertRaises(TypeError, c.next_plus, '10')

    def test_next_toward(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.next_toward(Decimal(1), Decimal(2))
        self.assertEqual(c.next_toward(1, 2), d)
        self.assertEqual(c.next_toward(Decimal(1), 2), d)
        self.assertEqual(c.next_toward(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.next_toward, '1', 2)
        self.assertRaises(TypeError, c.next_toward, 1, '2')

    def test_normalize(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.normalize(Decimal(10))
        self.assertEqual(c.normalize(10), d)
        self.assertRaises(TypeError, c.normalize, '10')

    def test_number_class(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        self.assertEqual(c.number_class(123), c.number_class(Decimal(123)))
        self.assertEqual(c.number_class(0), c.number_class(Decimal(0)))
        self.assertEqual(c.number_class(-45), c.number_class(Decimal(-45)))

    def test_plus(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.plus(Decimal(10))
        self.assertEqual(c.plus(10), d)
        self.assertRaises(TypeError, c.plus, '10')

    def test_power(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.power(Decimal(1), Decimal(4))
        self.assertEqual(c.power(1, 4), d)
        self.assertEqual(c.power(Decimal(1), 4), d)
        self.assertEqual(c.power(1, Decimal(4)), d)
        self.assertEqual(c.power(Decimal(1), Decimal(4)), d)
        self.assertRaises(TypeError, c.power, '1', 4)
        self.assertRaises(TypeError, c.power, 1, '4')
        self.assertEqual(c.power(modulo=5, b=8, a=2), 1)

    def test_quantize(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.quantize(Decimal(1), Decimal(2))
        self.assertEqual(c.quantize(1, 2), d)
        self.assertEqual(c.quantize(Decimal(1), 2), d)
        self.assertEqual(c.quantize(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.quantize, '1', 2)
        self.assertRaises(TypeError, c.quantize, 1, '2')

    def test_remainder(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.remainder(Decimal(1), Decimal(2))
        self.assertEqual(c.remainder(1, 2), d)
        self.assertEqual(c.remainder(Decimal(1), 2), d)
        self.assertEqual(c.remainder(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.remainder, '1', 2)
        self.assertRaises(TypeError, c.remainder, 1, '2')

    def test_remainder_near(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.remainder_near(Decimal(1), Decimal(2))
        self.assertEqual(c.remainder_near(1, 2), d)
        self.assertEqual(c.remainder_near(Decimal(1), 2), d)
        self.assertEqual(c.remainder_near(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.remainder_near, '1', 2)
        self.assertRaises(TypeError, c.remainder_near, 1, '2')

    def test_rotate(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.rotate(Decimal(1), Decimal(2))
        self.assertEqual(c.rotate(1, 2), d)
        self.assertEqual(c.rotate(Decimal(1), 2), d)
        self.assertEqual(c.rotate(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.rotate, '1', 2)
        self.assertRaises(TypeError, c.rotate, 1, '2')

    def test_sqrt(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.sqrt(Decimal(10))
        self.assertEqual(c.sqrt(10), d)
        self.assertRaises(TypeError, c.sqrt, '10')

    def test_same_quantum(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.same_quantum(Decimal(1), Decimal(2))
        self.assertEqual(c.same_quantum(1, 2), d)
        self.assertEqual(c.same_quantum(Decimal(1), 2), d)
        self.assertEqual(c.same_quantum(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.same_quantum, '1', 2)
        self.assertRaises(TypeError, c.same_quantum, 1, '2')

    def test_scaleb(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.scaleb(Decimal(1), Decimal(2))
        self.assertEqual(c.scaleb(1, 2), d)
        self.assertEqual(c.scaleb(Decimal(1), 2), d)
        self.assertEqual(c.scaleb(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.scaleb, '1', 2)
        self.assertRaises(TypeError, c.scaleb, 1, '2')

    def test_shift(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.shift(Decimal(1), Decimal(2))
        self.assertEqual(c.shift(1, 2), d)
        self.assertEqual(c.shift(Decimal(1), 2), d)
        self.assertEqual(c.shift(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.shift, '1', 2)
        self.assertRaises(TypeError, c.shift, 1, '2')

    def test_subtract(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.subtract(Decimal(1), Decimal(2))
        self.assertEqual(c.subtract(1, 2), d)
        self.assertEqual(c.subtract(Decimal(1), 2), d)
        self.assertEqual(c.subtract(1, Decimal(2)), d)
        self.assertRaises(TypeError, c.subtract, '1', 2)
        self.assertRaises(TypeError, c.subtract, 1, '2')

    def test_to_eng_string(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.to_eng_string(Decimal(10))
        self.assertEqual(c.to_eng_string(10), d)
        self.assertRaises(TypeError, c.to_eng_string, '10')

    def test_to_sci_string(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.to_sci_string(Decimal(10))
        self.assertEqual(c.to_sci_string(10), d)
        self.assertRaises(TypeError, c.to_sci_string, '10')

    def test_to_integral_exact(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.to_integral_exact(Decimal(10))
        self.assertEqual(c.to_integral_exact(10), d)
        self.assertRaises(TypeError, c.to_integral_exact, '10')

    def test_to_integral_value(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context

        c = Context()
        d = c.to_integral_value(Decimal(10))
        self.assertEqual(c.to_integral_value(10), d)
        self.assertRaises(TypeError, c.to_integral_value, '10')
        self.assertRaises(TypeError, c.to_integral_value, 10, 'x')

class CContextAPItests(ContextAPItests):
    decimal = C
class PyContextAPItests(ContextAPItests):
    decimal = P

class ContextWithStatement(unittest.TestCase):
    # Can't do these as docstrings until Python 2.6
    # as doctest can't handle __future__ statements

    def test_localcontext(self):
        # Use a copy of the current context in the block
        getcontext = self.decimal.getcontext
        localcontext = self.decimal.localcontext

        orig_ctx = getcontext()
        with localcontext() as enter_ctx:
            set_ctx = getcontext()
        final_ctx = getcontext()
        self.assertIs(orig_ctx, final_ctx, 'did not restore context correctly')
        self.assertIsNot(orig_ctx, set_ctx, 'did not copy the context')
        self.assertIs(set_ctx, enter_ctx, '__enter__ returned wrong context')

    def test_localcontextarg(self):
        # Use a copy of the supplied context in the block
        Context = self.decimal.Context
        getcontext = self.decimal.getcontext
        localcontext = self.decimal.localcontext

        localcontext = self.decimal.localcontext
        orig_ctx = getcontext()
        new_ctx = Context(prec=42)
        with localcontext(new_ctx) as enter_ctx:
            set_ctx = getcontext()
        final_ctx = getcontext()
        self.assertIs(orig_ctx, final_ctx, 'did not restore context correctly')
        self.assertEqual(set_ctx.prec, new_ctx.prec, 'did not set correct context')
        self.assertIsNot(new_ctx, set_ctx, 'did not copy the context')
        self.assertIs(set_ctx, enter_ctx, '__enter__ returned wrong context')

    def test_localcontext_kwargs(self):
        with self.decimal.localcontext(
            prec=10, rounding=ROUND_HALF_DOWN,
            Emin=-20, Emax=20, capitals=0,
            clamp=1
        ) as ctx:
            self.assertEqual(ctx.prec, 10)
            self.assertEqual(ctx.rounding, self.decimal.ROUND_HALF_DOWN)
            self.assertEqual(ctx.Emin, -20)
            self.assertEqual(ctx.Emax, 20)
            self.assertEqual(ctx.capitals, 0)
            self.assertEqual(ctx.clamp, 1)

        self.assertRaises(TypeError, self.decimal.localcontext, precision=10)

        self.assertRaises(ValueError, self.decimal.localcontext, Emin=1)
        self.assertRaises(ValueError, self.decimal.localcontext, Emax=-1)
        self.assertRaises(ValueError, self.decimal.localcontext, capitals=2)
        self.assertRaises(ValueError, self.decimal.localcontext, clamp=2)

        self.assertRaises(TypeError, self.decimal.localcontext, rounding="")
        self.assertRaises(TypeError, self.decimal.localcontext, rounding=1)

        self.assertRaises(TypeError, self.decimal.localcontext, flags="")
        self.assertRaises(TypeError, self.decimal.localcontext, traps="")
        self.assertRaises(TypeError, self.decimal.localcontext, Emin="")
        self.assertRaises(TypeError, self.decimal.localcontext, Emax="")

    def test_local_context_kwargs_does_not_overwrite_existing_argument(self):
        ctx = self.decimal.getcontext()
        ctx.prec = 28
        with self.decimal.localcontext(prec=10) as ctx2:
            self.assertEqual(ctx.prec, 28)

    def test_nested_with_statements(self):
        # Use a copy of the supplied context in the block
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        getcontext = self.decimal.getcontext
        localcontext = self.decimal.localcontext
        Clamped = self.decimal.Clamped
        Overflow = self.decimal.Overflow

        orig_ctx = getcontext()
        orig_ctx.clear_flags()
        new_ctx = Context(Emax=384)
        with localcontext() as c1:
            self.assertEqual(c1.flags, orig_ctx.flags)
            self.assertEqual(c1.traps, orig_ctx.traps)
            c1.traps[Clamped] = True
            c1.Emin = -383
            self.assertNotEqual(orig_ctx.Emin, -383)
            self.assertRaises(Clamped, c1.create_decimal, '0e-999')
            self.assertTrue(c1.flags[Clamped])
            with localcontext(new_ctx) as c2:
                self.assertEqual(c2.flags, new_ctx.flags)
                self.assertEqual(c2.traps, new_ctx.traps)
                self.assertRaises(Overflow, c2.power, Decimal('3.4e200'), 2)
                self.assertFalse(c2.flags[Clamped])
                self.assertTrue(c2.flags[Overflow])
                del c2
            self.assertFalse(c1.flags[Overflow])
            del c1
        self.assertNotEqual(orig_ctx.Emin, -383)
        self.assertFalse(orig_ctx.flags[Clamped])
        self.assertFalse(orig_ctx.flags[Overflow])
        self.assertFalse(new_ctx.flags[Clamped])
        self.assertFalse(new_ctx.flags[Overflow])

    def test_with_statements_gc1(self):
        localcontext = self.decimal.localcontext

        with localcontext() as c1:
            del c1
            with localcontext() as c2:
                del c2
                with localcontext() as c3:
                    del c3
                    with localcontext() as c4:
                        del c4

    def test_with_statements_gc2(self):
        localcontext = self.decimal.localcontext

        with localcontext() as c1:
            with localcontext(c1) as c2:
                del c1
                with localcontext(c2) as c3:
                    del c2
                    with localcontext(c3) as c4:
                        del c3
                        del c4

    def test_with_statements_gc3(self):
        Context = self.decimal.Context
        localcontext = self.decimal.localcontext
        getcontext = self.decimal.getcontext
        setcontext = self.decimal.setcontext

        with localcontext() as c1:
            del c1
            n1 = Context(prec=1)
            setcontext(n1)
            with localcontext(n1) as c2:
                del n1
                self.assertEqual(c2.prec, 1)
                del c2
                n2 = Context(prec=2)
                setcontext(n2)
                del n2
                self.assertEqual(getcontext().prec, 2)
                n3 = Context(prec=3)
                setcontext(n3)
                self.assertEqual(getcontext().prec, 3)
                with localcontext(n3) as c3:
                    del n3
                    self.assertEqual(c3.prec, 3)
                    del c3
                    n4 = Context(prec=4)
                    setcontext(n4)
                    del n4
                    self.assertEqual(getcontext().prec, 4)
                    with localcontext() as c4:
                        self.assertEqual(c4.prec, 4)
                        del c4

class CContextWithStatement(ContextWithStatement):
    decimal = C
class PyContextWithStatement(ContextWithStatement):
    decimal = P

class ContextFlags(unittest.TestCase):

    def test_flags_irrelevant(self):
        # check that the result (numeric result + flags raised) of an
        # arithmetic operation doesn't depend on the current flags
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        Inexact = self.decimal.Inexact
        Rounded = self.decimal.Rounded
        Underflow = self.decimal.Underflow
        Clamped = self.decimal.Clamped
        Subnormal = self.decimal.Subnormal

        def raise_error(context, flag):
            if self.decimal == C:
                context.flags[flag] = True
                if context.traps[flag]:
                    raise flag
            else:
                context._raise_error(flag)

        context = Context(prec=9, Emin = -425000000, Emax = 425000000,
                          rounding=ROUND_HALF_EVEN, traps=[], flags=[])

        # operations that raise various flags, in the form (function, arglist)
        operations = [
            (context._apply, [Decimal("100E-425000010")]),
            (context.sqrt, [Decimal(2)]),
            (context.add, [Decimal("1.23456789"), Decimal("9.87654321")]),
            (context.multiply, [Decimal("1.23456789"), Decimal("9.87654321")]),
            (context.subtract, [Decimal("1.23456789"), Decimal("9.87654321")]),
            ]

        # try various flags individually, then a whole lot at once
        flagsets = [[Inexact], [Rounded], [Underflow], [Clamped], [Subnormal],
                    [Inexact, Rounded, Underflow, Clamped, Subnormal]]

        for fn, args in operations:
            # find answer and flags raised using a clean context
            context.clear_flags()
            ans = fn(*args)
            flags = [k for k, v in context.flags.items() if v]

            for extra_flags in flagsets:
                # set flags, before calling operation
                context.clear_flags()
                for flag in extra_flags:
                    raise_error(context, flag)
                new_ans = fn(*args)

                # flags that we expect to be set after the operation
                expected_flags = list(flags)
                for flag in extra_flags:
                    if flag not in expected_flags:
                        expected_flags.append(flag)
                expected_flags.sort(key=id)

                # flags we actually got
                new_flags = [k for k,v in context.flags.items() if v]
                new_flags.sort(key=id)

                self.assertEqual(ans, new_ans,
                                 "operation produces different answers depending on flags set: " +
                                 "expected %s, got %s." % (ans, new_ans))
                self.assertEqual(new_flags, expected_flags,
                                  "operation raises different flags depending on flags set: " +
                                  "expected %s, got %s" % (expected_flags, new_flags))

    def test_flag_comparisons(self):
        Context = self.decimal.Context
        Inexact = self.decimal.Inexact
        Rounded = self.decimal.Rounded

        c = Context()

        # Valid SignalDict
        self.assertNotEqual(c.flags, c.traps)
        self.assertNotEqual(c.traps, c.flags)

        c.flags = c.traps
        self.assertEqual(c.flags, c.traps)
        self.assertEqual(c.traps, c.flags)

        c.flags[Rounded] = True
        c.traps = c.flags
        self.assertEqual(c.flags, c.traps)
        self.assertEqual(c.traps, c.flags)

        d = {}
        d.update(c.flags)
        self.assertEqual(d, c.flags)
        self.assertEqual(c.flags, d)

        d[Inexact] = True
        self.assertNotEqual(d, c.flags)
        self.assertNotEqual(c.flags, d)

        # Invalid SignalDict
        d = {Inexact:False}
        self.assertNotEqual(d, c.flags)
        self.assertNotEqual(c.flags, d)

        d = ["xyz"]
        self.assertNotEqual(d, c.flags)
        self.assertNotEqual(c.flags, d)

    @requires_IEEE_754
    def test_float_operation(self):
        Decimal = self.decimal.Decimal
        FloatOperation = self.decimal.FloatOperation
        localcontext = self.decimal.localcontext

        with localcontext() as c:
            ##### trap is off by default
            self.assertFalse(c.traps[FloatOperation])

            # implicit conversion sets the flag
            c.clear_flags()
            self.assertEqual(Decimal(7.5), 7.5)
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            self.assertEqual(c.create_decimal(7.5), 7.5)
            self.assertTrue(c.flags[FloatOperation])

            # explicit conversion does not set the flag
            c.clear_flags()
            x = Decimal.from_float(7.5)
            self.assertFalse(c.flags[FloatOperation])
            # comparison sets the flag
            self.assertEqual(x, 7.5)
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            x = c.create_decimal_from_float(7.5)
            self.assertFalse(c.flags[FloatOperation])
            self.assertEqual(x, 7.5)
            self.assertTrue(c.flags[FloatOperation])

            ##### set the trap
            c.traps[FloatOperation] = True

            # implicit conversion raises
            c.clear_flags()
            self.assertRaises(FloatOperation, Decimal, 7.5)
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            self.assertRaises(FloatOperation, c.create_decimal, 7.5)
            self.assertTrue(c.flags[FloatOperation])

            # explicit conversion is silent
            c.clear_flags()
            x = Decimal.from_float(7.5)
            self.assertFalse(c.flags[FloatOperation])

            c.clear_flags()
            x = c.create_decimal_from_float(7.5)
            self.assertFalse(c.flags[FloatOperation])

    def test_float_comparison(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        FloatOperation = self.decimal.FloatOperation
        localcontext = self.decimal.localcontext

        def assert_attr(a, b, attr, context, signal=None):
            context.clear_flags()
            f = getattr(a, attr)
            if signal == FloatOperation:
                self.assertRaises(signal, f, b)
            else:
                self.assertIs(f(b), True)
            self.assertTrue(context.flags[FloatOperation])

        small_d = Decimal('0.25')
        big_d = Decimal('3.0')
        small_f = 0.25
        big_f = 3.0

        zero_d = Decimal('0.0')
        neg_zero_d = Decimal('-0.0')
        zero_f = 0.0
        neg_zero_f = -0.0

        inf_d = Decimal('Infinity')
        neg_inf_d = Decimal('-Infinity')
        inf_f = float('inf')
        neg_inf_f = float('-inf')

        def doit(c, signal=None):
            # Order
            for attr in '__lt__', '__le__':
                assert_attr(small_d, big_f, attr, c, signal)

            for attr in '__gt__', '__ge__':
                assert_attr(big_d, small_f, attr, c, signal)

            # Equality
            assert_attr(small_d, small_f, '__eq__', c, None)

            assert_attr(neg_zero_d, neg_zero_f, '__eq__', c, None)
            assert_attr(neg_zero_d, zero_f, '__eq__', c, None)

            assert_attr(zero_d, neg_zero_f, '__eq__', c, None)
            assert_attr(zero_d, zero_f, '__eq__', c, None)

            assert_attr(neg_inf_d, neg_inf_f, '__eq__', c, None)
            assert_attr(inf_d, inf_f, '__eq__', c, None)

            # Inequality
            assert_attr(small_d, big_f, '__ne__', c, None)

            assert_attr(Decimal('0.1'), 0.1, '__ne__', c, None)

            assert_attr(neg_inf_d, inf_f, '__ne__', c, None)
            assert_attr(inf_d, neg_inf_f, '__ne__', c, None)

            assert_attr(Decimal('NaN'), float('nan'), '__ne__', c, None)

        def test_containers(c, signal=None):
            c.clear_flags()
            s = set([100.0, Decimal('100.0')])
            self.assertEqual(len(s), 1)
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            if signal:
                self.assertRaises(signal, sorted, [1.0, Decimal('10.0')])
            else:
                s = sorted([10.0, Decimal('10.0')])
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            b = 10.0 in [Decimal('10.0'), 1.0]
            self.assertTrue(c.flags[FloatOperation])

            c.clear_flags()
            b = 10.0 in {Decimal('10.0'):'a', 1.0:'b'}
            self.assertTrue(c.flags[FloatOperation])

        nc = Context()
        with localcontext(nc) as c:
            self.assertFalse(c.traps[FloatOperation])
            doit(c, signal=None)
            test_containers(c, signal=None)

            c.traps[FloatOperation] = True
            doit(c, signal=FloatOperation)
            test_containers(c, signal=FloatOperation)

    def test_float_operation_default(self):
        Decimal = self.decimal.Decimal
        Context = self.decimal.Context
        Inexact = self.decimal.Inexact
        FloatOperation= self.decimal.FloatOperation

        context = Context()
        self.assertFalse(context.flags[FloatOperation])
        self.assertFalse(context.traps[FloatOperation])

        context.clear_traps()
        context.traps[Inexact] = True
        context.traps[FloatOperation] = True
        self.assertTrue(context.traps[FloatOperation])
        self.assertTrue(context.traps[Inexact])

class CContextFlags(ContextFlags):
    decimal = C
class PyContextFlags(ContextFlags):
    decimal = P

class SpecialContexts(unittest.TestCase):
    """Test the context templates."""

    def test_context_templates(self):
        BasicContext = self.decimal.BasicContext
        ExtendedContext = self.decimal.ExtendedContext
        getcontext = self.decimal.getcontext
        setcontext = self.decimal.setcontext
        InvalidOperation = self.decimal.InvalidOperation
        DivisionByZero = self.decimal.DivisionByZero
        Overflow = self.decimal.Overflow
        Underflow = self.decimal.Underflow
        Clamped = self.decimal.Clamped

        assert_signals(self, BasicContext, 'traps',
            [InvalidOperation, DivisionByZero, Overflow, Underflow, Clamped]
        )

        savecontext = getcontext().copy()
        basic_context_prec = BasicContext.prec
        extended_context_prec = ExtendedContext.prec

        ex = None
        try:
            BasicContext.prec = ExtendedContext.prec = 441
            for template in BasicContext, ExtendedContext:
                setcontext(template)
                c = getcontext()
                self.assertIsNot(c, template)
                self.assertEqual(c.prec, 441)
        except Exception as e:
            ex = e.__class__
        finally:
            BasicContext.prec = basic_context_prec
            ExtendedContext.prec = extended_context_prec
            setcontext(savecontext)
            if ex:
                raise ex

    def test_default_context(self):
        DefaultContext = self.decimal.DefaultContext
        BasicContext = self.decimal.BasicContext
        ExtendedContext = self.decimal.ExtendedContext
        getcontext = self.decimal.getcontext
        setcontext = self.decimal.setcontext
        InvalidOperation = self.decimal.InvalidOperation
        DivisionByZero = self.decimal.DivisionByZero
        Overflow = self.decimal.Overflow

        self.assertEqual(BasicContext.prec, 9)
        self.assertEqual(ExtendedContext.prec, 9)

        assert_signals(self, DefaultContext, 'traps',
            [InvalidOperation, DivisionByZero, Overflow]
        )

        savecontext = getcontext().copy()
        default_context_prec = DefaultContext.prec

        ex = None
        try:
            c = getcontext()
            saveprec = c.prec

            DefaultContext.prec = 961
            c = getcontext()
            self.assertEqual(c.prec, saveprec)

            setcontext(DefaultContext)
            c = getcontext()
            self.assertIsNot(c, DefaultContext)
            self.assertEqual(c.prec, 961)
        except Exception as e:
            ex = e.__class__
        finally:
            DefaultContext.prec = default_context_prec
            setcontext(savecontext)
            if ex:
                raise ex

class CSpecialContexts(SpecialContexts):
    decimal = C
class PySpecialContexts(SpecialContexts):
    decimal = P

class ContextInputValidation(unittest.TestCase):

    def test_invalid_context(self):
        Context = self.decimal.Context
        DefaultContext = self.decimal.DefaultContext

        c = DefaultContext.copy()

        # prec, Emax
        for attr in ['prec', 'Emax']:
            setattr(c, attr, 999999)
            self.assertEqual(getattr(c, attr), 999999)
            self.assertRaises(ValueError, setattr, c, attr, -1)
            self.assertRaises(TypeError, setattr, c, attr, 'xyz')

        # Emin
        setattr(c, 'Emin', -999999)
        self.assertEqual(getattr(c, 'Emin'), -999999)
        self.assertRaises(ValueError, setattr, c, 'Emin', 1)
        self.assertRaises(TypeError, setattr, c, 'Emin', (1,2,3))

        self.assertRaises(TypeError, setattr, c, 'rounding', -1)
        self.assertRaises(TypeError, setattr, c, 'rounding', 9)
        self.assertRaises(TypeError, setattr, c, 'rounding', 1.0)
        self.assertRaises(TypeError, setattr, c, 'rounding', 'xyz')

        # capitals, clamp
        for attr in ['capitals', 'clamp']:
            self.assertRaises(ValueError, setattr, c, attr, -1)
            self.assertRaises(ValueError, setattr, c, attr, 2)
            self.assertRaises(TypeError, setattr, c, attr, [1,2,3])

        # Invalid attribute
        self.assertRaises(AttributeError, setattr, c, 'emax', 100)

        # Invalid signal dict
        self.assertRaises(TypeError, setattr, c, 'flags', [])
        self.assertRaises(KeyError, setattr, c, 'flags', {})
        self.assertRaises(KeyError, setattr, c, 'traps',
                          {'InvalidOperation':0})

        # Attributes cannot be deleted
        for attr in ['prec', 'Emax', 'Emin', 'rounding', 'capitals', 'clamp',
                     'flags', 'traps']:
            self.assertRaises(AttributeError, c.__delattr__, attr)

        # Invalid attributes
        self.assertRaises(TypeError, getattr, c, 9)
        self.assertRaises(TypeError, setattr, c, 9)

        # Invalid values in constructor
        self.assertRaises(TypeError, Context, rounding=999999)
        self.assertRaises(TypeError, Context, rounding='xyz')
        self.assertRaises(ValueError, Context, clamp=2)
        self.assertRaises(ValueError, Context, capitals=-1)
        self.assertRaises(KeyError, Context, flags=["P"])
        self.assertRaises(KeyError, Context, traps=["Q"])

        # Type error in conversion
        self.assertRaises(TypeError, Context, flags=(0,1))
        self.assertRaises(TypeError, Context, traps=(1,0))

class CContextInputValidation(ContextInputValidation):
    decimal = C
class PyContextInputValidation(ContextInputValidation):
    decimal = P

class ContextSubclassing(unittest.TestCase):

    def test_context_subclassing(self):
        decimal = self.decimal
        Decimal = decimal.Decimal
        Context = decimal.Context
        Clamped = decimal.Clamped
        DivisionByZero = decimal.DivisionByZero
        Inexact = decimal.Inexact
        Overflow = decimal.Overflow
        Rounded = decimal.Rounded
        Subnormal = decimal.Subnormal
        Underflow = decimal.Underflow
        InvalidOperation = decimal.InvalidOperation

        class MyContext(Context):
            def __init__(self, prec=None, rounding=None, Emin=None, Emax=None,
                               capitals=None, clamp=None, flags=None,
                               traps=None):
                Context.__init__(self)
                if prec is not None:
                    self.prec = prec
                if rounding is not None:
                    self.rounding = rounding
                if Emin is not None:
                    self.Emin = Emin
                if Emax is not None:
                    self.Emax = Emax
                if capitals is not None:
                    self.capitals = capitals
                if clamp is not None:
                    self.clamp = clamp
                if flags is not None:
                    if isinstance(flags, list):
                        flags = {v:(v in flags) for v in OrderedSignals[decimal] + flags}
                    self.flags = flags
                if traps is not None:
                    if isinstance(traps, list):
                        traps = {v:(v in traps) for v in OrderedSignals[decimal] + traps}
                    self.traps = traps

        c = Context()
        d = MyContext()
        for attr in ('prec', 'rounding', 'Emin', 'Emax', 'capitals', 'clamp',
                     'flags', 'traps'):
            self.assertEqual(getattr(c, attr), getattr(d, attr))

        # prec
        self.assertRaises(ValueError, MyContext, **{'prec':-1})
        c = MyContext(prec=1)
        self.assertEqual(c.prec, 1)
        self.assertRaises(InvalidOperation, c.quantize, Decimal('9e2'), 0)

        # rounding
        self.assertRaises(TypeError, MyContext, **{'rounding':'XYZ'})
        c = MyContext(rounding=ROUND_DOWN, prec=1)
        self.assertEqual(c.rounding, ROUND_DOWN)
        self.assertEqual(c.plus(Decimal('9.9')), 9)

        # Emin
        self.assertRaises(ValueError, MyContext, **{'Emin':5})
        c = MyContext(Emin=-1, prec=1)
        self.assertEqual(c.Emin, -1)
        x = c.add(Decimal('1e-99'), Decimal('2.234e-2000'))
        self.assertEqual(x, Decimal('0.0'))
        for signal in (Inexact, Underflow, Subnormal, Rounded, Clamped):
            self.assertTrue(c.flags[signal])

        # Emax
        self.assertRaises(ValueError, MyContext, **{'Emax':-1})
        c = MyContext(Emax=1, prec=1)
        self.assertEqual(c.Emax, 1)
        self.assertRaises(Overflow, c.add, Decimal('1e99'), Decimal('2.234e2000'))
        if self.decimal == C:
            for signal in (Inexact, Overflow, Rounded):
                self.assertTrue(c.flags[signal])

        # capitals
        self.assertRaises(ValueError, MyContext, **{'capitals':-1})
        c = MyContext(capitals=0)
        self.assertEqual(c.capitals, 0)
        x = c.create_decimal('1E222')
        self.assertEqual(c.to_sci_string(x), '1e+222')

        # clamp
        self.assertRaises(ValueError, MyContext, **{'clamp':2})
        c = MyContext(clamp=1, Emax=99)
        self.assertEqual(c.clamp, 1)
        x = c.plus(Decimal('1e99'))
        self.assertEqual(str(x), '1.000000000000000000000000000E+99')

        # flags
        self.assertRaises(TypeError, MyContext, **{'flags':'XYZ'})
        c = MyContext(flags=[Rounded, DivisionByZero])
        for signal in (Rounded, DivisionByZero):
            self.assertTrue(c.flags[signal])
        c.clear_flags()
        for signal in OrderedSignals[decimal]:
            self.assertFalse(c.flags[signal])

        # traps
        self.assertRaises(TypeError, MyContext, **{'traps':'XYZ'})
        c = MyContext(traps=[Rounded, DivisionByZero])
        for signal in (Rounded, DivisionByZero):
            self.assertTrue(c.traps[signal])
        c.clear_traps()
        for signal in OrderedSignals[decimal]:
            self.assertFalse(c.traps[signal])

class CContextSubclassing(ContextSubclassing):
    decimal = C
class PyContextSubclassing(ContextSubclassing):
    decimal = P

@skip_if_extra_functionality
class CheckAttributes(unittest.TestCase):

    def test_module_attributes(self):

        # Architecture dependent context limits
        self.assertEqual(C.MAX_PREC, P.MAX_PREC)
        self.assertEqual(C.MAX_EMAX, P.MAX_EMAX)
        self.assertEqual(C.MIN_EMIN, P.MIN_EMIN)
        self.assertEqual(C.MIN_ETINY, P.MIN_ETINY)

        self.assertTrue(C.HAVE_THREADS is True or C.HAVE_THREADS is False)
        self.assertTrue(P.HAVE_THREADS is True or P.HAVE_THREADS is False)

        self.assertEqual(C.__version__, P.__version__)

        self.assertEqual(dir(C), dir(P))

    def test_context_attributes(self):

        x = [s for s in dir(C.Context()) if '__' in s or not s.startswith('_')]
        y = [s for s in dir(P.Context()) if '__' in s or not s.startswith('_')]
        self.assertEqual(set(x) - set(y), set())

    def test_decimal_attributes(self):

        x = [s for s in dir(C.Decimal(9)) if '__' in s or not s.startswith('_')]
        y = [s for s in dir(C.Decimal(9)) if '__' in s or not s.startswith('_')]
        self.assertEqual(set(x) - set(y), set())

class Coverage(unittest.TestCase):

    def test_adjusted(self):
        Decimal = self.decimal.Decimal

        self.assertEqual(Decimal('1234e9999').adjusted(), 10002)
        # XXX raise?
        self.assertEqual(Decimal('nan').adjusted(), 0)
        self.assertEqual(Decimal('inf').adjusted(), 0)

    def test_canonical(self):
        Decimal = self.decimal.Decimal
        getcontext = self.decimal.getcontext

        x = Decimal(9).canonical()
        self.assertEqual(x, 9)

        c = getcontext()
        x = c.canonical(Decimal(9))
        self.assertEqual(x, 9)

    def test_context_repr(self):
        c = self.decimal.DefaultContext.copy()

        c.prec = 425000000
        c.Emax = 425000000
        c.Emin = -425000000
        c.rounding = ROUND_HALF_DOWN
        c.capitals = 0
        c.clamp = 1
        for sig in OrderedSignals[self.decimal]:
            c.flags[sig] = False
            c.traps[sig] = False

        s = c.__repr__()
        t = "Context(prec=425000000, rounding=ROUND_HALF_DOWN, " \
            "Emin=-425000000, Emax=425000000, capitals=0, clamp=1, " \
            "flags=[], traps=[])"
        self.assertEqual(s, t)

    def test_implicit_context(self):
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext

        with localcontext() as c:
            c.prec = 1
            c.Emax = 1
            c.Emin = -1

            # abs
            self.assertEqual(abs(Decimal("-10")), 10)
            # add
            self.assertEqual(Decimal("7") + 1, 8)
            # divide
            self.assertEqual(Decimal("10") / 5, 2)
            # divide_int
            self.assertEqual(Decimal("10") // 7, 1)
            # fma
            self.assertEqual(Decimal("1.2").fma(Decimal("0.01"), 1), 1)
            self.assertIs(Decimal("NaN").fma(7, 1).is_nan(), True)
            # three arg power
            self.assertEqual(pow(Decimal(10), 2, 7), 2)
            # exp
            self.assertEqual(Decimal("1.01").exp(), 3)
            # is_normal
            self.assertIs(Decimal("0.01").is_normal(), False)
            # is_subnormal
            self.assertIs(Decimal("0.01").is_subnormal(), True)
            # ln
            self.assertEqual(Decimal("20").ln(), 3)
            # log10
            self.assertEqual(Decimal("20").log10(), 1)
            # logb
            self.assertEqual(Decimal("580").logb(), 2)
            # logical_invert
            self.assertEqual(Decimal("10").logical_invert(), 1)
            # minus
            self.assertEqual(-Decimal("-10"), 10)
            # multiply
            self.assertEqual(Decimal("2") * 4, 8)
            # next_minus
            self.assertEqual(Decimal("10").next_minus(), 9)
            # next_plus
            self.assertEqual(Decimal("10").next_plus(), Decimal('2E+1'))
            # normalize
            self.assertEqual(Decimal("-10").normalize(), Decimal('-1E+1'))
            # number_class
            self.assertEqual(Decimal("10").number_class(), '+Normal')
            # plus
            self.assertEqual(+Decimal("-1"), -1)
            # remainder
            self.assertEqual(Decimal("10") % 7, 3)
            # subtract
            self.assertEqual(Decimal("10") - 7, 3)
            # to_integral_exact
            self.assertEqual(Decimal("1.12345").to_integral_exact(), 1)

            # Boolean functions
            self.assertTrue(Decimal("1").is_canonical())
            self.assertTrue(Decimal("1").is_finite())
            self.assertTrue(Decimal("1").is_finite())
            self.assertTrue(Decimal("snan").is_snan())
            self.assertTrue(Decimal("-1").is_signed())
            self.assertTrue(Decimal("0").is_zero())
            self.assertTrue(Decimal("0").is_zero())

        # Copy
        with localcontext() as c:
            c.prec = 10000
            x = 1228 ** 1523
            y = -Decimal(x)

            z = y.copy_abs()
            self.assertEqual(z, x)

            z = y.copy_negate()
            self.assertEqual(z, x)

            z = y.copy_sign(Decimal(1))
            self.assertEqual(z, x)

    def test_divmod(self):
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext
        InvalidOperation = self.decimal.InvalidOperation
        DivisionByZero = self.decimal.DivisionByZero

        with localcontext() as c:
            q, r = divmod(Decimal("10912837129"), 1001)
            self.assertEqual(q, Decimal('10901935'))
            self.assertEqual(r, Decimal('194'))

            q, r = divmod(Decimal("NaN"), 7)
            self.assertTrue(q.is_nan() and r.is_nan())

            c.traps[InvalidOperation] = False
            q, r = divmod(Decimal("NaN"), 7)
            self.assertTrue(q.is_nan() and r.is_nan())

            c.traps[InvalidOperation] = False
            c.clear_flags()
            q, r = divmod(Decimal("inf"), Decimal("inf"))
            self.assertTrue(q.is_nan() and r.is_nan())
            self.assertTrue(c.flags[InvalidOperation])

            c.clear_flags()
            q, r = divmod(Decimal("inf"), 101)
            self.assertTrue(q.is_infinite() and r.is_nan())
            self.assertTrue(c.flags[InvalidOperation])

            c.clear_flags()
            q, r = divmod(Decimal(0), 0)
            self.assertTrue(q.is_nan() and r.is_nan())
            self.assertTrue(c.flags[InvalidOperation])

            c.traps[DivisionByZero] = False
            c.clear_flags()
            q, r = divmod(Decimal(11), 0)
            self.assertTrue(q.is_infinite() and r.is_nan())
            self.assertTrue(c.flags[InvalidOperation] and
                            c.flags[DivisionByZero])

    def test_power(self):
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext
        Overflow = self.decimal.Overflow
        Rounded = self.decimal.Rounded

        with localcontext() as c:
            c.prec = 3
            c.clear_flags()
            self.assertEqual(Decimal("1.0") ** 100, Decimal('1.00'))
            self.assertTrue(c.flags[Rounded])

            c.prec = 1
            c.Emax = 1
            c.Emin = -1
            c.clear_flags()
            c.traps[Overflow] = False
            self.assertEqual(Decimal(10000) ** Decimal("0.5"), Decimal('inf'))
            self.assertTrue(c.flags[Overflow])

    def test_quantize(self):
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext
        InvalidOperation = self.decimal.InvalidOperation

        with localcontext() as c:
            c.prec = 1
            c.Emax = 1
            c.Emin = -1
            c.traps[InvalidOperation] = False
            x = Decimal(99).quantize(Decimal("1e1"))
            self.assertTrue(x.is_nan())

    def test_radix(self):
        Decimal = self.decimal.Decimal
        getcontext = self.decimal.getcontext

        c = getcontext()
        self.assertEqual(Decimal("1").radix(), 10)
        self.assertEqual(c.radix(), 10)

    def test_rop(self):
        Decimal = self.decimal.Decimal

        for attr in ('__radd__', '__rsub__', '__rmul__', '__rtruediv__',
                     '__rdivmod__', '__rmod__', '__rfloordiv__', '__rpow__'):
            self.assertIs(getattr(Decimal("1"), attr)("xyz"), NotImplemented)

    def test_round(self):
        # Python3 behavior: round() returns Decimal
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext

        with localcontext() as c:
            c.prec = 28

            self.assertEqual(str(Decimal("9.99").__round__()), "10")
            self.assertEqual(str(Decimal("9.99e-5").__round__()), "0")
            self.assertEqual(str(Decimal("1.23456789").__round__(5)), "1.23457")
            self.assertEqual(str(Decimal("1.2345").__round__(10)), "1.2345000000")
            self.assertEqual(str(Decimal("1.2345").__round__(-10)), "0E+10")

            self.assertRaises(TypeError, Decimal("1.23").__round__, "5")
            self.assertRaises(TypeError, Decimal("1.23").__round__, 5, 8)

    def test_create_decimal(self):
        c = self.decimal.Context()
        self.assertRaises(ValueError, c.create_decimal, ["%"])

    def test_int(self):
        Decimal = self.decimal.Decimal
        localcontext = self.decimal.localcontext

        with localcontext() as c:
            c.prec = 9999
            x = Decimal(1221**1271) / 10**3923
            self.assertEqual(int(x), 1)
            self.assertEqual(x.to_integral(), 2)

    def test_copy(self):
        Context = self.decimal.Context

        c = Context()
        c.prec = 10000
        x = -(1172 ** 1712)

        y = c.copy_abs(x)
        self.assertEqual(y, -x)

        y = c.copy_negate(x)
        self.assertEqual(y, -x)

        y = c.copy_sign(x, 1)
        self.assertEqual(y, -x)

class CCoverage(Coverage):
    decimal = C
class PyCoverage(Coverage):
    decimal = P

    def setUp(self):
        super().setUp()
        self._previous_int_limit = sys.get_int_max_str_digits()
        sys.set_int_max_str_digits(7000)

    def tearDown(self):
        sys.set_int_max_str_digits(self._previous_int_limit)
        super().tearDown()

class PyFunctionality(unittest.TestCase):
    """Extra functionality in decimal.py"""

    def test_py_alternate_formatting(self):
        # triples giving a format, a Decimal, and the expected result
        Decimal = P.Decimal
        localcontext = P.localcontext

        test_values = [
            # Issue 7094: Alternate formatting (specified by #)
            ('.0e', '1.0', '1e+0'),
            ('#.0e', '1.0', '1.e+0'),
            ('.0f', '1.0', '1'),
            ('#.0f', '1.0', '1.'),
            ('g', '1.1', '1.1'),
            ('#g', '1.1', '1.1'),
            ('.0g', '1', '1'),
            ('#.0g', '1', '1.'),
            ('.0%', '1.0', '100%'),
            ('#.0%', '1.0', '100.%'),
            ]
        for fmt, d, result in test_values:
            self.assertEqual(format(Decimal(d), fmt), result)

class PyWhitebox(unittest.TestCase):
    """White box testing for decimal.py"""

    def test_py_exact_power(self):
        # Rarely exercised lines in _power_exact.
        Decimal = P.Decimal
        localcontext = P.localcontext

        with localcontext() as c:
            c.prec = 8
            x = Decimal(2**16) ** Decimal("-0.5")
            self.assertEqual(x, Decimal('0.00390625'))

            x = Decimal(2**16) ** Decimal("-0.6")
            self.assertEqual(x, Decimal('0.0012885819'))

            x = Decimal("256e7") ** Decimal("-0.5")

            x = Decimal(152587890625) ** Decimal('-0.0625')
            self.assertEqual(x, Decimal("0.2"))

            x = Decimal("152587890625e7") ** Decimal('-0.0625')

            x = Decimal(5**2659) ** Decimal('-0.0625')

            c.prec = 1
            x = Decimal("152587890625") ** Decimal('-0.5')
            c.prec = 201
            x = Decimal(2**578) ** Decimal("-0.5")

    def test_py_immutability_operations(self):
        # Do operations and check that it didn't change internal objects.
        Decimal = P.Decimal
        DefaultContext = P.DefaultContext
        setcontext = P.setcontext

        c = DefaultContext.copy()
        c.traps = dict((s, 0) for s in OrderedSignals[P])
        setcontext(c)

        d1 = Decimal('-25e55')
        b1 = Decimal('-25e55')
        d2 = Decimal('33e+33')
        b2 = Decimal('33e+33')

        def checkSameDec(operation, useOther=False):
            if useOther:
                eval("d1." + operation + "(d2)")
                self.assertEqual(d1._sign, b1._sign)
                self.assertEqual(d1._int, b1._int)
                self.assertEqual(d1._exp, b1._exp)
                self.assertEqual(d2._sign, b2._sign)
                self.assertEqual(d2._int, b2._int)
                self.assertEqual(d2._exp, b2._exp)
            else:
                eval("d1." + operation + "()")
                self.assertEqual(d1._sign, b1._sign)
                self.assertEqual(d1._int, b1._int)
                self.assertEqual(d1._exp, b1._exp)

        Decimal(d1)
        self.assertEqual(d1._sign, b1._sign)
        self.assertEqual(d1._int, b1._int)
        self.assertEqual(d1._exp, b1._exp)

        checkSameDec("__abs__")
        checkSameDec("__add__", True)
        checkSameDec("__divmod__", True)
        checkSameDec("__eq__", True)
        checkSameDec("__ne__", True)
        checkSameDec("__le__", True)
        checkSameDec("__lt__", True)
        checkSameDec("__ge__", True)
        checkSameDec("__gt__", True)
        checkSameDec("__float__")
        checkSameDec("__floordiv__", True)
        checkSameDec("__hash__")
        checkSameDec("__int__")
        checkSameDec("__trunc__")
        checkSameDec("__mod__", True)
        checkSameDec("__mul__", True)
        checkSameDec("__neg__")
        checkSameDec("__bool__")
        checkSameDec("__pos__")
        checkSameDec("__pow__", True)
        checkSameDec("__radd__", True)
        checkSameDec("__rdivmod__", True)
        checkSameDec("__repr__")
        checkSameDec("__rfloordiv__", True)
        checkSameDec("__rmod__", True)
        checkSameDec("__rmul__", True)
        checkSameDec("__rpow__", True)
        checkSameDec("__rsub__", True)
        checkSameDec("__str__")
        checkSameDec("__sub__", True)
        checkSameDec("__truediv__", True)
        checkSameDec("adjusted")
        checkSameDec("as_tuple")
        checkSameDec("compare", True)
        checkSameDec("max", True)
        checkSameDec("min", True)
        checkSameDec("normalize")
        checkSameDec("quantize", True)
        checkSameDec("remainder_near", True)
        checkSameDec("same_quantum", True)
        checkSameDec("sqrt")
        checkSameDec("to_eng_string")
        checkSameDec("to_integral")

    def test_py_decimal_id(self):
        Decimal = P.Decimal

        d = Decimal(45)
        e = Decimal(d)
        self.assertEqual(str(e), '45')
        self.assertNotEqual(id(d), id(e))

    def test_py_rescale(self):
        # Coverage
        Decimal = P.Decimal
        localcontext = P.localcontext

        with localcontext() as c:
            x = Decimal("NaN")._rescale(3, ROUND_UP)
            self.assertTrue(x.is_nan())

    def test_py__round(self):
        # Coverage
        Decimal = P.Decimal

        self.assertRaises(ValueError, Decimal("3.1234")._round, 0, ROUND_UP)

class CFunctionality(unittest.TestCase):
    """Extra functionality in _decimal"""

    @requires_extra_functionality
    def test_c_ieee_context(self):
        # issue 8786: Add support for IEEE 754 contexts to decimal module.
        IEEEContext = C.IEEEContext
        DECIMAL32 = C.DECIMAL32
        DECIMAL64 = C.DECIMAL64
        DECIMAL128 = C.DECIMAL128

        def assert_rest(self, context):
            self.assertEqual(context.clamp, 1)
            assert_signals(self, context, 'traps', [])
            assert_signals(self, context, 'flags', [])

        c = IEEEContext(DECIMAL32)
        self.assertEqual(c.prec, 7)
        self.assertEqual(c.Emax, 96)
        self.assertEqual(c.Emin, -95)
        assert_rest(self, c)

        c = IEEEContext(DECIMAL64)
        self.assertEqual(c.prec, 16)
        self.assertEqual(c.Emax, 384)
        self.assertEqual(c.Emin, -383)
        assert_rest(self, c)

        c = IEEEContext(DECIMAL128)
        self.assertEqual(c.prec, 34)
        self.assertEqual(c.Emax, 6144)
        self.assertEqual(c.Emin, -6143)
        assert_rest(self, c)

        # Invalid values
        self.assertRaises(OverflowError, IEEEContext, 2**63)
        self.assertRaises(ValueError, IEEEContext, -1)
        self.assertRaises(ValueError, IEEEContext, 1024)

    @requires_extra_functionality
    def test_c_context(self):
        Context = C.Context

        c = Context(flags=C.DecClamped, traps=C.DecRounded)
        self.assertEqual(c._flags, C.DecClamped)
        self.assertEqual(c._traps, C.DecRounded)

    @requires_extra_functionality
    def test_constants(self):
        # Condition flags
        cond = (
            C.DecClamped, C.DecConversionSyntax, C.DecDivisionByZero,
            C.DecDivisionImpossible, C.DecDivisionUndefined,
            C.DecFpuError, C.DecInexact, C.DecInvalidContext,
            C.DecInvalidOperation, C.DecMallocError,
            C.DecFloatOperation, C.DecOverflow, C.DecRounded,
            C.DecSubnormal, C.DecUnderflow
        )

        # IEEEContext
        self.assertEqual(C.DECIMAL32, 32)
        self.assertEqual(C.DECIMAL64, 64)
        self.assertEqual(C.DECIMAL128, 128)
        self.assertEqual(C.IEEE_CONTEXT_MAX_BITS, 512)

        # Conditions
        for i, v in enumerate(cond):
            self.assertEqual(v, 1<<i)

        self.assertEqual(C.DecIEEEInvalidOperation,
                         C.DecConversionSyntax|
                         C.DecDivisionImpossible|
                         C.DecDivisionUndefined|
                         C.DecFpuError|
                         C.DecInvalidContext|
                         C.DecInvalidOperation|
                         C.DecMallocError)

        self.assertEqual(C.DecErrors,
                         C.DecIEEEInvalidOperation|
                         C.DecDivisionByZero)

        self.assertEqual(C.DecTraps,
                         C.DecErrors|C.DecOverflow|C.DecUnderflow)

class CWhitebox(unittest.TestCase):
    """Whitebox testing for _decimal"""

    def test_bignum(self):
        # Not exactly whitebox, but too slow with pydecimal.

        Decimal = C.Decimal
        localcontext = C.localcontext

        b1 = 10**35
        b2 = 10**36
        with localcontext() as c:
            c.prec = 1000000
            for i in range(5):
                a = random.randrange(b1, b2)
                b = random.randrange(1000, 1200)
                x = a ** b
                y = Decimal(a) ** Decimal(b)
                self.assertEqual(x, y)

    def test_invalid_construction(self):
        self.assertRaises(TypeError, C.Decimal, 9, "xyz")

    def test_c_input_restriction(self):
        # Too large for _decimal to be converted exactly
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        Context = C.Context
        localcontext = C.localcontext

        with localcontext(Context()):
            self.assertRaises(InvalidOperation, Decimal,
                              "1e9999999999999999999")

    def test_c_context_repr(self):
        # This test is _decimal-only because flags are not printed
        # in the same order.
        DefaultContext = C.DefaultContext
        FloatOperation = C.FloatOperation

        c = DefaultContext.copy()

        c.prec = 425000000
        c.Emax = 425000000
        c.Emin = -425000000
        c.rounding = ROUND_HALF_DOWN
        c.capitals = 0
        c.clamp = 1
        for sig in OrderedSignals[C]:
            c.flags[sig] = True
            c.traps[sig] = True
        c.flags[FloatOperation] = True
        c.traps[FloatOperation] = True

        s = c.__repr__()
        t = "Context(prec=425000000, rounding=ROUND_HALF_DOWN, " \
            "Emin=-425000000, Emax=425000000, capitals=0, clamp=1, " \
            "flags=[Clamped, InvalidOperation, DivisionByZero, Inexact, " \
                   "FloatOperation, Overflow, Rounded, Subnormal, Underflow], " \
            "traps=[Clamped, InvalidOperation, DivisionByZero, Inexact, " \
                   "FloatOperation, Overflow, Rounded, Subnormal, Underflow])"
        self.assertEqual(s, t)

    def test_c_context_errors(self):
        Context = C.Context
        InvalidOperation = C.InvalidOperation
        Overflow = C.Overflow
        FloatOperation = C.FloatOperation
        localcontext = C.localcontext
        getcontext = C.getcontext
        setcontext = C.setcontext
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        c = Context()

        # SignalDict: input validation
        self.assertRaises(KeyError, c.flags.__setitem__, 801, 0)
        self.assertRaises(KeyError, c.traps.__setitem__, 801, 0)
        self.assertRaises(ValueError, c.flags.__delitem__, Overflow)
        self.assertRaises(ValueError, c.traps.__delitem__, InvalidOperation)
        self.assertRaises(TypeError, setattr, c, 'flags', ['x'])
        self.assertRaises(TypeError, setattr, c,'traps', ['y'])
        self.assertRaises(KeyError, setattr, c, 'flags', {0:1})
        self.assertRaises(KeyError, setattr, c, 'traps', {0:1})

        # Test assignment from a signal dict with the correct length but
        # one invalid key.
        d = c.flags.copy()
        del d[FloatOperation]
        d["XYZ"] = 91283719
        self.assertRaises(KeyError, setattr, c, 'flags', d)
        self.assertRaises(KeyError, setattr, c, 'traps', d)

        # Input corner cases
        int_max = 2**63-1 if HAVE_CONFIG_64 else 2**31-1
        gt_max_emax = 10**18 if HAVE_CONFIG_64 else 10**9

        # prec, Emax, Emin
        for attr in ['prec', 'Emax']:
            self.assertRaises(ValueError, setattr, c, attr, gt_max_emax)
        self.assertRaises(ValueError, setattr, c, 'Emin', -gt_max_emax)

        # prec, Emax, Emin in context constructor
        self.assertRaises(ValueError, Context, prec=gt_max_emax)
        self.assertRaises(ValueError, Context, Emax=gt_max_emax)
        self.assertRaises(ValueError, Context, Emin=-gt_max_emax)

        # Overflow in conversion
        self.assertRaises(OverflowError, Context, prec=int_max+1)
        self.assertRaises(OverflowError, Context, Emax=int_max+1)
        self.assertRaises(OverflowError, Context, Emin=-int_max-2)
        self.assertRaises(OverflowError, Context, clamp=int_max+1)
        self.assertRaises(OverflowError, Context, capitals=int_max+1)

        # OverflowError, general ValueError
        for attr in ('prec', 'Emin', 'Emax', 'capitals', 'clamp'):
            self.assertRaises(OverflowError, setattr, c, attr, int_max+1)
            self.assertRaises(OverflowError, setattr, c, attr, -int_max-2)
            if sys.platform != 'win32':
                self.assertRaises(ValueError, setattr, c, attr, int_max)
                self.assertRaises(ValueError, setattr, c, attr, -int_max-1)

        # OverflowError: _unsafe_setprec, _unsafe_setemin, _unsafe_setemax
        if C.MAX_PREC == 425000000:
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setprec'),
                              int_max+1)
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setemax'),
                              int_max+1)
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setemin'),
                              -int_max-2)

        # ValueError: _unsafe_setprec, _unsafe_setemin, _unsafe_setemax
        if C.MAX_PREC == 425000000:
            self.assertRaises(ValueError, getattr(c, '_unsafe_setprec'), 0)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setprec'),
                              1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemax'), -1)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemax'),
                              1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemin'),
                              -1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemin'), 1)

        # capitals, clamp
        for attr in ['capitals', 'clamp']:
            self.assertRaises(ValueError, setattr, c, attr, -1)
            self.assertRaises(ValueError, setattr, c, attr, 2)
            self.assertRaises(TypeError, setattr, c, attr, [1,2,3])
            if HAVE_CONFIG_64:
                self.assertRaises(ValueError, setattr, c, attr, 2**32)
                self.assertRaises(ValueError, setattr, c, attr, 2**32+1)

        # Invalid local context
        self.assertRaises(TypeError, exec, 'with localcontext("xyz"): pass',
                          locals())
        self.assertRaises(TypeError, exec,
                          'with localcontext(context=getcontext()): pass',
                          locals())

        # setcontext
        saved_context = getcontext()
        self.assertRaises(TypeError, setcontext, "xyz")
        setcontext(saved_context)

    def test_rounding_strings_interned(self):

        self.assertIs(C.ROUND_UP, P.ROUND_UP)
        self.assertIs(C.ROUND_DOWN, P.ROUND_DOWN)
        self.assertIs(C.ROUND_CEILING, P.ROUND_CEILING)
        self.assertIs(C.ROUND_FLOOR, P.ROUND_FLOOR)
        self.assertIs(C.ROUND_HALF_UP, P.ROUND_HALF_UP)
        self.assertIs(C.ROUND_HALF_DOWN, P.ROUND_HALF_DOWN)
        self.assertIs(C.ROUND_HALF_EVEN, P.ROUND_HALF_EVEN)
        self.assertIs(C.ROUND_05UP, P.ROUND_05UP)

    @requires_extra_functionality
    def test_c_context_errors_extra(self):
        Context = C.Context
        InvalidOperation = C.InvalidOperation
        Overflow = C.Overflow
        localcontext = C.localcontext
        getcontext = C.getcontext
        setcontext = C.setcontext
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        c = Context()

        # Input corner cases
        int_max = 2**63-1 if HAVE_CONFIG_64 else 2**31-1

        # OverflowError, general ValueError
        self.assertRaises(OverflowError, setattr, c, '_allcr', int_max+1)
        self.assertRaises(OverflowError, setattr, c, '_allcr', -int_max-2)
        if sys.platform != 'win32':
            self.assertRaises(ValueError, setattr, c, '_allcr', int_max)
            self.assertRaises(ValueError, setattr, c, '_allcr', -int_max-1)

        # OverflowError, general TypeError
        for attr in ('_flags', '_traps'):
            self.assertRaises(OverflowError, setattr, c, attr, int_max+1)
            self.assertRaises(OverflowError, setattr, c, attr, -int_max-2)
            if sys.platform != 'win32':
                self.assertRaises(TypeError, setattr, c, attr, int_max)
                self.assertRaises(TypeError, setattr, c, attr, -int_max-1)

        # _allcr
        self.assertRaises(ValueError, setattr, c, '_allcr', -1)
        self.assertRaises(ValueError, setattr, c, '_allcr', 2)
        self.assertRaises(TypeError, setattr, c, '_allcr', [1,2,3])
        if HAVE_CONFIG_64:
            self.assertRaises(ValueError, setattr, c, '_allcr', 2**32)
            self.assertRaises(ValueError, setattr, c, '_allcr', 2**32+1)

        # _flags, _traps
        for attr in ['_flags', '_traps']:
            self.assertRaises(TypeError, setattr, c, attr, 999999)
            self.assertRaises(TypeError, setattr, c, attr, 'x')

    def test_c_valid_context(self):
        # These tests are for code coverage in _decimal.
        DefaultContext = C.DefaultContext
        Clamped = C.Clamped
        Underflow = C.Underflow
        Inexact = C.Inexact
        Rounded = C.Rounded
        Subnormal = C.Subnormal

        c = DefaultContext.copy()

        # Exercise all getters and setters
        c.prec = 34
        c.rounding = ROUND_HALF_UP
        c.Emax = 3000
        c.Emin = -3000
        c.capitals = 1
        c.clamp = 0

        self.assertEqual(c.prec, 34)
        self.assertEqual(c.rounding, ROUND_HALF_UP)
        self.assertEqual(c.Emin, -3000)
        self.assertEqual(c.Emax, 3000)
        self.assertEqual(c.capitals, 1)
        self.assertEqual(c.clamp, 0)

        self.assertEqual(c.Etiny(), -3033)
        self.assertEqual(c.Etop(), 2967)

        # Exercise all unsafe setters
        if C.MAX_PREC == 425000000:
            c._unsafe_setprec(999999999)
            c._unsafe_setemax(999999999)
            c._unsafe_setemin(-999999999)
            self.assertEqual(c.prec, 999999999)
            self.assertEqual(c.Emax, 999999999)
            self.assertEqual(c.Emin, -999999999)

    @requires_extra_functionality
    def test_c_valid_context_extra(self):
        DefaultContext = C.DefaultContext

        c = DefaultContext.copy()
        self.assertEqual(c._allcr, 1)
        c._allcr = 0
        self.assertEqual(c._allcr, 0)

    def test_c_round(self):
        # Restricted input.
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        localcontext = C.localcontext
        MAX_EMAX = C.MAX_EMAX
        MIN_ETINY = C.MIN_ETINY
        int_max = 2**63-1 if C.MAX_PREC > 425000000 else 2**31-1

        with localcontext() as c:
            c.traps[InvalidOperation] = True
            self.assertRaises(InvalidOperation, Decimal("1.23").__round__,
                              -int_max-1)
            self.assertRaises(InvalidOperation, Decimal("1.23").__round__,
                              int_max)
            self.assertRaises(InvalidOperation, Decimal("1").__round__,
                              int(MAX_EMAX+1))
            self.assertRaises(C.InvalidOperation, Decimal("1").__round__,
                              -int(MIN_ETINY-1))
            self.assertRaises(OverflowError, Decimal("1.23").__round__,
                              -int_max-2)
            self.assertRaises(OverflowError, Decimal("1.23").__round__,
                              int_max+1)

    def test_c_format(self):
        # Restricted input
        Decimal = C.Decimal
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        self.assertRaises(TypeError, Decimal(1).__format__, "=10.10", [], 9)
        self.assertRaises(TypeError, Decimal(1).__format__, "=10.10", 9)
        self.assertRaises(TypeError, Decimal(1).__format__, [])

        self.assertRaises(ValueError, Decimal(1).__format__, "<>=10.10")
        maxsize = 2**63-1 if HAVE_CONFIG_64 else 2**31-1
        self.assertRaises(ValueError, Decimal("1.23456789").__format__,
                          "=%d.1" % maxsize)

    def test_c_integral(self):
        Decimal = C.Decimal
        Inexact = C.Inexact
        localcontext = C.localcontext

        x = Decimal(10)
        self.assertEqual(x.to_integral(), 10)
        self.assertRaises(TypeError, x.to_integral, '10')
        self.assertRaises(TypeError, x.to_integral, 10, 'x')
        self.assertRaises(TypeError, x.to_integral, 10)

        self.assertEqual(x.to_integral_value(), 10)
        self.assertRaises(TypeError, x.to_integral_value, '10')
        self.assertRaises(TypeError, x.to_integral_value, 10, 'x')
        self.assertRaises(TypeError, x.to_integral_value, 10)

        self.assertEqual(x.to_integral_exact(), 10)
        self.assertRaises(TypeError, x.to_integral_exact, '10')
        self.assertRaises(TypeError, x.to_integral_exact, 10, 'x')
        self.assertRaises(TypeError, x.to_integral_exact, 10)

        with localcontext() as c:
            x = Decimal("99999999999999999999999999.9").to_integral_value(ROUND_UP)
            self.assertEqual(x, Decimal('100000000000000000000000000'))

            x = Decimal("99999999999999999999999999.9").to_integral_exact(ROUND_UP)
            self.assertEqual(x, Decimal('100000000000000000000000000'))

            c.traps[Inexact] = True
            self.assertRaises(Inexact, Decimal("999.9").to_integral_exact, ROUND_UP)

    def test_c_funcs(self):
        # Invalid arguments
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        DivisionByZero = C.DivisionByZero
        getcontext = C.getcontext
        localcontext = C.localcontext

        self.assertEqual(Decimal('9.99e10').to_eng_string(), '99.9E+9')

        self.assertRaises(TypeError, pow, Decimal(1), 2, "3")
        self.assertRaises(TypeError, Decimal(9).number_class, "x", "y")
        self.assertRaises(TypeError, Decimal(9).same_quantum, 3, "x", "y")

        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), []
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), getcontext()
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), 10
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), ROUND_UP, 1000
        )

        with localcontext() as c:
            c.clear_traps()

            # Invalid arguments
            self.assertRaises(TypeError, c.copy_sign, Decimal(1), "x", "y")
            self.assertRaises(TypeError, c.canonical, 200)
            self.assertRaises(TypeError, c.is_canonical, 200)
            self.assertRaises(TypeError, c.divmod, 9, 8, "x", "y")
            self.assertRaises(TypeError, c.same_quantum, 9, 3, "x", "y")

            self.assertEqual(str(c.canonical(Decimal(200))), '200')
            self.assertEqual(c.radix(), 10)

            c.traps[DivisionByZero] = True
            self.assertRaises(DivisionByZero, Decimal(9).__divmod__, 0)
            self.assertRaises(DivisionByZero, c.divmod, 9, 0)
            self.assertTrue(c.flags[InvalidOperation])

            c.clear_flags()
            c.traps[InvalidOperation] = True
            self.assertRaises(InvalidOperation, Decimal(9).__divmod__, 0)
            self.assertRaises(InvalidOperation, c.divmod, 9, 0)
            self.assertTrue(c.flags[DivisionByZero])

            c.traps[InvalidOperation] = True
            c.prec = 2
            self.assertRaises(InvalidOperation, pow, Decimal(1000), 1, 501)

    def test_va_args_exceptions(self):
        Decimal = C.Decimal
        Context = C.Context

        x = Decimal("10001111111")

        for attr in ['exp', 'is_normal', 'is_subnormal', 'ln', 'log10',
                     'logb', 'logical_invert', 'next_minus', 'next_plus',
                     'normalize', 'number_class', 'sqrt', 'to_eng_string']:
            func = getattr(x, attr)
            self.assertRaises(TypeError, func, context="x")
            self.assertRaises(TypeError, func, "x", context=None)

        for attr in ['compare', 'compare_signal', 'logical_and',
                     'logical_or', 'max', 'max_mag', 'min', 'min_mag',
                     'remainder_near', 'rotate', 'scaleb', 'shift']:
            func = getattr(x, attr)
            self.assertRaises(TypeError, func, context="x")
            self.assertRaises(TypeError, func, "x", context=None)

        self.assertRaises(TypeError, x.to_integral, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral, [], [])

        self.assertRaises(TypeError, x.to_integral_value, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral_value, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral_value, [], [])

        self.assertRaises(TypeError, x.to_integral_exact, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral_exact, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral_exact, [], [])

        self.assertRaises(TypeError, x.fma, 1, 2, context="x")
        self.assertRaises(TypeError, x.fma, 1, 2, "x", context=None)

        self.assertRaises(TypeError, x.quantize, 1, [], context=None)
        self.assertRaises(TypeError, x.quantize, 1, [], rounding=None)
        self.assertRaises(TypeError, x.quantize, 1, [], [])

        c = Context()
        self.assertRaises(TypeError, c.power, 1, 2, mod="x")
        self.assertRaises(TypeError, c.power, 1, "x", mod=None)
        self.assertRaises(TypeError, c.power, "x", 2, mod=None)

    @requires_extra_functionality
    def test_c_context_templates(self):
        self.assertEqual(
            C.BasicContext._traps,
            C.DecIEEEInvalidOperation|C.DecDivisionByZero|C.DecOverflow|
            C.DecUnderflow|C.DecClamped
        )
        self.assertEqual(
            C.DefaultContext._traps,
            C.DecIEEEInvalidOperation|C.DecDivisionByZero|C.DecOverflow
        )

    @requires_extra_functionality
    def test_c_signal_dict(self):

        # SignalDict coverage
        Context = C.Context
        DefaultContext = C.DefaultContext

        InvalidOperation = C.InvalidOperation
        FloatOperation = C.FloatOperation
        DivisionByZero = C.DivisionByZero
        Overflow = C.Overflow
        Subnormal = C.Subnormal
        Underflow = C.Underflow
        Rounded = C.Rounded
        Inexact = C.Inexact
        Clamped = C.Clamped

        DecClamped = C.DecClamped
        DecInvalidOperation = C.DecInvalidOperation
        DecIEEEInvalidOperation = C.DecIEEEInvalidOperation

        def assertIsExclusivelySet(signal, signal_dict):
            for sig in signal_dict:
                if sig == signal:
                    self.assertTrue(signal_dict[sig])
                else:
                    self.assertFalse(signal_dict[sig])

        c = DefaultContext.copy()

        # Signal dict methods
        self.assertTrue(Overflow in c.traps)
        c.clear_traps()
        for k in c.traps.keys():
            c.traps[k] = True
        for v in c.traps.values():
            self.assertTrue(v)
        c.clear_traps()
        for k, v in c.traps.items():
            self.assertFalse(v)

        self.assertFalse(c.flags.get(Overflow))
        self.assertIs(c.flags.get("x"), None)
        self.assertEqual(c.flags.get("x", "y"), "y")
        self.assertRaises(TypeError, c.flags.get, "x", "y", "z")

        self.assertEqual(len(c.flags), len(c.traps))
        s = sys.getsizeof(c.flags)
        s = sys.getsizeof(c.traps)
        s = c.flags.__repr__()

        # Set flags/traps.
        c.clear_flags()
        c._flags = DecClamped
        self.assertTrue(c.flags[Clamped])

        c.clear_traps()
        c._traps = DecInvalidOperation
        self.assertTrue(c.traps[InvalidOperation])

        # Set flags/traps from dictionary.
        c.clear_flags()
        d = c.flags.copy()
        d[DivisionByZero] = True
        c.flags = d
        assertIsExclusivelySet(DivisionByZero, c.flags)

        c.clear_traps()
        d = c.traps.copy()
        d[Underflow] = True
        c.traps = d
        assertIsExclusivelySet(Underflow, c.traps)

        # Random constructors
        IntSignals = {
          Clamped: C.DecClamped,
          Rounded: C.DecRounded,
          Inexact: C.DecInexact,
          Subnormal: C.DecSubnormal,
          Underflow: C.DecUnderflow,
          Overflow: C.DecOverflow,
          DivisionByZero: C.DecDivisionByZero,
          FloatOperation: C.DecFloatOperation,
          InvalidOperation: C.DecIEEEInvalidOperation
        }
        IntCond = [
          C.DecDivisionImpossible, C.DecDivisionUndefined, C.DecFpuError,
          C.DecInvalidContext, C.DecInvalidOperation, C.DecMallocError,
          C.DecConversionSyntax,
        ]

        lim = len(OrderedSignals[C])
        for r in range(lim):
            for t in range(lim):
                for round in RoundingModes:
                    flags = random.sample(OrderedSignals[C], r)
                    traps = random.sample(OrderedSignals[C], t)
                    prec = random.randrange(1, 10000)
                    emin = random.randrange(-10000, 0)
                    emax = random.randrange(0, 10000)
                    clamp = random.randrange(0, 2)
                    caps = random.randrange(0, 2)
                    cr = random.randrange(0, 2)
                    c = Context(prec=prec, rounding=round, Emin=emin, Emax=emax,
                                capitals=caps, clamp=clamp, flags=list(flags),
                                traps=list(traps))

                    self.assertEqual(c.prec, prec)
                    self.assertEqual(c.rounding, round)
                    self.assertEqual(c.Emin, emin)
                    self.assertEqual(c.Emax, emax)
                    self.assertEqual(c.capitals, caps)
                    self.assertEqual(c.clamp, clamp)

                    f = 0
                    for x in flags:
                        f |= IntSignals[x]
                    self.assertEqual(c._flags, f)

                    f = 0
                    for x in traps:
                        f |= IntSignals[x]
                    self.assertEqual(c._traps, f)

        for cond in IntCond:
            c._flags = cond
            self.assertTrue(c._flags&DecIEEEInvalidOperation)
            assertIsExclusivelySet(InvalidOperation, c.flags)

        for cond in IntCond:
            c._traps = cond
            self.assertTrue(c._traps&DecIEEEInvalidOperation)
            assertIsExclusivelySet(InvalidOperation, c.traps)

    def test_invalid_override(self):
        Decimal = C.Decimal

        try:
            from locale import CHAR_MAX
        except ImportError:
            self.skipTest('locale.CHAR_MAX not available')

        def make_grouping(lst):
            return ''.join([chr(x) for x in lst])

        def get_fmt(x, override=None, fmt='n'):
            return Decimal(x).__format__(fmt, override)

        invalid_grouping = {
            'decimal_point' : ',',
            'grouping' : make_grouping([255, 255, 0]),
            'thousands_sep' : ','
        }
        invalid_dot = {
            'decimal_point' : 'xxxxx',
            'grouping' : make_grouping([3, 3, 0]),
            'thousands_sep' : ','
        }
        invalid_sep = {
            'decimal_point' : '.',
            'grouping' : make_grouping([3, 3, 0]),
            'thousands_sep' : 'yyyyy'
        }

        if CHAR_MAX == 127: # negative grouping in override
            self.assertRaises(ValueError, get_fmt, 12345,
                              invalid_grouping, 'g')

        self.assertRaises(ValueError, get_fmt, 12345, invalid_dot, 'g')
        self.assertRaises(ValueError, get_fmt, 12345, invalid_sep, 'g')

    def test_exact_conversion(self):
        Decimal = C.Decimal
        localcontext = C.localcontext
        InvalidOperation = C.InvalidOperation

        with localcontext() as c:

            c.traps[InvalidOperation] = True

            # Clamped
            x = "0e%d" % sys.maxsize
            self.assertRaises(InvalidOperation, Decimal, x)

            x = "0e%d" % (-sys.maxsize-1)
            self.assertRaises(InvalidOperation, Decimal, x)

            # Overflow
            x = "1e%d" % sys.maxsize
            self.assertRaises(InvalidOperation, Decimal, x)

            # Underflow
            x = "1e%d" % (-sys.maxsize-1)
            self.assertRaises(InvalidOperation, Decimal, x)

    def test_from_tuple(self):
        Decimal = C.Decimal
        localcontext = C.localcontext
        InvalidOperation = C.InvalidOperation
        Overflow = C.Overflow
        Underflow = C.Underflow

        with localcontext() as c:

            c.prec = 9
            c.traps[InvalidOperation] = True
            c.traps[Overflow] = True
            c.traps[Underflow] = True

            # SSIZE_MAX
            x = (1, (), sys.maxsize)
            self.assertEqual(str(c.create_decimal(x)), '-0E+999999')
            self.assertRaises(InvalidOperation, Decimal, x)

            x = (1, (0, 1, 2), sys.maxsize)
            self.assertRaises(Overflow, c.create_decimal, x)
            self.assertRaises(InvalidOperation, Decimal, x)

            # SSIZE_MIN
            x = (1, (), -sys.maxsize-1)
            self.assertEqual(str(c.create_decimal(x)), '-0E-1000007')
            self.assertRaises(InvalidOperation, Decimal, x)

            x = (1, (0, 1, 2), -sys.maxsize-1)
            self.assertRaises(Underflow, c.create_decimal, x)
            self.assertRaises(InvalidOperation, Decimal, x)

            # OverflowError
            x = (1, (), sys.maxsize+1)
            self.assertRaises(OverflowError, c.create_decimal, x)
            self.assertRaises(OverflowError, Decimal, x)

            x = (1, (), -sys.maxsize-2)
            self.assertRaises(OverflowError, c.create_decimal, x)
            self.assertRaises(OverflowError, Decimal, x)

            # Specials
            x = (1, (), "N")
            self.assertEqual(str(Decimal(x)), '-sNaN')
            x = (1, (0,), "N")
            self.assertEqual(str(Decimal(x)), '-sNaN')
            x = (1, (0, 1), "N")
            self.assertEqual(str(Decimal(x)), '-sNaN1')

    def test_sizeof(self):
        Decimal = C.Decimal
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        self.assertGreater(Decimal(0).__sizeof__(), 0)
        if HAVE_CONFIG_64:
            x = Decimal(10**(19*24)).__sizeof__()
            y = Decimal(10**(19*25)).__sizeof__()
            self.assertEqual(y, x+8)
        else:
            x = Decimal(10**(9*24)).__sizeof__()
            y = Decimal(10**(9*25)).__sizeof__()
            self.assertEqual(y, x+4)

    def test_internal_use_of_overridden_methods(self):
        Decimal = C.Decimal

        # Unsound subtyping
        class X(float):
            def as_integer_ratio(self):
                return 1
            def __abs__(self):
                return self

        class Y(float):
            def __abs__(self):
                return [1]*200

        class I(int):
            def bit_length(self):
                return [1]*200

        class Z(float):
            def as_integer_ratio(self):
                return (I(1), I(1))
            def __abs__(self):
                return self

        for cls in X, Y, Z:
            self.assertEqual(Decimal.from_float(cls(101.1)),
                             Decimal.from_float(101.1))

    # Issue 41540:
    @unittest.skipIf(sys.platform.startswith("aix"),
                     "AIX: default ulimit: test is flaky because of extreme over-allocation")
    @unittest.skipIf(is_emscripten, "Test is unstable on Emscripten")
    @unittest.skipIf(check_sanitizer(address=True, memory=True),
                     "ASAN/MSAN sanitizer defaults to crashing "
                     "instead of returning NULL for malloc failure.")
    def test_maxcontext_exact_arith(self):

        # Make sure that exact operations do not raise MemoryError due
        # to huge intermediate values when the context precision is very
        # large.

        # The following functions fill the available precision and are
        # therefore not suitable for large precisions (by design of the
        # specification).
        MaxContextSkip = ['logical_invert', 'next_minus', 'next_plus',
                          'logical_and', 'logical_or', 'logical_xor',
                          'next_toward', 'rotate', 'shift']

        Decimal = C.Decimal
        Context = C.Context
        localcontext = C.localcontext

        # Here only some functions that are likely candidates for triggering a
        # MemoryError are tested.  deccheck.py has an exhaustive test.
        maxcontext = Context(prec=C.MAX_PREC, Emin=C.MIN_EMIN, Emax=C.MAX_EMAX)
        with localcontext(maxcontext):
            self.assertEqual(Decimal(0).exp(), 1)
            self.assertEqual(Decimal(1).ln(), 0)
            self.assertEqual(Decimal(1).log10(), 0)
            self.assertEqual(Decimal(10**2).log10(), 2)
            self.assertEqual(Decimal(10**223).log10(), 223)
            self.assertEqual(Decimal(10**19).logb(), 19)
            self.assertEqual(Decimal(4).sqrt(), 2)
            self.assertEqual(Decimal("40E9").sqrt(), Decimal('2.0E+5'))
            self.assertEqual(divmod(Decimal(10), 3), (3, 1))
            self.assertEqual(Decimal(10) // 3, 3)
            self.assertEqual(Decimal(4) / 2, 2)
            self.assertEqual(Decimal(400) ** -1, Decimal('0.0025'))


@requires_docstrings
@unittest.skipUnless(C, "test requires C version")
class SignatureTest(unittest.TestCase):
    """Function signatures"""

    def test_inspect_module(self):
        for attr in dir(P):
            if attr.startswith('_'):
                continue
            p_func = getattr(P, attr)
            c_func = getattr(C, attr)
            if (attr == 'Decimal' or attr == 'Context' or
                inspect.isfunction(p_func)):
                p_sig = inspect.signature(p_func)
                c_sig = inspect.signature(c_func)

                # parameter names:
                c_names = list(c_sig.parameters.keys())
                p_names = [x for x in p_sig.parameters.keys() if not
                           x.startswith('_')]

                self.assertEqual(c_names, p_names,
                                 msg="parameter name mismatch in %s" % p_func)

                c_kind = [x.kind for x in c_sig.parameters.values()]
                p_kind = [x[1].kind for x in p_sig.parameters.items() if not
                          x[0].startswith('_')]

                # parameters:
                if attr != 'setcontext':
                    self.assertEqual(c_kind, p_kind,
                                     msg="parameter kind mismatch in %s" % p_func)

    def test_inspect_types(self):

        POS = inspect._ParameterKind.POSITIONAL_ONLY
        POS_KWD = inspect._ParameterKind.POSITIONAL_OR_KEYWORD

        # Type heuristic (type annotations would help!):
        pdict = {C: {'other': C.Decimal(1),
                     'third': C.Decimal(1),
                     'x': C.Decimal(1),
                     'y': C.Decimal(1),
                     'z': C.Decimal(1),
                     'a': C.Decimal(1),
                     'b': C.Decimal(1),
                     'c': C.Decimal(1),
                     'exp': C.Decimal(1),
                     'modulo': C.Decimal(1),
                     'num': "1",
                     'f': 1.0,
                     'rounding': C.ROUND_HALF_UP,
                     'context': C.getcontext()},
                 P: {'other': P.Decimal(1),
                     'third': P.Decimal(1),
                     'a': P.Decimal(1),
                     'b': P.Decimal(1),
                     'c': P.Decimal(1),
                     'exp': P.Decimal(1),
                     'modulo': P.Decimal(1),
                     'num': "1",
                     'f': 1.0,
                     'rounding': P.ROUND_HALF_UP,
                     'context': P.getcontext()}}

        def mkargs(module, sig):
            args = []
            kwargs = {}
            for name, param in sig.parameters.items():
                if name == 'self': continue
                if param.kind == POS:
                    args.append(pdict[module][name])
                elif param.kind == POS_KWD:
                    kwargs[name] = pdict[module][name]
                else:
                    raise TestFailed("unexpected parameter kind")
            return args, kwargs

        def tr(s):
            """The C Context docstrings use 'x' in order to prevent confusion
               with the article 'a' in the descriptions."""
            if s == 'x': return 'a'
            if s == 'y': return 'b'
            if s == 'z': return 'c'
            return s

        def doit(ty):
            p_type = getattr(P, ty)
            c_type = getattr(C, ty)
            for attr in dir(p_type):
                if attr.startswith('_'):
                    continue
                p_func = getattr(p_type, attr)
                c_func = getattr(c_type, attr)
                if inspect.isfunction(p_func):
                    p_sig = inspect.signature(p_func)
                    c_sig = inspect.signature(c_func)

                    # parameter names:
                    p_names = list(p_sig.parameters.keys())
                    c_names = [tr(x) for x in c_sig.parameters.keys()]

                    self.assertEqual(c_names, p_names,
                                     msg="parameter name mismatch in %s" % p_func)

                    p_kind = [x.kind for x in p_sig.parameters.values()]
                    c_kind = [x.kind for x in c_sig.parameters.values()]

                    # 'self' parameter:
                    self.assertIs(p_kind[0], POS_KWD)
                    self.assertIs(c_kind[0], POS)

                    # remaining parameters:
                    if ty == 'Decimal':
                        self.assertEqual(c_kind[1:], p_kind[1:],
                                         msg="parameter kind mismatch in %s" % p_func)
                    else: # Context methods are positional only in the C version.
                        self.assertEqual(len(c_kind), len(p_kind),
                                         msg="parameter kind mismatch in %s" % p_func)

                    # Run the function:
                    args, kwds = mkargs(C, c_sig)
                    try:
                        getattr(c_type(9), attr)(*args, **kwds)
                    except Exception:
                        raise TestFailed("invalid signature for %s: %s %s" % (c_func, args, kwds))

                    args, kwds = mkargs(P, p_sig)
                    try:
                        getattr(p_type(9), attr)(*args, **kwds)
                    except Exception:
                        raise TestFailed("invalid signature for %s: %s %s" % (p_func, args, kwds))

        doit('Decimal')
        doit('Context')


all_tests = [
  CExplicitConstructionTest, PyExplicitConstructionTest,
  CImplicitConstructionTest, PyImplicitConstructionTest,
  CFormatTest,               PyFormatTest,
  CArithmeticOperatorsTest,  PyArithmeticOperatorsTest,
  CThreadingTest,            PyThreadingTest,
  CUsabilityTest,            PyUsabilityTest,
  CPythonAPItests,           PyPythonAPItests,
  CContextAPItests,          PyContextAPItests,
  CContextWithStatement,     PyContextWithStatement,
  CContextFlags,             PyContextFlags,
  CSpecialContexts,          PySpecialContexts,
  CContextInputValidation,   PyContextInputValidation,
  CContextSubclassing,       PyContextSubclassing,
  CCoverage,                 PyCoverage,
  CFunctionality,            PyFunctionality,
  CWhitebox,                 PyWhitebox,
  CIBMTestCases,             PyIBMTestCases,
]

# Delete C tests if _decimal.so is not present.
if not C:
    all_tests = all_tests[1::2]
else:
    all_tests.insert(0, CheckAttributes)
    all_tests.insert(1, SignatureTest)


def test_main(arith=None, verbose=None, todo_tests=None, debug=None):
    """ Execute the tests.

    Runs all arithmetic tests if arith is True or if the "decimal" resource
    is enabled in regrtest.py
    """

    init(C)
    init(P)
    global TEST_ALL, DEBUG
    TEST_ALL = arith if arith is not None else is_resource_enabled('decimal')
    DEBUG = debug

    if todo_tests is None:
        test_classes = all_tests
    else:
        test_classes = [CIBMTestCases, PyIBMTestCases]

    # Dynamically build custom test definition for each file in the test
    # directory and add the definitions to the DecimalTest class.  This
    # procedure insures that new files do not get skipped.
    for filename in os.listdir(directory):
        if '.decTest' not in filename or filename.startswith("."):
            continue
        head, tail = filename.split('.')
        if todo_tests is not None and head not in todo_tests:
            continue
        tester = lambda self, f=filename: self.eval_file(directory + f)
        setattr(CIBMTestCases, 'test_' + head, tester)
        setattr(PyIBMTestCases, 'test_' + head, tester)
        del filename, head, tail, tester


    try:
        run_unittest(*test_classes)
        if todo_tests is None:
            from doctest import IGNORE_EXCEPTION_DETAIL
            savedecimal = sys.modules['decimal']
            if C:
                sys.modules['decimal'] = C
                run_doctest(C, verbose, optionflags=IGNORE_EXCEPTION_DETAIL)
            sys.modules['decimal'] = P
            run_doctest(P, verbose)
            sys.modules['decimal'] = savedecimal
    finally:
        if C: C.setcontext(ORIGINAL_CONTEXT[C])
        P.setcontext(ORIGINAL_CONTEXT[P])
        if not C:
            warnings.warn('C tests skipped: no module named _decimal.',
                          UserWarning)
        if not orig_sys_decimal is sys.modules['decimal']:
            raise TestFailed("Internal error: unbalanced number of changes to "
                             "sys.modules['decimal'].")


if __name__ == '__main__':
    import optparse
    p = optparse.OptionParser("test_decimal.py [--debug] [{--skip | test1 [test2 [...]]}]")
    p.add_option('--debug', '-d', action='store_true', help='shows the test number and context before each test')
    p.add_option('--skip',  '-s', action='store_true', help='skip over 90% of the arithmetic tests')
    (opt, args) = p.parse_args()

    if opt.skip:
        test_main(arith=False, verbose=True)
    elif args:
        test_main(arith=True, verbose=True, todo_tests=args, debug=opt.debug)
    else:
        test_main(arith=True, verbose=True)
class PyCoverage(Coverage):
    decimal = P

    def setUp(self):
        super().setUp()
        self._previous_int_limit = sys.get_int_max_str_digits()
        sys.set_int_max_str_digits(7000)

    def tearDown(self):
        sys.set_int_max_str_digits(self._previous_int_limit)
        super().tearDown()

class PyFunctionality(unittest.TestCase):
    """Extra functionality in decimal.py"""

    def test_py_alternate_formatting(self):
        # triples giving a format, a Decimal, and the expected result
        Decimal = P.Decimal
        localcontext = P.localcontext

        test_values = [
            # Issue 7094: Alternate formatting (specified by #)
            ('.0e', '1.0', '1e+0'),
            ('#.0e', '1.0', '1.e+0'),
            ('.0f', '1.0', '1'),
            ('#.0f', '1.0', '1.'),
            ('g', '1.1', '1.1'),
            ('#g', '1.1', '1.1'),
            ('.0g', '1', '1'),
            ('#.0g', '1', '1.'),
            ('.0%', '1.0', '100%'),
            ('#.0%', '1.0', '100.%'),
            ]
        for fmt, d, result in test_values:
            self.assertEqual(format(Decimal(d), fmt), result)

class PyWhitebox(unittest.TestCase):
    """White box testing for decimal.py"""

    def test_py_exact_power(self):
        # Rarely exercised lines in _power_exact.
        Decimal = P.Decimal
        localcontext = P.localcontext

        with localcontext() as c:
            c.prec = 8
            x = Decimal(2**16) ** Decimal("-0.5")
            self.assertEqual(x, Decimal('0.00390625'))

            x = Decimal(2**16) ** Decimal("-0.6")
            self.assertEqual(x, Decimal('0.0012885819'))

            x = Decimal("256e7") ** Decimal("-0.5")

            x = Decimal(152587890625) ** Decimal('-0.0625')
            self.assertEqual(x, Decimal("0.2"))

            x = Decimal("152587890625e7") ** Decimal('-0.0625')

            x = Decimal(5**2659) ** Decimal('-0.0625')

            c.prec = 1
            x = Decimal("152587890625") ** Decimal('-0.5')
            c.prec = 201
            x = Decimal(2**578) ** Decimal("-0.5")

    def test_py_immutability_operations(self):
        # Do operations and check that it didn't change internal objects.
        Decimal = P.Decimal
        DefaultContext = P.DefaultContext
        setcontext = P.setcontext

        c = DefaultContext.copy()
        c.traps = dict((s, 0) for s in OrderedSignals[P])
        setcontext(c)

        d1 = Decimal('-25e55')
        b1 = Decimal('-25e55')
        d2 = Decimal('33e+33')
        b2 = Decimal('33e+33')

        def checkSameDec(operation, useOther=False):
            if useOther:
                eval("d1." + operation + "(d2)")
                self.assertEqual(d1._sign, b1._sign)
                self.assertEqual(d1._int, b1._int)
                self.assertEqual(d1._exp, b1._exp)
                self.assertEqual(d2._sign, b2._sign)
                self.assertEqual(d2._int, b2._int)
                self.assertEqual(d2._exp, b2._exp)
            else:
                eval("d1." + operation + "()")
                self.assertEqual(d1._sign, b1._sign)
                self.assertEqual(d1._int, b1._int)
                self.assertEqual(d1._exp, b1._exp)

        Decimal(d1)
        self.assertEqual(d1._sign, b1._sign)
        self.assertEqual(d1._int, b1._int)
        self.assertEqual(d1._exp, b1._exp)

        checkSameDec("__abs__")
        checkSameDec("__add__", True)
        checkSameDec("__divmod__", True)
        checkSameDec("__eq__", True)
        checkSameDec("__ne__", True)
        checkSameDec("__le__", True)
        checkSameDec("__lt__", True)
        checkSameDec("__ge__", True)
        checkSameDec("__gt__", True)
        checkSameDec("__float__")
        checkSameDec("__floordiv__", True)
        checkSameDec("__hash__")
        checkSameDec("__int__")
        checkSameDec("__trunc__")
        checkSameDec("__mod__", True)
        checkSameDec("__mul__", True)
        checkSameDec("__neg__")
        checkSameDec("__bool__")
        checkSameDec("__pos__")
        checkSameDec("__pow__", True)
        checkSameDec("__radd__", True)
        checkSameDec("__rdivmod__", True)
        checkSameDec("__repr__")
        checkSameDec("__rfloordiv__", True)
        checkSameDec("__rmod__", True)
        checkSameDec("__rmul__", True)
        checkSameDec("__rpow__", True)
        checkSameDec("__rsub__", True)
        checkSameDec("__str__")
        checkSameDec("__sub__", True)
        checkSameDec("__truediv__", True)
        checkSameDec("adjusted")
        checkSameDec("as_tuple")
        checkSameDec("compare", True)
        checkSameDec("max", True)
        checkSameDec("min", True)
        checkSameDec("normalize")
        checkSameDec("quantize", True)
        checkSameDec("remainder_near", True)
        checkSameDec("same_quantum", True)
        checkSameDec("sqrt")
        checkSameDec("to_eng_string")
        checkSameDec("to_integral")

    def test_py_decimal_id(self):
        Decimal = P.Decimal

        d = Decimal(45)
        e = Decimal(d)
        self.assertEqual(str(e), '45')
        self.assertNotEqual(id(d), id(e))

    def test_py_rescale(self):
        # Coverage
        Decimal = P.Decimal
        localcontext = P.localcontext

        with localcontext() as c:
            x = Decimal("NaN")._rescale(3, ROUND_UP)
            self.assertTrue(x.is_nan())

    def test_py__round(self):
        # Coverage
        Decimal = P.Decimal

        self.assertRaises(ValueError, Decimal("3.1234")._round, 0, ROUND_UP)

class CFunctionality(unittest.TestCase):
    """Extra functionality in _decimal"""

    @requires_extra_functionality
    def test_c_ieee_context(self):
        # issue 8786: Add support for IEEE 754 contexts to decimal module.
        IEEEContext = C.IEEEContext
        DECIMAL32 = C.DECIMAL32
        DECIMAL64 = C.DECIMAL64
        DECIMAL128 = C.DECIMAL128

        def assert_rest(self, context):
            self.assertEqual(context.clamp, 1)
            assert_signals(self, context, 'traps', [])
            assert_signals(self, context, 'flags', [])

        c = IEEEContext(DECIMAL32)
        self.assertEqual(c.prec, 7)
        self.assertEqual(c.Emax, 96)
        self.assertEqual(c.Emin, -95)
        assert_rest(self, c)

        c = IEEEContext(DECIMAL64)
        self.assertEqual(c.prec, 16)
        self.assertEqual(c.Emax, 384)
        self.assertEqual(c.Emin, -383)
        assert_rest(self, c)

        c = IEEEContext(DECIMAL128)
        self.assertEqual(c.prec, 34)
        self.assertEqual(c.Emax, 6144)
        self.assertEqual(c.Emin, -6143)
        assert_rest(self, c)

        # Invalid values
        self.assertRaises(OverflowError, IEEEContext, 2**63)
        self.assertRaises(ValueError, IEEEContext, -1)
        self.assertRaises(ValueError, IEEEContext, 1024)

    @requires_extra_functionality
    def test_c_context(self):
        Context = C.Context

        c = Context(flags=C.DecClamped, traps=C.DecRounded)
        self.assertEqual(c._flags, C.DecClamped)
        self.assertEqual(c._traps, C.DecRounded)

    @requires_extra_functionality
    def test_constants(self):
        # Condition flags
        cond = (
            C.DecClamped, C.DecConversionSyntax, C.DecDivisionByZero,
            C.DecDivisionImpossible, C.DecDivisionUndefined,
            C.DecFpuError, C.DecInexact, C.DecInvalidContext,
            C.DecInvalidOperation, C.DecMallocError,
            C.DecFloatOperation, C.DecOverflow, C.DecRounded,
            C.DecSubnormal, C.DecUnderflow
        )

        # IEEEContext
        self.assertEqual(C.DECIMAL32, 32)
        self.assertEqual(C.DECIMAL64, 64)
        self.assertEqual(C.DECIMAL128, 128)
        self.assertEqual(C.IEEE_CONTEXT_MAX_BITS, 512)

        # Conditions
        for i, v in enumerate(cond):
            self.assertEqual(v, 1<<i)

        self.assertEqual(C.DecIEEEInvalidOperation,
                         C.DecConversionSyntax|
                         C.DecDivisionImpossible|
                         C.DecDivisionUndefined|
                         C.DecFpuError|
                         C.DecInvalidContext|
                         C.DecInvalidOperation|
                         C.DecMallocError)

        self.assertEqual(C.DecErrors,
                         C.DecIEEEInvalidOperation|
                         C.DecDivisionByZero)

        self.assertEqual(C.DecTraps,
                         C.DecErrors|C.DecOverflow|C.DecUnderflow)

class CWhitebox(unittest.TestCase):
    """Whitebox testing for _decimal"""

    def test_bignum(self):
        # Not exactly whitebox, but too slow with pydecimal.

        Decimal = C.Decimal
        localcontext = C.localcontext

        b1 = 10**35
        b2 = 10**36
        with localcontext() as c:
            c.prec = 1000000
            for i in range(5):
                a = random.randrange(b1, b2)
                b = random.randrange(1000, 1200)
                x = a ** b
                y = Decimal(a) ** Decimal(b)
                self.assertEqual(x, y)

    def test_invalid_construction(self):
        self.assertRaises(TypeError, C.Decimal, 9, "xyz")

    def test_c_input_restriction(self):
        # Too large for _decimal to be converted exactly
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        Context = C.Context
        localcontext = C.localcontext

        with localcontext(Context()):
            self.assertRaises(InvalidOperation, Decimal,
                              "1e9999999999999999999")

    def test_c_context_repr(self):
        # This test is _decimal-only because flags are not printed
        # in the same order.
        DefaultContext = C.DefaultContext
        FloatOperation = C.FloatOperation

        c = DefaultContext.copy()

        c.prec = 425000000
        c.Emax = 425000000
        c.Emin = -425000000
        c.rounding = ROUND_HALF_DOWN
        c.capitals = 0
        c.clamp = 1
        for sig in OrderedSignals[C]:
            c.flags[sig] = True
            c.traps[sig] = True
        c.flags[FloatOperation] = True
        c.traps[FloatOperation] = True

        s = c.__repr__()
        t = "Context(prec=425000000, rounding=ROUND_HALF_DOWN, " \
            "Emin=-425000000, Emax=425000000, capitals=0, clamp=1, " \
            "flags=[Clamped, InvalidOperation, DivisionByZero, Inexact, " \
                   "FloatOperation, Overflow, Rounded, Subnormal, Underflow], " \
            "traps=[Clamped, InvalidOperation, DivisionByZero, Inexact, " \
                   "FloatOperation, Overflow, Rounded, Subnormal, Underflow])"
        self.assertEqual(s, t)

    def test_c_context_errors(self):
        Context = C.Context
        InvalidOperation = C.InvalidOperation
        Overflow = C.Overflow
        FloatOperation = C.FloatOperation
        localcontext = C.localcontext
        getcontext = C.getcontext
        setcontext = C.setcontext
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        c = Context()

        # SignalDict: input validation
        self.assertRaises(KeyError, c.flags.__setitem__, 801, 0)
        self.assertRaises(KeyError, c.traps.__setitem__, 801, 0)
        self.assertRaises(ValueError, c.flags.__delitem__, Overflow)
        self.assertRaises(ValueError, c.traps.__delitem__, InvalidOperation)
        self.assertRaises(TypeError, setattr, c, 'flags', ['x'])
        self.assertRaises(TypeError, setattr, c,'traps', ['y'])
        self.assertRaises(KeyError, setattr, c, 'flags', {0:1})
        self.assertRaises(KeyError, setattr, c, 'traps', {0:1})

        # Test assignment from a signal dict with the correct length but
        # one invalid key.
        d = c.flags.copy()
        del d[FloatOperation]
        d["XYZ"] = 91283719
        self.assertRaises(KeyError, setattr, c, 'flags', d)
        self.assertRaises(KeyError, setattr, c, 'traps', d)

        # Input corner cases
        int_max = 2**63-1 if HAVE_CONFIG_64 else 2**31-1
        gt_max_emax = 10**18 if HAVE_CONFIG_64 else 10**9

        # prec, Emax, Emin
        for attr in ['prec', 'Emax']:
            self.assertRaises(ValueError, setattr, c, attr, gt_max_emax)
        self.assertRaises(ValueError, setattr, c, 'Emin', -gt_max_emax)

        # prec, Emax, Emin in context constructor
        self.assertRaises(ValueError, Context, prec=gt_max_emax)
        self.assertRaises(ValueError, Context, Emax=gt_max_emax)
        self.assertRaises(ValueError, Context, Emin=-gt_max_emax)

        # Overflow in conversion
        self.assertRaises(OverflowError, Context, prec=int_max+1)
        self.assertRaises(OverflowError, Context, Emax=int_max+1)
        self.assertRaises(OverflowError, Context, Emin=-int_max-2)
        self.assertRaises(OverflowError, Context, clamp=int_max+1)
        self.assertRaises(OverflowError, Context, capitals=int_max+1)

        # OverflowError, general ValueError
        for attr in ('prec', 'Emin', 'Emax', 'capitals', 'clamp'):
            self.assertRaises(OverflowError, setattr, c, attr, int_max+1)
            self.assertRaises(OverflowError, setattr, c, attr, -int_max-2)
            if sys.platform != 'win32':
                self.assertRaises(ValueError, setattr, c, attr, int_max)
                self.assertRaises(ValueError, setattr, c, attr, -int_max-1)

        # OverflowError: _unsafe_setprec, _unsafe_setemin, _unsafe_setemax
        if C.MAX_PREC == 425000000:
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setprec'),
                              int_max+1)
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setemax'),
                              int_max+1)
            self.assertRaises(OverflowError, getattr(c, '_unsafe_setemin'),
                              -int_max-2)

        # ValueError: _unsafe_setprec, _unsafe_setemin, _unsafe_setemax
        if C.MAX_PREC == 425000000:
            self.assertRaises(ValueError, getattr(c, '_unsafe_setprec'), 0)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setprec'),
                              1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemax'), -1)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemax'),
                              1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemin'),
                              -1070000001)
            self.assertRaises(ValueError, getattr(c, '_unsafe_setemin'), 1)

        # capitals, clamp
        for attr in ['capitals', 'clamp']:
            self.assertRaises(ValueError, setattr, c, attr, -1)
            self.assertRaises(ValueError, setattr, c, attr, 2)
            self.assertRaises(TypeError, setattr, c, attr, [1,2,3])
            if HAVE_CONFIG_64:
                self.assertRaises(ValueError, setattr, c, attr, 2**32)
                self.assertRaises(ValueError, setattr, c, attr, 2**32+1)

        # Invalid local context
        self.assertRaises(TypeError, exec, 'with localcontext("xyz"): pass',
                          locals())
        self.assertRaises(TypeError, exec,
                          'with localcontext(context=getcontext()): pass',
                          locals())

        # setcontext
        saved_context = getcontext()
        self.assertRaises(TypeError, setcontext, "xyz")
        setcontext(saved_context)

    def test_rounding_strings_interned(self):

        self.assertIs(C.ROUND_UP, P.ROUND_UP)
        self.assertIs(C.ROUND_DOWN, P.ROUND_DOWN)
        self.assertIs(C.ROUND_CEILING, P.ROUND_CEILING)
        self.assertIs(C.ROUND_FLOOR, P.ROUND_FLOOR)
        self.assertIs(C.ROUND_HALF_UP, P.ROUND_HALF_UP)
        self.assertIs(C.ROUND_HALF_DOWN, P.ROUND_HALF_DOWN)
        self.assertIs(C.ROUND_HALF_EVEN, P.ROUND_HALF_EVEN)
        self.assertIs(C.ROUND_05UP, P.ROUND_05UP)

    @requires_extra_functionality
    def test_c_context_errors_extra(self):
        Context = C.Context
        InvalidOperation = C.InvalidOperation
        Overflow = C.Overflow
        localcontext = C.localcontext
        getcontext = C.getcontext
        setcontext = C.setcontext
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        c = Context()

        # Input corner cases
        int_max = 2**63-1 if HAVE_CONFIG_64 else 2**31-1

        # OverflowError, general ValueError
        self.assertRaises(OverflowError, setattr, c, '_allcr', int_max+1)
        self.assertRaises(OverflowError, setattr, c, '_allcr', -int_max-2)
        if sys.platform != 'win32':
            self.assertRaises(ValueError, setattr, c, '_allcr', int_max)
            self.assertRaises(ValueError, setattr, c, '_allcr', -int_max-1)

        # OverflowError, general TypeError
        for attr in ('_flags', '_traps'):
            self.assertRaises(OverflowError, setattr, c, attr, int_max+1)
            self.assertRaises(OverflowError, setattr, c, attr, -int_max-2)
            if sys.platform != 'win32':
                self.assertRaises(TypeError, setattr, c, attr, int_max)
                self.assertRaises(TypeError, setattr, c, attr, -int_max-1)

        # _allcr
        self.assertRaises(ValueError, setattr, c, '_allcr', -1)
        self.assertRaises(ValueError, setattr, c, '_allcr', 2)
        self.assertRaises(TypeError, setattr, c, '_allcr', [1,2,3])
        if HAVE_CONFIG_64:
            self.assertRaises(ValueError, setattr, c, '_allcr', 2**32)
            self.assertRaises(ValueError, setattr, c, '_allcr', 2**32+1)

        # _flags, _traps
        for attr in ['_flags', '_traps']:
            self.assertRaises(TypeError, setattr, c, attr, 999999)
            self.assertRaises(TypeError, setattr, c, attr, 'x')

    def test_c_valid_context(self):
        # These tests are for code coverage in _decimal.
        DefaultContext = C.DefaultContext
        Clamped = C.Clamped
        Underflow = C.Underflow
        Inexact = C.Inexact
        Rounded = C.Rounded
        Subnormal = C.Subnormal

        c = DefaultContext.copy()

        # Exercise all getters and setters
        c.prec = 34
        c.rounding = ROUND_HALF_UP
        c.Emax = 3000
        c.Emin = -3000
        c.capitals = 1
        c.clamp = 0

        self.assertEqual(c.prec, 34)
        self.assertEqual(c.rounding, ROUND_HALF_UP)
        self.assertEqual(c.Emin, -3000)
        self.assertEqual(c.Emax, 3000)
        self.assertEqual(c.capitals, 1)
        self.assertEqual(c.clamp, 0)

        self.assertEqual(c.Etiny(), -3033)
        self.assertEqual(c.Etop(), 2967)

        # Exercise all unsafe setters
        if C.MAX_PREC == 425000000:
            c._unsafe_setprec(999999999)
            c._unsafe_setemax(999999999)
            c._unsafe_setemin(-999999999)
            self.assertEqual(c.prec, 999999999)
            self.assertEqual(c.Emax, 999999999)
            self.assertEqual(c.Emin, -999999999)

    @requires_extra_functionality
    def test_c_valid_context_extra(self):
        DefaultContext = C.DefaultContext

        c = DefaultContext.copy()
        self.assertEqual(c._allcr, 1)
        c._allcr = 0
        self.assertEqual(c._allcr, 0)

    def test_c_round(self):
        # Restricted input.
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        localcontext = C.localcontext
        MAX_EMAX = C.MAX_EMAX
        MIN_ETINY = C.MIN_ETINY
        int_max = 2**63-1 if C.MAX_PREC > 425000000 else 2**31-1

        with localcontext() as c:
            c.traps[InvalidOperation] = True
            self.assertRaises(InvalidOperation, Decimal("1.23").__round__,
                              -int_max-1)
            self.assertRaises(InvalidOperation, Decimal("1.23").__round__,
                              int_max)
            self.assertRaises(InvalidOperation, Decimal("1").__round__,
                              int(MAX_EMAX+1))
            self.assertRaises(C.InvalidOperation, Decimal("1").__round__,
                              -int(MIN_ETINY-1))
            self.assertRaises(OverflowError, Decimal("1.23").__round__,
                              -int_max-2)
            self.assertRaises(OverflowError, Decimal("1.23").__round__,
                              int_max+1)

    def test_c_format(self):
        # Restricted input
        Decimal = C.Decimal
        HAVE_CONFIG_64 = (C.MAX_PREC > 425000000)

        self.assertRaises(TypeError, Decimal(1).__format__, "=10.10", [], 9)
        self.assertRaises(TypeError, Decimal(1).__format__, "=10.10", 9)
        self.assertRaises(TypeError, Decimal(1).__format__, [])

        self.assertRaises(ValueError, Decimal(1).__format__, "<>=10.10")
        maxsize = 2**63-1 if HAVE_CONFIG_64 else 2**31-1
        self.assertRaises(ValueError, Decimal("1.23456789").__format__,
                          "=%d.1" % maxsize)

    def test_c_integral(self):
        Decimal = C.Decimal
        Inexact = C.Inexact
        localcontext = C.localcontext

        x = Decimal(10)
        self.assertEqual(x.to_integral(), 10)
        self.assertRaises(TypeError, x.to_integral, '10')
        self.assertRaises(TypeError, x.to_integral, 10, 'x')
        self.assertRaises(TypeError, x.to_integral, 10)

        self.assertEqual(x.to_integral_value(), 10)
        self.assertRaises(TypeError, x.to_integral_value, '10')
        self.assertRaises(TypeError, x.to_integral_value, 10, 'x')
        self.assertRaises(TypeError, x.to_integral_value, 10)

        self.assertEqual(x.to_integral_exact(), 10)
        self.assertRaises(TypeError, x.to_integral_exact, '10')
        self.assertRaises(TypeError, x.to_integral_exact, 10, 'x')
        self.assertRaises(TypeError, x.to_integral_exact, 10)

        with localcontext() as c:
            x = Decimal("99999999999999999999999999.9").to_integral_value(ROUND_UP)
            self.assertEqual(x, Decimal('100000000000000000000000000'))

            x = Decimal("99999999999999999999999999.9").to_integral_exact(ROUND_UP)
            self.assertEqual(x, Decimal('100000000000000000000000000'))

            c.traps[Inexact] = True
            self.assertRaises(Inexact, Decimal("999.9").to_integral_exact, ROUND_UP)

    def test_c_funcs(self):
        # Invalid arguments
        Decimal = C.Decimal
        InvalidOperation = C.InvalidOperation
        DivisionByZero = C.DivisionByZero
        getcontext = C.getcontext
        localcontext = C.localcontext

        self.assertEqual(Decimal('9.99e10').to_eng_string(), '99.9E+9')

        self.assertRaises(TypeError, pow, Decimal(1), 2, "3")
        self.assertRaises(TypeError, Decimal(9).number_class, "x", "y")
        self.assertRaises(TypeError, Decimal(9).same_quantum, 3, "x", "y")

        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), []
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), getcontext()
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), 10
        )
        self.assertRaises(
            TypeError,
            Decimal("1.23456789").quantize, Decimal('1e-100000'), ROUND_UP, 1000
        )

        with localcontext() as c:
            c.clear_traps()

            # Invalid arguments
            self.assertRaises(TypeError, c.copy_sign, Decimal(1), "x", "y")
            self.assertRaises(TypeError, c.canonical, 200)
            self.assertRaises(TypeError, c.is_canonical, 200)
            self.assertRaises(TypeError, c.divmod, 9, 8, "x", "y")
            self.assertRaises(TypeError, c.same_quantum, 9, 3, "x", "y")

            self.assertEqual(str(c.canonical(Decimal(200))), '200')
            self.assertEqual(c.radix(), 10)

            c.traps[DivisionByZero] = True
            self.assertRaises(DivisionByZero, Decimal(9).__divmod__, 0)
            self.assertRaises(DivisionByZero, c.divmod, 9, 0)
            self.assertTrue(c.flags[InvalidOperation])

            c.clear_flags()
            c.traps[InvalidOperation] = True
            self.assertRaises(InvalidOperation, Decimal(9).__divmod__, 0)
            self.assertRaises(InvalidOperation, c.divmod, 9, 0)
            self.assertTrue(c.flags[DivisionByZero])

            c.traps[InvalidOperation] = True
            c.prec = 2
            self.assertRaises(InvalidOperation, pow, Decimal(1000), 1, 501)

    def test_va_args_exceptions(self):
        Decimal = C.Decimal
        Context = C.Context

        x = Decimal("10001111111")

        for attr in ['exp', 'is_normal', 'is_subnormal', 'ln', 'log10',
                     'logb', 'logical_invert', 'next_minus', 'next_plus',
                     'normalize', 'number_class', 'sqrt', 'to_eng_string']:
            func = getattr(x, attr)
            self.assertRaises(TypeError, func, context="x")
            self.assertRaises(TypeError, func, "x", context=None)

        for attr in ['compare', 'compare_signal', 'logical_and',
                     'logical_or', 'max', 'max_mag', 'min', 'min_mag',
                     'remainder_near', 'rotate', 'scaleb', 'shift']:
            func = getattr(x, attr)
            self.assertRaises(TypeError, func, context="x")
            self.assertRaises(TypeError, func, "x", context=None)

        self.assertRaises(TypeError, x.to_integral, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral, [], [])

        self.assertRaises(TypeError, x.to_integral_value, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral_value, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral_value, [], [])

        self.assertRaises(TypeError, x.to_integral_exact, rounding=None, context=[])
        self.assertRaises(TypeError, x.to_integral_exact, rounding={}, context=[])
        self.assertRaises(TypeError, x.to_integral_exact, [], [])

        self.assertRaises(TypeError, x.fma, 1, 2, context="x")
        self.assertRaises(TypeError, x.fma, 1, 2, "x", context=None)

        self.assertRaises(TypeError, x.quantize, 1, [], context=None)
        self.assertRaises(TypeError, x.quantize, 1, [], rounding=None)
        self.assertRaises(TypeError, x.quantize, 1, [], [])

        c = Context()
        self.assertRaises(TypeError, c.power, 1, 2, mod="x")
        self.assertRaises(TypeError, c.power, 1, "x", mod=None)
        self.assertRaises(TypeError, c.power, "x", 2, mod=None)

    @requires_extra_functionality
    def test_c_context_templates(self):
        self.assertEqual(
            C.BasicContext._traps,
            C.DecIEEEInvalidOperation|C.DecDivisionByZero|C.DecOverflow|
            C.DecUnderflow|C.DecClamped
        )
        self.assertEqual(
            C.DefaultContext._traps,
            C.DecIEEEInvalidOperation|C.DecDivisionByZero|C.DecOverflow
        )

    @requires_extra_functionality
    def test_c_signal_dict(self):

        # SignalDict coverage
        Context = C.Context
        DefaultContext = C.DefaultContext

        InvalidOperation = C.InvalidOperation
        FloatOperation = C.FloatOperation
        DivisionByZero = C.DivisionByZero
        Overflow = C.Overflow
        Subnormal = C.Subnormal
        Underflow = C.Underflow
        Rounded = C.Rounded
        Inexact = C.Inexact
        Clamped = C.Clamped

        DecClamped = C.DecClamped
        DecInvalidOperation = C.DecInvalidOperation
        DecIEEEInvalidOperation = C.DecIEEEInvalidOperation

        def assertIsExclusivelySet(signal, signal_dict):
            for sig in signal_dict:
                if sig == signal:
                    self.assertTrue(signal_dict[sig])
                else:
                    self.assertFalse(signal_dict[sig])

        c = DefaultContext.copy()

        # Signal dict methods
        self.assertTrue(Overflow in c.traps)
        c.clear_traps()
        for k in c.traps.keys():
            c.traps[k] = True
        for v in c.traps.values():
            self.assertTrue(v)
        c.clear_traps()
        for k, v in c.traps.items():
            self.assertFalse(v)

        self.assertFalse(c.flags.get(Overflow))
        self.assertIs(c.flags.get("x"), None)
        self.assertEqual(c.flags.get("x", "y"), "y")
        self.assertRaises(TypeError, c.flags.get, "x", "y", "z")

        self.assertEqual(len(c.flags), len(c.traps))
        s = sys.getsizeof(c.flags)
        s = sys.getsizeof(c.traps)
        s = c.flags.__repr__()

        # Set flags/traps.
        c.clear_flags()
        c._flags = DecClamped
        self.assertTrue(c.flags[Clamped])

        c.clear_traps()
        c._traps = DecInvalidOperation
        self.assertTrue(c.traps[InvalidOperation])

        # Set flags/traps from dictionary.
        c.clear_flags()
        d = c.flags.copy()
        d[DivisionByZero] = True
        c.flags = d
        assertIsExclusivelySet(DivisionByZero, c.flags)

        c.clear_traps()
        d = c.traps.copy()
        d[Underflow] = True
        c.traps = d
        assertIsExclusivelySet(Underflow, c.traps)

        # Random constructors
        IntSignals = {
          Clamped: C.DecClamped,
          Rounded: C.DecRounded,
          Inexact: C.DecInexact,
          Subnormal: C.DecSubnormal,
          Underflow: C.DecUnderflow,
          Overflow: C.DecOverflow,
          DivisionByZero: C.DecDivisionByZero,
          FloatOperation: C.DecFloatOperation,
          InvalidOperation: C.DecIEEEInvalidOperation
        }
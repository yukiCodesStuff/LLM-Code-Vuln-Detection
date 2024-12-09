        self.assertEqual(eval("0o777"), 511)
        self.assertEqual(eval("-0o0000010"), -8)

    def test_int_literals_too_long(self):
        n = 3000
        source = f"a = 1\nb = 2\nc = {'3'*n}\nd = 4"
        with support.adjust_int_max_str_digits(n):
            compile(source, "<long_int_pass>", "exec")  # no errors.
        with support.adjust_int_max_str_digits(n-1):
            with self.assertRaises(SyntaxError) as err_ctx:
                compile(source, "<long_int_fail>", "exec")
            exc = err_ctx.exception
            self.assertEqual(exc.lineno, 3)
            self.assertIn('Exceeds the limit ', str(exc))
            self.assertIn(' Consider hexadecimal ', str(exc))

    def test_unary_minus(self):
        # Verify treatment of unary minus on negative numbers SF bug #660455
        if sys.maxsize == 2147483647:
            # 32-bit machine
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
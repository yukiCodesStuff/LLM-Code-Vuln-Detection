        self.assertRaises(ValueError, ast.literal_eval, '+True')
        self.assertRaises(ValueError, ast.literal_eval, '2+3')

    def test_literal_eval_complex(self):
        # Issue #4907
        self.assertEqual(ast.literal_eval('6j'), 6j)
        self.assertEqual(ast.literal_eval('-6j'), -6j)
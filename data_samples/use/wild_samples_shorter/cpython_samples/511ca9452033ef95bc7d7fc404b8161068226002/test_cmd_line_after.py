        self.assertTrue(proc.stderr.startswith(err_msg), proc.stderr)
        self.assertNotEqual(proc.returncode, 0)

    def test_int_max_str_digits(self):
        code = "import sys; print(sys.flags.int_max_str_digits, sys.get_int_max_str_digits())"

        assert_python_failure('-X', 'int_max_str_digits', '-c', code)
        assert_python_failure('-X', 'int_max_str_digits=foo', '-c', code)
        assert_python_failure('-X', 'int_max_str_digits=100', '-c', code)

        assert_python_failure('-c', code, PYTHONINTMAXSTRDIGITS='foo')
        assert_python_failure('-c', code, PYTHONINTMAXSTRDIGITS='100')

        def res2int(res):
            out = res.out.strip().decode("utf-8")
            return tuple(int(i) for i in out.split())

        res = assert_python_ok('-c', code)
        self.assertEqual(res2int(res), (-1, sys.get_int_max_str_digits()))
        res = assert_python_ok('-X', 'int_max_str_digits=0', '-c', code)
        self.assertEqual(res2int(res), (0, 0))
        res = assert_python_ok('-X', 'int_max_str_digits=4000', '-c', code)
        self.assertEqual(res2int(res), (4000, 4000))
        res = assert_python_ok('-X', 'int_max_str_digits=100000', '-c', code)
        self.assertEqual(res2int(res), (100000, 100000))

        res = assert_python_ok('-c', code, PYTHONINTMAXSTRDIGITS='0')
        self.assertEqual(res2int(res), (0, 0))
        res = assert_python_ok('-c', code, PYTHONINTMAXSTRDIGITS='4000')
        self.assertEqual(res2int(res), (4000, 4000))
        res = assert_python_ok(
            '-X', 'int_max_str_digits=6000', '-c', code,
            PYTHONINTMAXSTRDIGITS='4000'
        )
        self.assertEqual(res2int(res), (6000, 6000))


@unittest.skipIf(interpreter_requires_environment(),
                 'Cannot run -I tests when PYTHON env vars are required.')
class IgnoreEnvironmentTest(unittest.TestCase):
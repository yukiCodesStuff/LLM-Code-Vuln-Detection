    def test_parsing_error(self):
        args = [sys.executable, '-I', '--unknown-option']
        proc = subprocess.run(args,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              text=True)
        err_msg = "unknown option --unknown-option\nusage: "
        self.assertTrue(proc.stderr.startswith(err_msg), proc.stderr)
        self.assertNotEqual(proc.returncode, 0)


@unittest.skipIf(interpreter_requires_environment(),
                 'Cannot run -I tests when PYTHON env vars are required.')
class IgnoreEnvironmentTest(unittest.TestCase):

    def run_ignoring_vars(self, predicate, **env_vars):
        # Runs a subprocess with -E set, even though we're passing
        # specific environment variables
        # Logical inversion to match predicate check to a zero return
        # code indicating success
        code = "import sys; sys.stderr.write(str(sys.flags)); sys.exit(not ({}))".format(predicate)
        return assert_python_ok('-E', '-c', code, **env_vars)

    def test_ignore_PYTHONPATH(self):
        path = "should_be_ignored"
        self.run_ignoring_vars("'{}' not in sys.path".format(path),
                               PYTHONPATH=path)

    def test_ignore_PYTHONHASHSEED(self):
        self.run_ignoring_vars("sys.flags.hash_randomization == 1",
                               PYTHONHASHSEED="0")

    def test_sys_flags_not_set(self):
        # Issue 31845: a startup refactoring broke reading flags from env vars
        expected_outcome = """
            (sys.flags.debug == sys.flags.optimize ==
             sys.flags.dont_write_bytecode ==
             sys.flags.verbose == sys.flags.safe_path == 0)
        """
        self.run_ignoring_vars(
            expected_outcome,
            PYTHONDEBUG="1",
            PYTHONOPTIMIZE="1",
            PYTHONDONTWRITEBYTECODE="1",
            PYTHONVERBOSE="1",
            PYTHONSAFEPATH="1",
        )


class SyntaxErrorTests(unittest.TestCase):
    def check_string(self, code):
        proc = subprocess.run([sys.executable, "-"], input=code,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        self.assertNotEqual(proc.returncode, 0)
        self.assertNotEqual(proc.stderr, None)
        self.assertIn(b"\nSyntaxError", proc.stderr)

    def test_tokenizer_error_with_stdin(self):
        self.check_string(b"(1+2+3")

    def test_decoding_error_at_the_end_of_the_line(self):
        self.check_string(br"'\u1f'")


def tearDownModule():
    support.reap_children()


if __name__ == "__main__":
    unittest.main()
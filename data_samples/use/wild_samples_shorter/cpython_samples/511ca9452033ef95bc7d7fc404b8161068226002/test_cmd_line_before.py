        self.assertTrue(proc.stderr.startswith(err_msg), proc.stderr)
        self.assertNotEqual(proc.returncode, 0)


@unittest.skipIf(interpreter_requires_environment(),
                 'Cannot run -I tests when PYTHON env vars are required.')
class IgnoreEnvironmentTest(unittest.TestCase):
import unittest
import unittest.mock
from test.support import requires, swap_attr
import tkinter as tk
from idlelib.idle_test.tkinter_testing_utils import run_in_tk_mainloop

from idlelib.delegator import Delegator

    @run_in_tk_mainloop()
    def test_very_long_wrapped_line(self):
        with swap_attr(self.shell, 'squeezer', None):
            self.do_input('x = ' + '1'*10_000 + '\n')
            yield
            self.assertEqual(self.get_sidebar_lines(), ['>>>'])

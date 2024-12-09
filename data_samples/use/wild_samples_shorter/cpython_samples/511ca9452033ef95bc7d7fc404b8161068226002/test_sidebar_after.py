import unittest
import unittest.mock
from test.support import requires, swap_attr
from test import support
import tkinter as tk
from idlelib.idle_test.tkinter_testing_utils import run_in_tk_mainloop

from idlelib.delegator import Delegator

    @run_in_tk_mainloop()
    def test_very_long_wrapped_line(self):
        with support.adjust_int_max_str_digits(11_111), \
                swap_attr(self.shell, 'squeezer', None):
            self.do_input('x = ' + '1'*10_000 + '\n')
            yield
            self.assertEqual(self.get_sidebar_lines(), ['>>>'])

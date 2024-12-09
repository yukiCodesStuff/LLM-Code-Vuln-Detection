class PyUsabilityTest(UsabilityTest):
    decimal = P

class PythonAPItests(unittest.TestCase):

    def test_abc(self):
        Decimal = self.decimal.Decimal
class PyCoverage(Coverage):
    decimal = P

class PyFunctionality(unittest.TestCase):
    """Extra functionality in decimal.py"""

    def test_py_alternate_formatting(self):
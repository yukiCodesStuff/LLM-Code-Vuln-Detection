from test.support import threading_helper
from test.support import warnings_helper
from test.support import asyncore
import socket
import select
import time
import enum
import gc
import os
import errno
import pprint
import urllib.request
                s.connect((HOST, server.port))


class TestEnumerations(unittest.TestCase):

    def test_tlsversion(self):
        class CheckedTLSVersion(enum.IntEnum):
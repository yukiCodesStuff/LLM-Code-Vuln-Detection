from test.support import threading_helper
from test.support import warnings_helper
from test.support import asyncore
import re
import socket
import select
import struct
import time
import enum
import gc
import http.client
import os
import errno
import pprint
import urllib.request
                s.connect((HOST, server.port))


def set_socket_so_linger_on_with_zero_timeout(sock):
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))


class TestPreHandshakeClose(unittest.TestCase):
    """Verify behavior of close sockets with received data before to the handshake.
    """

    class SingleConnectionTestServerThread(threading.Thread):

        def __init__(self, *, name, call_after_accept):
            self.call_after_accept = call_after_accept
            self.received_data = b''  # set by .run()
            self.wrap_error = None  # set by .run()
            self.listener = None  # set by .start()
            self.port = None  # set by .start()
            super().__init__(name=name)

        def __enter__(self):
            self.start()
            return self

        def __exit__(self, *args):
            try:
                if self.listener:
                    self.listener.close()
            except OSError:
                pass
            self.join()
            self.wrap_error = None  # avoid dangling references

        def start(self):
            self.ssl_ctx = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
            self.ssl_ctx.verify_mode = ssl.CERT_REQUIRED
            self.ssl_ctx.load_verify_locations(cafile=ONLYCERT)
            self.ssl_ctx.load_cert_chain(certfile=ONLYCERT, keyfile=ONLYKEY)
            self.listener = socket.socket()
            self.port = socket_helper.bind_port(self.listener)
            self.listener.settimeout(2.0)
            self.listener.listen(1)
            super().start()

        def run(self):
            conn, address = self.listener.accept()
            self.listener.close()
            with conn:
                if self.call_after_accept(conn):
                    return
                try:
                    tls_socket = self.ssl_ctx.wrap_socket(conn, server_side=True)
                except OSError as err:  # ssl.SSLError inherits from OSError
                    self.wrap_error = err
                else:
                    try:
                        self.received_data = tls_socket.recv(400)
                    except OSError:
                        pass  # closed, protocol error, etc.

    def non_linux_skip_if_other_okay_error(self, err):
        if sys.platform == "linux":
            return  # Expect the full test setup to always work on Linux.
        if (isinstance(err, ConnectionResetError) or
            (isinstance(err, OSError) and err.errno == errno.EINVAL) or
            re.search('wrong.version.number', getattr(err, "reason", ""), re.I)):
            # On Windows the TCP RST leads to a ConnectionResetError
            # (ECONNRESET) which Linux doesn't appear to surface to userspace.
            # If wrap_socket() winds up on the "if connected:" path and doing
            # the actual wrapping... we get an SSLError from OpenSSL. Typically
            # WRONG_VERSION_NUMBER. While appropriate, neither is the scenario
            # we're specifically trying to test. The way this test is written
            # is known to work on Linux. We'll skip it anywhere else that it
            # does not present as doing so.
            self.skipTest(f"Could not recreate conditions on {sys.platform}:"
                          f" {err=}")
        # If maintaining this conditional winds up being a problem.
        # just turn this into an unconditional skip anything but Linux.
        # The important thing is that our CI has the logic covered.

    def test_preauth_data_to_tls_server(self):
        server_accept_called = threading.Event()
        ready_for_server_wrap_socket = threading.Event()

        def call_after_accept(unused):
            server_accept_called.set()
            if not ready_for_server_wrap_socket.wait(2.0):
                raise RuntimeError("wrap_socket event never set, test may fail.")
            return False  # Tell the server thread to continue.

        server = self.SingleConnectionTestServerThread(
                call_after_accept=call_after_accept,
                name="preauth_data_to_tls_server")
        self.enterContext(server)  # starts it & unittest.TestCase stops it.

        with socket.socket() as client:
            client.connect(server.listener.getsockname())
            # This forces an immediate connection close via RST on .close().
            set_socket_so_linger_on_with_zero_timeout(client)
            client.setblocking(False)

            server_accept_called.wait()
            client.send(b"DELETE /data HTTP/1.0\r\n\r\n")
            client.close()  # RST

        ready_for_server_wrap_socket.set()
        server.join()
        wrap_error = server.wrap_error
        self.assertEqual(b"", server.received_data)
        self.assertIsInstance(wrap_error, OSError)  # All platforms.
        self.non_linux_skip_if_other_okay_error(wrap_error)
        self.assertIsInstance(wrap_error, ssl.SSLError)
        self.assertIn("before TLS handshake with data", wrap_error.args[1])
        self.assertIn("before TLS handshake with data", wrap_error.reason)
        self.assertNotEqual(0, wrap_error.args[0])
        self.assertIsNone(wrap_error.library, msg="attr must exist")

    def test_preauth_data_to_tls_client(self):
        client_can_continue_with_wrap_socket = threading.Event()

        def call_after_accept(conn_to_client):
            # This forces an immediate connection close via RST on .close().
            set_socket_so_linger_on_with_zero_timeout(conn_to_client)
            conn_to_client.send(
                    b"HTTP/1.0 307 Temporary Redirect\r\n"
                    b"Location: https://example.com/someone-elses-server\r\n"
                    b"\r\n")
            conn_to_client.close()  # RST
            client_can_continue_with_wrap_socket.set()
            return True  # Tell the server to stop.

        server = self.SingleConnectionTestServerThread(
                call_after_accept=call_after_accept,
                name="preauth_data_to_tls_client")
        self.enterContext(server)  # starts it & unittest.TestCase stops it.
        # Redundant; call_after_accept sets SO_LINGER on the accepted conn.
        set_socket_so_linger_on_with_zero_timeout(server.listener)

        with socket.socket() as client:
            client.connect(server.listener.getsockname())
            if not client_can_continue_with_wrap_socket.wait(2.0):
                self.fail("test server took too long.")
            ssl_ctx = ssl.create_default_context()
            try:
                tls_client = ssl_ctx.wrap_socket(
                        client, server_hostname="localhost")
            except OSError as err:  # SSLError inherits from OSError
                wrap_error = err
                received_data = b""
            else:
                wrap_error = None
                received_data = tls_client.recv(400)
                tls_client.close()

        server.join()
        self.assertEqual(b"", received_data)
        self.assertIsInstance(wrap_error, OSError)  # All platforms.
        self.non_linux_skip_if_other_okay_error(wrap_error)
        self.assertIsInstance(wrap_error, ssl.SSLError)
        self.assertIn("before TLS handshake with data", wrap_error.args[1])
        self.assertIn("before TLS handshake with data", wrap_error.reason)
        self.assertNotEqual(0, wrap_error.args[0])
        self.assertIsNone(wrap_error.library, msg="attr must exist")

    def test_https_client_non_tls_response_ignored(self):

        server_responding = threading.Event()

        class SynchronizedHTTPSConnection(http.client.HTTPSConnection):
            def connect(self):
                http.client.HTTPConnection.connect(self)
                # Wait for our fault injection server to have done its thing.
                if not server_responding.wait(1.0) and support.verbose:
                    sys.stdout.write("server_responding event never set.")
                self.sock = self._context.wrap_socket(
                        self.sock, server_hostname=self.host)

        def call_after_accept(conn_to_client):
            # This forces an immediate connection close via RST on .close().
            set_socket_so_linger_on_with_zero_timeout(conn_to_client)
            conn_to_client.send(
                    b"HTTP/1.0 402 Payment Required\r\n"
                    b"\r\n")
            conn_to_client.close()  # RST
            server_responding.set()
            return True  # Tell the server to stop.

        server = self.SingleConnectionTestServerThread(
                call_after_accept=call_after_accept,
                name="non_tls_http_RST_responder")
        self.enterContext(server)  # starts it & unittest.TestCase stops it.
        # Redundant; call_after_accept sets SO_LINGER on the accepted conn.
        set_socket_so_linger_on_with_zero_timeout(server.listener)

        connection = SynchronizedHTTPSConnection(
                f"localhost",
                port=server.port,
                context=ssl.create_default_context(),
                timeout=2.0,
        )
        # There are lots of reasons this raises as desired, long before this
        # test was added. Sending the request requires a successful TLS wrapped
        # socket; that fails if the connection is broken. It may seem pointless
        # to test this. It serves as an illustration of something that we never
        # want to happen... properly not happening.
        with self.assertRaises(OSError) as err_ctx:
            connection.request("HEAD", "/test", headers={"Host": "localhost"})
            response = connection.getresponse()


class TestEnumerations(unittest.TestCase):

    def test_tlsversion(self):
        class CheckedTLSVersion(enum.IntEnum):
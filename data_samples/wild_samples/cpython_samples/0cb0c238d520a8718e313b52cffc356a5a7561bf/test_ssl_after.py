# Test the support for SSL and sockets

import sys
import unittest
import unittest.mock
from test import support
from test.support import import_helper
from test.support import os_helper
from test.support import socket_helper
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
import threading
import traceback
import weakref
import platform
import sysconfig
import functools
try:
    import ctypes
except ImportError:
    ctypes = None


ssl = import_helper.import_module("ssl")
import _ssl

from ssl import TLSVersion, _TLSContentType, _TLSMessageType, _TLSAlertType

Py_DEBUG_WIN32 = support.Py_DEBUG and sys.platform == 'win32'

PROTOCOLS = sorted(ssl._PROTOCOL_NAMES)
HOST = socket_helper.HOST
IS_OPENSSL_3_0_0 = ssl.OPENSSL_VERSION_INFO >= (3, 0, 0)
PY_SSL_DEFAULT_CIPHERS = sysconfig.get_config_var('PY_SSL_DEFAULT_CIPHERS')

PROTOCOL_TO_TLS_VERSION = {}
for proto, ver in (
    ("PROTOCOL_SSLv3", "SSLv3"),
    ("PROTOCOL_TLSv1", "TLSv1"),
    ("PROTOCOL_TLSv1_1", "TLSv1_1"),
):
    try:
        proto = getattr(ssl, proto)
        ver = getattr(ssl.TLSVersion, ver)
    except AttributeError:
        continue
    PROTOCOL_TO_TLS_VERSION[proto] = ver

def data_file(*name):
    return os.path.join(os.path.dirname(__file__), *name)

# The custom key and certificate files used in test_ssl are generated
# using Lib/test/make_ssl_certs.py.
# Other certificates are simply fetched from the internet servers they
# are meant to authenticate.

CERTFILE = data_file("keycert.pem")
BYTES_CERTFILE = os.fsencode(CERTFILE)
ONLYCERT = data_file("ssl_cert.pem")
ONLYKEY = data_file("ssl_key.pem")
BYTES_ONLYCERT = os.fsencode(ONLYCERT)
BYTES_ONLYKEY = os.fsencode(ONLYKEY)
CERTFILE_PROTECTED = data_file("keycert.passwd.pem")
ONLYKEY_PROTECTED = data_file("ssl_key.passwd.pem")
KEY_PASSWORD = "somepass"
CAPATH = data_file("capath")
BYTES_CAPATH = os.fsencode(CAPATH)
CAFILE_NEURONIO = data_file("capath", "4e1295a3.0")
CAFILE_CACERT = data_file("capath", "5ed36f99.0")

CERTFILE_INFO = {
    'issuer': ((('countryName', 'XY'),),
               (('localityName', 'Castle Anthrax'),),
               (('organizationName', 'Python Software Foundation'),),
               (('commonName', 'localhost'),)),
    'notAfter': 'Aug 26 14:23:15 2028 GMT',
    'notBefore': 'Aug 29 14:23:15 2018 GMT',
    'serialNumber': '98A7CF88C74A32ED',
    'subject': ((('countryName', 'XY'),),
             (('localityName', 'Castle Anthrax'),),
             (('organizationName', 'Python Software Foundation'),),
             (('commonName', 'localhost'),)),
    'subjectAltName': (('DNS', 'localhost'),),
    'version': 3
}

# empty CRL
CRLFILE = data_file("revocation.crl")

# Two keys and certs signed by the same CA (for SNI tests)
SIGNED_CERTFILE = data_file("keycert3.pem")
SIGNED_CERTFILE_HOSTNAME = 'localhost'

SIGNED_CERTFILE_INFO = {
    'OCSP': ('http://testca.pythontest.net/testca/ocsp/',),
    'caIssuers': ('http://testca.pythontest.net/testca/pycacert.cer',),
    'crlDistributionPoints': ('http://testca.pythontest.net/testca/revocation.crl',),
    'issuer': ((('countryName', 'XY'),),
            (('organizationName', 'Python Software Foundation CA'),),
            (('commonName', 'our-ca-server'),)),
    'notAfter': 'Oct 28 14:23:16 2037 GMT',
    'notBefore': 'Aug 29 14:23:16 2018 GMT',
    'serialNumber': 'CB2D80995A69525C',
    'subject': ((('countryName', 'XY'),),
             (('localityName', 'Castle Anthrax'),),
             (('organizationName', 'Python Software Foundation'),),
             (('commonName', 'localhost'),)),
    'subjectAltName': (('DNS', 'localhost'),),
    'version': 3
}

SIGNED_CERTFILE2 = data_file("keycert4.pem")
SIGNED_CERTFILE2_HOSTNAME = 'fakehostname'
SIGNED_CERTFILE_ECC = data_file("keycertecc.pem")
SIGNED_CERTFILE_ECC_HOSTNAME = 'localhost-ecc'

# Same certificate as pycacert.pem, but without extra text in file
SIGNING_CA = data_file("capath", "ceff1710.0")
# cert with all kinds of subject alt names
ALLSANFILE = data_file("allsans.pem")
IDNSANSFILE = data_file("idnsans.pem")
NOSANFILE = data_file("nosan.pem")
NOSAN_HOSTNAME = 'localhost'

REMOTE_HOST = "self-signed.pythontest.net"

EMPTYCERT = data_file("nullcert.pem")
BADCERT = data_file("badcert.pem")
NONEXISTINGCERT = data_file("XXXnonexisting.pem")
BADKEY = data_file("badkey.pem")
NOKIACERT = data_file("nokia.pem")
NULLBYTECERT = data_file("nullbytecert.pem")
TALOS_INVALID_CRLDP = data_file("talos-2019-0758.pem")

DHFILE = data_file("ffdh3072.pem")
BYTES_DHFILE = os.fsencode(DHFILE)

# Not defined in all versions of OpenSSL
OP_NO_COMPRESSION = getattr(ssl, "OP_NO_COMPRESSION", 0)
OP_SINGLE_DH_USE = getattr(ssl, "OP_SINGLE_DH_USE", 0)
OP_SINGLE_ECDH_USE = getattr(ssl, "OP_SINGLE_ECDH_USE", 0)
OP_CIPHER_SERVER_PREFERENCE = getattr(ssl, "OP_CIPHER_SERVER_PREFERENCE", 0)
OP_ENABLE_MIDDLEBOX_COMPAT = getattr(ssl, "OP_ENABLE_MIDDLEBOX_COMPAT", 0)

# Ubuntu has patched OpenSSL and changed behavior of security level 2
# see https://bugs.python.org/issue41561#msg389003
def is_ubuntu():
    try:
        # Assume that any references of "ubuntu" implies Ubuntu-like distro
        # The workaround is not required for 18.04, but doesn't hurt either.
        with open("/etc/os-release", encoding="utf-8") as f:
            return "ubuntu" in f.read()
    except FileNotFoundError:
        return False

if is_ubuntu():
    def seclevel_workaround(*ctxs):
        """"Lower security level to '1' and allow all ciphers for TLS 1.0/1"""
        for ctx in ctxs:
            if (
                hasattr(ctx, "minimum_version") and
                ctx.minimum_version <= ssl.TLSVersion.TLSv1_1
            ):
                ctx.set_ciphers("@SECLEVEL=1:ALL")
else:
    def seclevel_workaround(*ctxs):
        pass


def has_tls_protocol(protocol):
    """Check if a TLS protocol is available and enabled

    :param protocol: enum ssl._SSLMethod member or name
    :return: bool
    """
    if isinstance(protocol, str):
        assert protocol.startswith('PROTOCOL_')
        protocol = getattr(ssl, protocol, None)
        if protocol is None:
            return False
    if protocol in {
        ssl.PROTOCOL_TLS, ssl.PROTOCOL_TLS_SERVER,
        ssl.PROTOCOL_TLS_CLIENT
    }:
        # auto-negotiate protocols are always available
        return True
    name = protocol.name
    return has_tls_version(name[len('PROTOCOL_'):])


@functools.lru_cache
def has_tls_version(version):
    """Check if a TLS/SSL version is enabled

    :param version: TLS version name or ssl.TLSVersion member
    :return: bool
    """
    if isinstance(version, str):
        version = ssl.TLSVersion.__members__[version]

    # check compile time flags like ssl.HAS_TLSv1_2
    if not getattr(ssl, f'HAS_{version.name}'):
        return False

    if IS_OPENSSL_3_0_0 and version < ssl.TLSVersion.TLSv1_2:
        # bpo43791: 3.0.0-alpha14 fails with TLSV1_ALERT_INTERNAL_ERROR
        return False

    # check runtime and dynamic crypto policy settings. A TLS version may
    # be compiled in but disabled by a policy or config option.
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    if (
            hasattr(ctx, 'minimum_version') and
            ctx.minimum_version != ssl.TLSVersion.MINIMUM_SUPPORTED and
            version < ctx.minimum_version
    ):
        return False
    if (
        hasattr(ctx, 'maximum_version') and
        ctx.maximum_version != ssl.TLSVersion.MAXIMUM_SUPPORTED and
        version > ctx.maximum_version
    ):
        return False

    return True


def requires_tls_version(version):
    """Decorator to skip tests when a required TLS version is not available

    :param version: TLS version name or ssl.TLSVersion member
    :return:
    """
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kw):
            if not has_tls_version(version):
                raise unittest.SkipTest(f"{version} is not available.")
            else:
                return func(*args, **kw)
        return wrapper
    return decorator


def handle_error(prefix):
    exc_format = ' '.join(traceback.format_exception(sys.exception()))
    if support.verbose:
        sys.stdout.write(prefix + exc_format)


def utc_offset(): #NOTE: ignore issues like #1647654
    # local time = utc time + utc offset
    if time.daylight and time.localtime().tm_isdst > 0:
        return -time.altzone  # seconds
    return -time.timezone


ignore_deprecation = warnings_helper.ignore_warnings(
    category=DeprecationWarning
)


def test_wrap_socket(sock, *,
                     cert_reqs=ssl.CERT_NONE, ca_certs=None,
                     ciphers=None, certfile=None, keyfile=None,
                     **kwargs):
    if not kwargs.get("server_side"):
        kwargs["server_hostname"] = SIGNED_CERTFILE_HOSTNAME
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    else:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    if cert_reqs is not None:
        if cert_reqs == ssl.CERT_NONE:
            context.check_hostname = False
        context.verify_mode = cert_reqs
    if ca_certs is not None:
        context.load_verify_locations(ca_certs)
    if certfile is not None or keyfile is not None:
        context.load_cert_chain(certfile, keyfile)
    if ciphers is not None:
        context.set_ciphers(ciphers)
    return context.wrap_socket(sock, **kwargs)


def testing_context(server_cert=SIGNED_CERTFILE, *, server_chain=True):
    """Create context

    client_context, server_context, hostname = testing_context()
    """
    if server_cert == SIGNED_CERTFILE:
        hostname = SIGNED_CERTFILE_HOSTNAME
    elif server_cert == SIGNED_CERTFILE2:
        hostname = SIGNED_CERTFILE2_HOSTNAME
    elif server_cert == NOSANFILE:
        hostname = NOSAN_HOSTNAME
    else:
        raise ValueError(server_cert)

    client_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    client_context.load_verify_locations(SIGNING_CA)

    server_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    server_context.load_cert_chain(server_cert)
    if server_chain:
        server_context.load_verify_locations(SIGNING_CA)

    return client_context, server_context, hostname


class BasicSocketTests(unittest.TestCase):

    def test_constants(self):
        ssl.CERT_NONE
        ssl.CERT_OPTIONAL
        ssl.CERT_REQUIRED
        ssl.OP_CIPHER_SERVER_PREFERENCE
        ssl.OP_SINGLE_DH_USE
        ssl.OP_SINGLE_ECDH_USE
        ssl.OP_NO_COMPRESSION
        self.assertEqual(ssl.HAS_SNI, True)
        self.assertEqual(ssl.HAS_ECDH, True)
        self.assertEqual(ssl.HAS_TLSv1_2, True)
        self.assertEqual(ssl.HAS_TLSv1_3, True)
        ssl.OP_NO_SSLv2
        ssl.OP_NO_SSLv3
        ssl.OP_NO_TLSv1
        ssl.OP_NO_TLSv1_3
        ssl.OP_NO_TLSv1_1
        ssl.OP_NO_TLSv1_2
        self.assertEqual(ssl.PROTOCOL_TLS, ssl.PROTOCOL_SSLv23)

    def test_options(self):
        # gh-106687: SSL options values are unsigned integer (uint64_t)
        for name in dir(ssl):
            if not name.startswith('OP_'):
                continue
            with self.subTest(option=name):
                value = getattr(ssl, name)
                self.assertGreaterEqual(value, 0, f"ssl.{name}")

    def test_ssl_types(self):
        ssl_types = [
            _ssl._SSLContext,
            _ssl._SSLSocket,
            _ssl.MemoryBIO,
            _ssl.Certificate,
            _ssl.SSLSession,
            _ssl.SSLError,
        ]
        for ssl_type in ssl_types:
            with self.subTest(ssl_type=ssl_type):
                with self.assertRaisesRegex(TypeError, "immutable type"):
                    ssl_type.value = None
        support.check_disallow_instantiation(self, _ssl.Certificate)

    def test_private_init(self):
        with self.assertRaisesRegex(TypeError, "public constructor"):
            with socket.socket() as s:
                ssl.SSLSocket(s)

    def test_str_for_enums(self):
        # Make sure that the PROTOCOL_* constants have enum-like string
        # reprs.
        proto = ssl.PROTOCOL_TLS_CLIENT
        self.assertEqual(repr(proto), '<_SSLMethod.PROTOCOL_TLS_CLIENT: %r>' % proto.value)
        self.assertEqual(str(proto), str(proto.value))
        ctx = ssl.SSLContext(proto)
        self.assertIs(ctx.protocol, proto)

    def test_random(self):
        v = ssl.RAND_status()
        if support.verbose:
            sys.stdout.write("\n RAND_status is %d (%s)\n"
                             % (v, (v and "sufficient randomness") or
                                "insufficient randomness"))

        if v:
            data = ssl.RAND_bytes(16)
            self.assertEqual(len(data), 16)
        else:
            self.assertRaises(ssl.SSLError, ssl.RAND_bytes, 16)

        # negative num is invalid
        self.assertRaises(ValueError, ssl.RAND_bytes, -5)

        ssl.RAND_add("this is a random string", 75.0)
        ssl.RAND_add(b"this is a random bytes object", 75.0)
        ssl.RAND_add(bytearray(b"this is a random bytearray object"), 75.0)

    def test_parse_cert(self):
        # note that this uses an 'unofficial' function in _ssl.c,
        # provided solely for this test, to exercise the certificate
        # parsing code
        self.assertEqual(
            ssl._ssl._test_decode_cert(CERTFILE),
            CERTFILE_INFO
        )
        self.assertEqual(
            ssl._ssl._test_decode_cert(SIGNED_CERTFILE),
            SIGNED_CERTFILE_INFO
        )

        # Issue #13034: the subjectAltName in some certificates
        # (notably projects.developer.nokia.com:443) wasn't parsed
        p = ssl._ssl._test_decode_cert(NOKIACERT)
        if support.verbose:
            sys.stdout.write("\n" + pprint.pformat(p) + "\n")
        self.assertEqual(p['subjectAltName'],
                         (('DNS', 'projects.developer.nokia.com'),
                          ('DNS', 'projects.forum.nokia.com'))
                        )
        # extra OCSP and AIA fields
        self.assertEqual(p['OCSP'], ('http://ocsp.verisign.com',))
        self.assertEqual(p['caIssuers'],
                         ('http://SVRIntl-G3-aia.verisign.com/SVRIntlG3.cer',))
        self.assertEqual(p['crlDistributionPoints'],
                         ('http://SVRIntl-G3-crl.verisign.com/SVRIntlG3.crl',))

    def test_parse_cert_CVE_2019_5010(self):
        p = ssl._ssl._test_decode_cert(TALOS_INVALID_CRLDP)
        if support.verbose:
            sys.stdout.write("\n" + pprint.pformat(p) + "\n")
        self.assertEqual(
            p,
            {
                'issuer': (
                    (('countryName', 'UK'),), (('commonName', 'cody-ca'),)),
                'notAfter': 'Jun 14 18:00:58 2028 GMT',
                'notBefore': 'Jun 18 18:00:58 2018 GMT',
                'serialNumber': '02',
                'subject': ((('countryName', 'UK'),),
                            (('commonName',
                              'codenomicon-vm-2.test.lal.cisco.com'),)),
                'subjectAltName': (
                    ('DNS', 'codenomicon-vm-2.test.lal.cisco.com'),),
                'version': 3
            }
        def sni_cb(sock, servername, ctx):
            sock.context = server_context2

        server_context._msg_callback = msg_cb
        server_context.sni_callback = sni_cb

        server = ThreadedEchoServer(context=server_context, chatty=False)
        with server:
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
                s.connect((HOST, server.port))
            with client_context.wrap_socket(socket.socket(),
                                            server_hostname=hostname) as s:
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
            MINIMUM_SUPPORTED = _ssl.PROTO_MINIMUM_SUPPORTED
            SSLv3 = _ssl.PROTO_SSLv3
            TLSv1 = _ssl.PROTO_TLSv1
            TLSv1_1 = _ssl.PROTO_TLSv1_1
            TLSv1_2 = _ssl.PROTO_TLSv1_2
            TLSv1_3 = _ssl.PROTO_TLSv1_3
            MAXIMUM_SUPPORTED = _ssl.PROTO_MAXIMUM_SUPPORTED
        enum._test_simple_enum(CheckedTLSVersion, TLSVersion)

    def test_tlscontenttype(self):
        class Checked_TLSContentType(enum.IntEnum):
            """Content types (record layer)

            See RFC 8446, section B.1
            """
            CHANGE_CIPHER_SPEC = 20
            ALERT = 21
            HANDSHAKE = 22
            APPLICATION_DATA = 23
            # pseudo content types
            HEADER = 0x100
            INNER_CONTENT_TYPE = 0x101
        enum._test_simple_enum(Checked_TLSContentType, _TLSContentType)

    def test_tlsalerttype(self):
        class Checked_TLSAlertType(enum.IntEnum):
            """Alert types for TLSContentType.ALERT messages

            See RFC 8466, section B.2
            """
            CLOSE_NOTIFY = 0
            UNEXPECTED_MESSAGE = 10
            BAD_RECORD_MAC = 20
            DECRYPTION_FAILED = 21
            RECORD_OVERFLOW = 22
            DECOMPRESSION_FAILURE = 30
            HANDSHAKE_FAILURE = 40
            NO_CERTIFICATE = 41
            BAD_CERTIFICATE = 42
            UNSUPPORTED_CERTIFICATE = 43
            CERTIFICATE_REVOKED = 44
            CERTIFICATE_EXPIRED = 45
            CERTIFICATE_UNKNOWN = 46
            ILLEGAL_PARAMETER = 47
            UNKNOWN_CA = 48
            ACCESS_DENIED = 49
            DECODE_ERROR = 50
            DECRYPT_ERROR = 51
            EXPORT_RESTRICTION = 60
            PROTOCOL_VERSION = 70
            INSUFFICIENT_SECURITY = 71
            INTERNAL_ERROR = 80
            INAPPROPRIATE_FALLBACK = 86
            USER_CANCELED = 90
            NO_RENEGOTIATION = 100
            MISSING_EXTENSION = 109
            UNSUPPORTED_EXTENSION = 110
            CERTIFICATE_UNOBTAINABLE = 111
            UNRECOGNIZED_NAME = 112
            BAD_CERTIFICATE_STATUS_RESPONSE = 113
            BAD_CERTIFICATE_HASH_VALUE = 114
            UNKNOWN_PSK_IDENTITY = 115
            CERTIFICATE_REQUIRED = 116
            NO_APPLICATION_PROTOCOL = 120
        enum._test_simple_enum(Checked_TLSAlertType, _TLSAlertType)

    def test_tlsmessagetype(self):
        class Checked_TLSMessageType(enum.IntEnum):
            """Message types (handshake protocol)

            See RFC 8446, section B.3
            """
            HELLO_REQUEST = 0
            CLIENT_HELLO = 1
            SERVER_HELLO = 2
            HELLO_VERIFY_REQUEST = 3
            NEWSESSION_TICKET = 4
            END_OF_EARLY_DATA = 5
            HELLO_RETRY_REQUEST = 6
            ENCRYPTED_EXTENSIONS = 8
            CERTIFICATE = 11
            SERVER_KEY_EXCHANGE = 12
            CERTIFICATE_REQUEST = 13
            SERVER_DONE = 14
            CERTIFICATE_VERIFY = 15
            CLIENT_KEY_EXCHANGE = 16
            FINISHED = 20
            CERTIFICATE_URL = 21
            CERTIFICATE_STATUS = 22
            SUPPLEMENTAL_DATA = 23
            KEY_UPDATE = 24
            NEXT_PROTO = 67
            MESSAGE_HASH = 254
            CHANGE_CIPHER_SPEC = 0x0101
        enum._test_simple_enum(Checked_TLSMessageType, _TLSMessageType)

    def test_sslmethod(self):
        Checked_SSLMethod = enum._old_convert_(
                enum.IntEnum, '_SSLMethod', 'ssl',
                lambda name: name.startswith('PROTOCOL_') and name != 'PROTOCOL_SSLv23',
                source=ssl._ssl,
                )
        # This member is assigned dynamically in `ssl.py`:
        Checked_SSLMethod.PROTOCOL_SSLv23 = Checked_SSLMethod.PROTOCOL_TLS
        enum._test_simple_enum(Checked_SSLMethod, ssl._SSLMethod)

    def test_options(self):
        CheckedOptions = enum._old_convert_(
                enum.IntFlag, 'Options', 'ssl',
                lambda name: name.startswith('OP_'),
                source=ssl._ssl,
                )
        enum._test_simple_enum(CheckedOptions, ssl.Options)

    def test_alertdescription(self):
        CheckedAlertDescription = enum._old_convert_(
                enum.IntEnum, 'AlertDescription', 'ssl',
                lambda name: name.startswith('ALERT_DESCRIPTION_'),
                source=ssl._ssl,
                )
        enum._test_simple_enum(CheckedAlertDescription, ssl.AlertDescription)

    def test_sslerrornumber(self):
        Checked_SSLErrorNumber = enum._old_convert_(
                enum.IntEnum, 'SSLErrorNumber', 'ssl',
                lambda name: name.startswith('SSL_ERROR_'),
                source=ssl._ssl,
                )
        enum._test_simple_enum(Checked_SSLErrorNumber, ssl.SSLErrorNumber)

    def test_verifyflags(self):
        CheckedVerifyFlags = enum._old_convert_(
                enum.IntFlag, 'VerifyFlags', 'ssl',
                lambda name: name.startswith('VERIFY_'),
                source=ssl._ssl,
                )
        enum._test_simple_enum(CheckedVerifyFlags, ssl.VerifyFlags)

    def test_verifymode(self):
        CheckedVerifyMode = enum._old_convert_(
                enum.IntEnum, 'VerifyMode', 'ssl',
                lambda name: name.startswith('CERT_'),
                source=ssl._ssl,
                )
        enum._test_simple_enum(CheckedVerifyMode, ssl.VerifyMode)


def setUpModule():
    if support.verbose:
        plats = {
            'Mac': platform.mac_ver,
            'Windows': platform.win32_ver,
        }
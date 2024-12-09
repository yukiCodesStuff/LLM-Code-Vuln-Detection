    def test_dotted_attribute(self):
        # Raises an AttributeError because private methods are not allowed.
        self.assertRaises(AttributeError,
                          SimpleXMLRPCServer.resolve_dotted_attribute, str, '__add')

        self.assertTrue(SimpleXMLRPCServer.resolve_dotted_attribute(str, 'title'))
        # Get the test to run faster by sending a request with test_simple1.
        # This avoids waiting for the socket timeout.
        self.test_simple1()

class MultiPathServerTestCase(BaseServerTestCase):
    threadFunc = staticmethod(http_multi_server)
    request_count = 2
    def test_path1(self):
        p = xmlrpclib.ServerProxy(URL+"/foo")
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertRaises(xmlrpclib.Fault, p.add, 6, 8)
    def test_path2(self):
        p = xmlrpclib.ServerProxy(URL+"/foo/bar")
        self.assertEqual(p.add(6,8), 6+8)
        self.assertRaises(xmlrpclib.Fault, p.pow, 6, 8)

#A test case that verifies that a server using the HTTP/1.1 keep-alive mechanism
#does indeed serve subsequent requests on the same connection
class BaseKeepaliveServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(SimpleXMLRPCServer.SimpleXMLRPCRequestHandler):
        parentClass = SimpleXMLRPCServer.SimpleXMLRPCRequestHandler
        protocol_version = 'HTTP/1.1'
        myRequests = []
        def handle(self):
            self.myRequests.append([])
            self.reqidx = len(self.myRequests)-1
            return self.parentClass.handle(self)
        def handle_one_request(self):
            result = self.parentClass.handle_one_request(self)
            self.myRequests[self.reqidx].append(self.raw_requestline)
            return result

    requestHandler = RequestHandler
    def setUp(self):
        #clear request log
        self.RequestHandler.myRequests = []
        return BaseServerTestCase.setUp(self)

#A test case that verifies that a server using the HTTP/1.1 keep-alive mechanism
#does indeed serve subsequent requests on the same connection
class KeepaliveServerTestCase1(BaseKeepaliveServerTestCase):
    def test_two(self):
        p = xmlrpclib.ServerProxy(URL)
        #do three requests.
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)

        #they should have all been handled by a single request handler
        self.assertEqual(len(self.RequestHandler.myRequests), 1)

        #check that we did at least two (the third may be pending append
        #due to thread scheduling)
        self.assertGreaterEqual(len(self.RequestHandler.myRequests[-1]), 2)

#test special attribute access on the serverproxy, through the __call__
#function.
class KeepaliveServerTestCase2(BaseKeepaliveServerTestCase):
    #ask for two keepalive requests to be handled.
    request_count=2

    def test_close(self):
        p = xmlrpclib.ServerProxy(URL)
        #do some requests with close.
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)
        p("close")() #this should trigger a new keep-alive request
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(p.pow(6,8), 6**8)

        #they should have all been two request handlers, each having logged at least
        #two complete requests
        self.assertEqual(len(self.RequestHandler.myRequests), 2)
        self.assertGreaterEqual(len(self.RequestHandler.myRequests[-1]), 2)
        self.assertGreaterEqual(len(self.RequestHandler.myRequests[-2]), 2)

    def test_transport(self):
        p = xmlrpclib.ServerProxy(URL)
        #do some requests with close.
        self.assertEqual(p.pow(6,8), 6**8)
        p("transport").close() #same as above, really.
        self.assertEqual(p.pow(6,8), 6**8)
        self.assertEqual(len(self.RequestHandler.myRequests), 2)

#A test case that verifies that gzip encoding works in both directions
#(for a request and the response)
class GzipServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(SimpleXMLRPCServer.SimpleXMLRPCRequestHandler):
        parentClass = SimpleXMLRPCServer.SimpleXMLRPCRequestHandler
        protocol_version = 'HTTP/1.1'

        def do_POST(self):
            #store content of last request in class
            self.__class__.content_length = int(self.headers["content-length"])
            return self.parentClass.do_POST(self)
    requestHandler = RequestHandler

    class Transport(xmlrpclib.Transport):
        #custom transport, stores the response length for our perusal
        fake_gzip = False
        def parse_response(self, response):
            self.response_length=int(response.getheader("content-length", 0))
            return xmlrpclib.Transport.parse_response(self, response)

        def send_content(self, connection, body):
            if self.fake_gzip:
                #add a lone gzip header to induce decode error remotely
                connection.putheader("Content-Encoding", "gzip")
            return xmlrpclib.Transport.send_content(self, connection, body)

    def setUp(self):
        BaseServerTestCase.setUp(self)

    def test_gzip_request(self):
        t = self.Transport()
        t.encode_threshold = None
        p = xmlrpclib.ServerProxy(URL, transport=t)
        self.assertEqual(p.pow(6,8), 6**8)
        a = self.RequestHandler.content_length
        t.encode_threshold = 0 #turn on request encoding
        self.assertEqual(p.pow(6,8), 6**8)
        b = self.RequestHandler.content_length
        self.assertTrue(a>b)

    def test_bad_gzip_request(self):
        t = self.Transport()
        t.encode_threshold = None
        t.fake_gzip = True
        p = xmlrpclib.ServerProxy(URL, transport=t)
        cm = self.assertRaisesRegexp(xmlrpclib.ProtocolError,
                                     re.compile(r"\b400\b"))
        with cm:
            p.pow(6, 8)

    def test_gsip_response(self):
        t = self.Transport()
        p = xmlrpclib.ServerProxy(URL, transport=t)
        old = self.requestHandler.encode_threshold
        self.requestHandler.encode_threshold = None #no encoding
        self.assertEqual(p.pow(6,8), 6**8)
        a = t.response_length
        self.requestHandler.encode_threshold = 0 #always encode
        self.assertEqual(p.pow(6,8), 6**8)
        b = t.response_length
        self.requestHandler.encode_threshold = old
        self.assertTrue(a>b)

#Test special attributes of the ServerProxy object
class ServerProxyTestCase(unittest.TestCase):
    def setUp(self):
        unittest.TestCase.setUp(self)
        if threading:
            self.url = URL
        else:
            # Without threading, http_server() and http_multi_server() will not
            # be executed and URL is still equal to None. 'http://' is a just
            # enough to choose the scheme (HTTP)
            self.url = 'http://'

    def test_close(self):
        p = xmlrpclib.ServerProxy(self.url)
        self.assertEqual(p('close')(), None)

    def test_transport(self):
        t = xmlrpclib.Transport()
        p = xmlrpclib.ServerProxy(self.url, transport=t)
        self.assertEqual(p('transport'), t)

# This is a contrived way to make a failure occur on the server side
# in order to test the _send_traceback_header flag on the server
class FailingMessageClass(mimetools.Message):
    def __getitem__(self, key):
        key = key.lower()
        if key == 'content-length':
            return 'I am broken'
        return mimetools.Message.__getitem__(self, key)


@unittest.skipUnless(threading, 'Threading required for this test.')
class FailingServerTestCase(unittest.TestCase):
    def setUp(self):
        self.evt = threading.Event()
        # start server thread to handle requests
        serv_args = (self.evt, 1)
        threading.Thread(target=http_server, args=serv_args).start()

        # wait for the server to be ready
        self.evt.wait()
        self.evt.clear()

    def tearDown(self):
        # wait on the server thread to terminate
        self.evt.wait()
        # reset flag
        SimpleXMLRPCServer.SimpleXMLRPCServer._send_traceback_header = False
        # reset message class
        SimpleXMLRPCServer.SimpleXMLRPCRequestHandler.MessageClass = mimetools.Message

    def test_basic(self):
        # check that flag is false by default
        flagval = SimpleXMLRPCServer.SimpleXMLRPCServer._send_traceback_header
        self.assertEqual(flagval, False)

        # enable traceback reporting
        SimpleXMLRPCServer.SimpleXMLRPCServer._send_traceback_header = True

        # test a call that shouldn't fail just as a smoke test
        try:
            p = xmlrpclib.ServerProxy(URL)
            self.assertEqual(p.pow(6,8), 6**8)
        except (xmlrpclib.ProtocolError, socket.error), e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_fail_no_info(self):
        # use the broken message class
        SimpleXMLRPCServer.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error), e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # The two server-side error headers shouldn't be sent back in this case
                self.assertTrue(e.headers.get("X-exception") is None)
                self.assertTrue(e.headers.get("X-traceback") is None)
        else:
            self.fail('ProtocolError not raised')

    def test_fail_with_info(self):
        # use the broken message class
        SimpleXMLRPCServer.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        # Check that errors in the server send back exception/traceback
        # info when flag is set
        SimpleXMLRPCServer.SimpleXMLRPCServer._send_traceback_header = True

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error), e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # We should get error info in the response
                expected_err = "invalid literal for int() with base 10: 'I am broken'"
                self.assertEqual(e.headers.get("x-exception"), expected_err)
                self.assertTrue(e.headers.get("x-traceback") is not None)
        else:
            self.fail('ProtocolError not raised')

class CGIHandlerTestCase(unittest.TestCase):
    def setUp(self):
        self.cgi = SimpleXMLRPCServer.CGIXMLRPCRequestHandler()

    def tearDown(self):
        self.cgi = None

    def test_cgi_get(self):
        with test_support.EnvironmentVarGuard() as env:
            env['REQUEST_METHOD'] = 'GET'
            # if the method is GET and no request_text is given, it runs handle_get
            # get sysout output
            with test_support.captured_stdout() as data_out:
                self.cgi.handle_request()

            # parse Status header
            data_out.seek(0)
            handle = data_out.read()
            status = handle.split()[1]
            message = ' '.join(handle.split()[2:4])

            self.assertEqual(status, '400')
            self.assertEqual(message, 'Bad Request')


    def test_cgi_xmlrpc_response(self):
        data = """<?xml version='1.0'?>
        <methodCall>
            <methodName>test_method</methodName>
            <params>
                <param>
                    <value><string>foo</string></value>
                </param>
                <param>
                    <value><string>bar</string></value>
                </param>
            </params>
        </methodCall>
        """

        with test_support.EnvironmentVarGuard() as env, \
             test_support.captured_stdout() as data_out, \
             test_support.captured_stdin() as data_in:
            data_in.write(data)
            data_in.seek(0)
            env['CONTENT_LENGTH'] = str(len(data))
            self.cgi.handle_request()
        data_out.seek(0)

        # will respond exception, if so, our goal is achieved ;)
        handle = data_out.read()

        # start with 44th char so as not to get http header, we just need only xml
        self.assertRaises(xmlrpclib.Fault, xmlrpclib.loads, handle[44:])

        # Also test the content-length returned  by handle_request
        # Using the same test method inorder to avoid all the datapassing
        # boilerplate code.
        # Test for bug: http://bugs.python.org/issue5040

        content = handle[handle.find("<?xml"):]

        self.assertEqual(
            int(re.search('Content-Length: (\d+)', handle).group(1)),
            len(content))


class FakeSocket:

    def __init__(self):
        self.data = StringIO.StringIO()

    def send(self, buf):
        self.data.write(buf)
        return len(buf)

    def sendall(self, buf):
        self.data.write(buf)

    def getvalue(self):
        return self.data.getvalue()

    def makefile(self, x='r', y=-1):
        raise RuntimeError

    def close(self):
        pass

class FakeTransport(xmlrpclib.Transport):
    """A Transport instance that records instead of sending a request.

    This class replaces the actual socket used by httplib with a
    FakeSocket object that records the request.  It doesn't provide a
    response.
    """

    def make_connection(self, host):
        conn = xmlrpclib.Transport.make_connection(self, host)
        conn.sock = self.fake_socket = FakeSocket()
        return conn

class TransportSubclassTestCase(unittest.TestCase):

    def issue_request(self, transport_class):
        """Return an HTTP request made via transport_class."""
        transport = transport_class()
        proxy = xmlrpclib.ServerProxy("http://example.com/",
                                      transport=transport)
        try:
            proxy.pow(6, 8)
        except RuntimeError:
            return transport.fake_socket.getvalue()
        return None

    def test_custom_user_agent(self):
        class TestTransport(FakeTransport):

            def send_user_agent(self, conn):
                xmlrpclib.Transport.send_user_agent(self, conn)
                conn.putheader("X-Test", "test_custom_user_agent")

        req = self.issue_request(TestTransport)
        self.assertIn("X-Test: test_custom_user_agent\r\n", req)

    def test_send_host(self):
        class TestTransport(FakeTransport):

            def send_host(self, conn, host):
                xmlrpclib.Transport.send_host(self, conn, host)
                conn.putheader("X-Test", "test_send_host")

        req = self.issue_request(TestTransport)
        self.assertIn("X-Test: test_send_host\r\n", req)

    def test_send_request(self):
        class TestTransport(FakeTransport):

            def send_request(self, conn, url, body):
                xmlrpclib.Transport.send_request(self, conn, url, body)
                conn.putheader("X-Test", "test_send_request")

        req = self.issue_request(TestTransport)
        self.assertIn("X-Test: test_send_request\r\n", req)

    def test_send_content(self):
        class TestTransport(FakeTransport):

            def send_content(self, conn, body):
                conn.putheader("X-Test", "test_send_content")
                xmlrpclib.Transport.send_content(self, conn, body)

        req = self.issue_request(TestTransport)
        self.assertIn("X-Test: test_send_content\r\n", req)

@test_support.reap_threads
def test_main():
    xmlrpc_tests = [XMLRPCTestCase, HelperTestCase, DateTimeTestCase,
         BinaryTestCase, FaultTestCase, TransportSubclassTestCase]
    xmlrpc_tests.append(SimpleServerTestCase)
    xmlrpc_tests.append(KeepaliveServerTestCase1)
    xmlrpc_tests.append(KeepaliveServerTestCase2)
    try:
        import gzip
        xmlrpc_tests.append(GzipServerTestCase)
    except ImportError:
        pass #gzip not supported in this build
    xmlrpc_tests.append(MultiPathServerTestCase)
    xmlrpc_tests.append(ServerProxyTestCase)
    xmlrpc_tests.append(FailingServerTestCase)
    xmlrpc_tests.append(CGIHandlerTestCase)

    test_support.run_unittest(*xmlrpc_tests)

if __name__ == "__main__":
    test_main()
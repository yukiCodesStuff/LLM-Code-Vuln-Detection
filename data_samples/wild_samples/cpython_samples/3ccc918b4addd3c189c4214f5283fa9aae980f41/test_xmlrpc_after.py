    def tearDown(self):
        # wait on the server thread to terminate
        self.evt.wait()

        # disable traceback reporting
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = False

class SimpleServerTestCase(BaseServerTestCase):
    def test_simple1(self):
        try:
            p = xmlrpclib.ServerProxy(URL)
            self.assertEqual(p.pow(6,8), 6**8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_nonascii(self):
        start_string = 'P\N{LATIN SMALL LETTER Y WITH CIRCUMFLEX}t'
        end_string = 'h\N{LATIN SMALL LETTER O WITH HORN}n'
        try:
            p = xmlrpclib.ServerProxy(URL)
            self.assertEqual(p.add(start_string, end_string),
                             start_string + end_string)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    # [ch] The test 404 is causing lots of false alarms.
    def XXXtest_404(self):
        # send POST with http.client, it should return 404 header and
        # 'Not Found' message.
        conn = httplib.client.HTTPConnection(ADDR, PORT)
        conn.request('POST', '/this-is-not-valid')
        response = conn.getresponse()
        conn.close()

        self.assertEqual(response.status, 404)
        self.assertEqual(response.reason, 'Not Found')

    def test_introspection1(self):
        expected_methods = set(['pow', 'div', 'my_function', 'add',
                                'system.listMethods', 'system.methodHelp',
                                'system.methodSignature', 'system.multicall'])
        try:
            p = xmlrpclib.ServerProxy(URL)
            meth = p.system.listMethods()
            self.assertEqual(set(meth), expected_methods)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))


    def test_introspection2(self):
        try:
            # test _methodHelp()
            p = xmlrpclib.ServerProxy(URL)
            divhelp = p.system.methodHelp('div')
            self.assertEqual(divhelp, 'This is the div function')
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    @make_request_and_skipIf(sys.flags.optimize >= 2,
                     "Docstrings are omitted with -O2 and above")
    def test_introspection3(self):
        try:
            # test native doc
            p = xmlrpclib.ServerProxy(URL)
            myfunction = p.system.methodHelp('my_function')
            self.assertEqual(myfunction, 'This is my function')
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_introspection4(self):
        # the SimpleXMLRPCServer doesn't support signatures, but
        # at least check that we can try making the call
        try:
            p = xmlrpclib.ServerProxy(URL)
            divsig = p.system.methodSignature('div')
            self.assertEqual(divsig, 'signatures not supported')
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_multicall(self):
        try:
            p = xmlrpclib.ServerProxy(URL)
            multicall = xmlrpclib.MultiCall(p)
            multicall.add(2,3)
            multicall.pow(6,8)
            multicall.div(127,42)
            add_result, pow_result, div_result = multicall()
            self.assertEqual(add_result, 2+3)
            self.assertEqual(pow_result, 6**8)
            self.assertEqual(div_result, 127//42)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_non_existing_multicall(self):
        try:
            p = xmlrpclib.ServerProxy(URL)
            multicall = xmlrpclib.MultiCall(p)
            multicall.this_is_not_exists()
            result = multicall()

            # result.results contains;
            # [{'faultCode': 1, 'faultString': '<class \'exceptions.Exception\'>:'
            #   'method "this_is_not_exists" is not supported'>}]

            self.assertEqual(result.results[0]['faultCode'], 1)
            self.assertEqual(result.results[0]['faultString'],
                '<class \'Exception\'>:method "this_is_not_exists" '
                'is not supported')
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_dotted_attribute(self):
        # Raises an AttributeError because private methods are not allowed.
        self.assertRaises(AttributeError,
                          xmlrpc.server.resolve_dotted_attribute, str, '__add')

        self.assertTrue(xmlrpc.server.resolve_dotted_attribute(str, 'title'))
        # Get the test to run faster by sending a request with test_simple1.
        # This avoids waiting for the socket timeout.
        self.test_simple1()

    def test_unicode_host(self):
        server = xmlrpclib.ServerProxy("http://%s:%d/RPC2" % (ADDR, PORT))
        self.assertEqual(server.add("a", "\xe9"), "a\xe9")

    def test_partial_post(self):
        # Check that a partial POST doesn't make the server loop: issue #14001.
        conn = http.client.HTTPConnection(ADDR, PORT)
        conn.request('POST', '/RPC2 HTTP/1.0\r\nContent-Length: 100\r\n\r\nbye')
        conn.close()


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

    def test_path3(self):
        p = xmlrpclib.ServerProxy(URL+"/is/broken")
        self.assertRaises(xmlrpclib.Fault, p.add, 6, 8)

#A test case that verifies that a server using the HTTP/1.1 keep-alive mechanism
#does indeed serve subsequent requests on the same connection
class BaseKeepaliveServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(xmlrpc.server.SimpleXMLRPCRequestHandler):
        parentClass = xmlrpc.server.SimpleXMLRPCRequestHandler
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
        p("close")()

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
        p("close")()

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
        p("close")()
        self.assertEqual(len(self.RequestHandler.myRequests), 2)

#A test case that verifies that gzip encoding works in both directions
#(for a request and the response)
class GzipServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(xmlrpc.server.SimpleXMLRPCRequestHandler):
        parentClass = xmlrpc.server.SimpleXMLRPCRequestHandler
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
        p("close")()

    def test_bad_gzip_request(self):
        t = self.Transport()
        t.encode_threshold = None
        t.fake_gzip = True
        p = xmlrpclib.ServerProxy(URL, transport=t)
        cm = self.assertRaisesRegex(xmlrpclib.ProtocolError,
                                    re.compile(r"\b400\b"))
        with cm:
            p.pow(6, 8)
        p("close")()

    def test_gsip_response(self):
        t = self.Transport()
        p = xmlrpclib.ServerProxy(URL, transport=t)
        old = self.requestHandler.encode_threshold
        self.requestHandler.encode_threshold = None #no encoding
        self.assertEqual(p.pow(6,8), 6**8)
        a = t.response_length
        self.requestHandler.encode_threshold = 0 #always encode
        self.assertEqual(p.pow(6,8), 6**8)
        p("close")()
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
class FailingMessageClass(http.client.HTTPMessage):
    def get(self, key, failobj=None):
        key = key.lower()
        if key == 'content-length':
            return 'I am broken'
        return super().get(key, failobj)


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
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = False
        # reset message class
        default_class = http.client.HTTPMessage
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = default_class

    def test_basic(self):
        # check that flag is false by default
        flagval = xmlrpc.server.SimpleXMLRPCServer._send_traceback_header
        self.assertEqual(flagval, False)

        # enable traceback reporting
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = True

        # test a call that shouldn't fail just as a smoke test
        try:
            p = xmlrpclib.ServerProxy(URL)
            self.assertEqual(p.pow(6,8), 6**8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_fail_no_info(self):
        # use the broken message class
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # The two server-side error headers shouldn't be sent back in this case
                self.assertTrue(e.headers.get("X-exception") is None)
                self.assertTrue(e.headers.get("X-traceback") is None)
        else:
            self.fail('ProtocolError not raised')

    def test_fail_with_info(self):
        # use the broken message class
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        # Check that errors in the server send back exception/traceback
        # info when flag is set
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = True

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # We should get error info in the response
                expected_err = "invalid literal for int() with base 10: 'I am broken'"
                self.assertEqual(e.headers.get("X-exception"), expected_err)
                self.assertTrue(e.headers.get("X-traceback") is not None)
        else:
            self.fail('ProtocolError not raised')


@contextlib.contextmanager
def captured_stdout(encoding='utf-8'):
    """A variation on support.captured_stdout() which gives a text stream
    having a `buffer` attribute.
    """
    import io
    orig_stdout = sys.stdout
    sys.stdout = io.TextIOWrapper(io.BytesIO(), encoding=encoding)
    try:
        yield sys.stdout
    finally:
        sys.stdout = orig_stdout


class CGIHandlerTestCase(unittest.TestCase):
    def setUp(self):
        self.cgi = xmlrpc.server.CGIXMLRPCRequestHandler()

    def tearDown(self):
        self.cgi = None

    def test_cgi_get(self):
        with support.EnvironmentVarGuard() as env:
            env['REQUEST_METHOD'] = 'GET'
            # if the method is GET and no request_text is given, it runs handle_get
            # get sysout output
            with captured_stdout(encoding=self.cgi.encoding) as data_out:
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

        with support.EnvironmentVarGuard() as env, \
             captured_stdout(encoding=self.cgi.encoding) as data_out, \
             support.captured_stdin() as data_in:
            data_in.write(data)
            data_in.seek(0)
            env['CONTENT_LENGTH'] = str(len(data))
            self.cgi.handle_request()
        data_out.seek(0)

        # will respond exception, if so, our goal is achieved ;)
        handle = data_out.read()

        # start with 44th char so as not to get http header, we just
        # need only xml
        self.assertRaises(xmlrpclib.Fault, xmlrpclib.loads, handle[44:])

        # Also test the content-length returned  by handle_request
        # Using the same test method inorder to avoid all the datapassing
        # boilerplate code.
        # Test for bug: http://bugs.python.org/issue5040

        content = handle[handle.find("<?xml"):]

        self.assertEqual(
            int(re.search('Content-Length: (\d+)', handle).group(1)),
            len(content))


class UseBuiltinTypesTestCase(unittest.TestCase):

    def test_use_builtin_types(self):
        # SimpleXMLRPCDispatcher.__init__ accepts use_builtin_types, which
        # makes all dispatch of binary data as bytes instances, and all
        # dispatch of datetime argument as datetime.datetime instances.
        self.log = []
        expected_bytes = b"my dog has fleas"
        expected_date = datetime.datetime(2008, 5, 26, 18, 25, 12)
        marshaled = xmlrpclib.dumps((expected_bytes, expected_date), 'foobar')
        def foobar(*args):
            self.log.extend(args)
        handler = xmlrpc.server.SimpleXMLRPCDispatcher(
            allow_none=True, encoding=None, use_builtin_types=True)
        handler.register_function(foobar)
        handler._marshaled_dispatch(marshaled)
        self.assertEqual(len(self.log), 2)
        mybytes, mydate = self.log
        self.assertEqual(self.log, [expected_bytes, expected_date])
        self.assertIs(type(mydate), datetime.datetime)
        self.assertIs(type(mybytes), bytes)

    def test_cgihandler_has_use_builtin_types_flag(self):
        handler = xmlrpc.server.CGIXMLRPCRequestHandler(use_builtin_types=True)
        self.assertTrue(handler.use_builtin_types)

    def test_xmlrpcserver_has_use_builtin_types_flag(self):
        server = xmlrpc.server.SimpleXMLRPCServer(("localhost", 0),
            use_builtin_types=True)
        server.server_close()
        self.assertTrue(server.use_builtin_types)


@support.reap_threads
def test_main():
    xmlrpc_tests = [XMLRPCTestCase, HelperTestCase, DateTimeTestCase,
         BinaryTestCase, FaultTestCase]
    xmlrpc_tests.append(UseBuiltinTypesTestCase)
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

    support.run_unittest(*xmlrpc_tests)

if __name__ == "__main__":
    test_main()
    def test_unicode_host(self):
        server = xmlrpclib.ServerProxy("http://%s:%d/RPC2" % (ADDR, PORT))
        self.assertEqual(server.add("a", "\xe9"), "a\xe9")

    def test_partial_post(self):
        # Check that a partial POST doesn't make the server loop: issue #14001.
        conn = http.client.HTTPConnection(ADDR, PORT)
        conn.request('POST', '/RPC2 HTTP/1.0\r\nContent-Length: 100\r\n\r\nbye')
        conn.close()


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

    def test_path3(self):
        p = xmlrpclib.ServerProxy(URL+"/is/broken")
        self.assertRaises(xmlrpclib.Fault, p.add, 6, 8)

#A test case that verifies that a server using the HTTP/1.1 keep-alive mechanism
#does indeed serve subsequent requests on the same connection
class BaseKeepaliveServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(xmlrpc.server.SimpleXMLRPCRequestHandler):
        parentClass = xmlrpc.server.SimpleXMLRPCRequestHandler
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
        p("close")()

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
        p("close")()

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
        p("close")()
        self.assertEqual(len(self.RequestHandler.myRequests), 2)

#A test case that verifies that gzip encoding works in both directions
#(for a request and the response)
class GzipServerTestCase(BaseServerTestCase):
    #a request handler that supports keep-alive and logs requests into a
    #class variable
    class RequestHandler(xmlrpc.server.SimpleXMLRPCRequestHandler):
        parentClass = xmlrpc.server.SimpleXMLRPCRequestHandler
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
        p("close")()

    def test_bad_gzip_request(self):
        t = self.Transport()
        t.encode_threshold = None
        t.fake_gzip = True
        p = xmlrpclib.ServerProxy(URL, transport=t)
        cm = self.assertRaisesRegex(xmlrpclib.ProtocolError,
                                    re.compile(r"\b400\b"))
        with cm:
            p.pow(6, 8)
        p("close")()

    def test_gsip_response(self):
        t = self.Transport()
        p = xmlrpclib.ServerProxy(URL, transport=t)
        old = self.requestHandler.encode_threshold
        self.requestHandler.encode_threshold = None #no encoding
        self.assertEqual(p.pow(6,8), 6**8)
        a = t.response_length
        self.requestHandler.encode_threshold = 0 #always encode
        self.assertEqual(p.pow(6,8), 6**8)
        p("close")()
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
class FailingMessageClass(http.client.HTTPMessage):
    def get(self, key, failobj=None):
        key = key.lower()
        if key == 'content-length':
            return 'I am broken'
        return super().get(key, failobj)


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
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = False
        # reset message class
        default_class = http.client.HTTPMessage
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = default_class

    def test_basic(self):
        # check that flag is false by default
        flagval = xmlrpc.server.SimpleXMLRPCServer._send_traceback_header
        self.assertEqual(flagval, False)

        # enable traceback reporting
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = True

        # test a call that shouldn't fail just as a smoke test
        try:
            p = xmlrpclib.ServerProxy(URL)
            self.assertEqual(p.pow(6,8), 6**8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e):
                # protocol error; provide additional information in test output
                self.fail("%s\n%s" % (e, getattr(e, "headers", "")))

    def test_fail_no_info(self):
        # use the broken message class
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # The two server-side error headers shouldn't be sent back in this case
                self.assertTrue(e.headers.get("X-exception") is None)
                self.assertTrue(e.headers.get("X-traceback") is None)
        else:
            self.fail('ProtocolError not raised')

    def test_fail_with_info(self):
        # use the broken message class
        xmlrpc.server.SimpleXMLRPCRequestHandler.MessageClass = FailingMessageClass

        # Check that errors in the server send back exception/traceback
        # info when flag is set
        xmlrpc.server.SimpleXMLRPCServer._send_traceback_header = True

        try:
            p = xmlrpclib.ServerProxy(URL)
            p.pow(6,8)
        except (xmlrpclib.ProtocolError, socket.error) as e:
            # ignore failures due to non-blocking socket 'unavailable' errors
            if not is_unavailable_exception(e) and hasattr(e, "headers"):
                # We should get error info in the response
                expected_err = "invalid literal for int() with base 10: 'I am broken'"
                self.assertEqual(e.headers.get("X-exception"), expected_err)
                self.assertTrue(e.headers.get("X-traceback") is not None)
        else:
            self.fail('ProtocolError not raised')


@contextlib.contextmanager
def captured_stdout(encoding='utf-8'):
    """A variation on support.captured_stdout() which gives a text stream
    having a `buffer` attribute.
    """
    import io
    orig_stdout = sys.stdout
    sys.stdout = io.TextIOWrapper(io.BytesIO(), encoding=encoding)
    try:
        yield sys.stdout
    finally:
        sys.stdout = orig_stdout


class CGIHandlerTestCase(unittest.TestCase):
    def setUp(self):
        self.cgi = xmlrpc.server.CGIXMLRPCRequestHandler()

    def tearDown(self):
        self.cgi = None

    def test_cgi_get(self):
        with support.EnvironmentVarGuard() as env:
            env['REQUEST_METHOD'] = 'GET'
            # if the method is GET and no request_text is given, it runs handle_get
            # get sysout output
            with captured_stdout(encoding=self.cgi.encoding) as data_out:
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

        with support.EnvironmentVarGuard() as env, \
             captured_stdout(encoding=self.cgi.encoding) as data_out, \
             support.captured_stdin() as data_in:
            data_in.write(data)
            data_in.seek(0)
            env['CONTENT_LENGTH'] = str(len(data))
            self.cgi.handle_request()
        data_out.seek(0)

        # will respond exception, if so, our goal is achieved ;)
        handle = data_out.read()

        # start with 44th char so as not to get http header, we just
        # need only xml
        self.assertRaises(xmlrpclib.Fault, xmlrpclib.loads, handle[44:])

        # Also test the content-length returned  by handle_request
        # Using the same test method inorder to avoid all the datapassing
        # boilerplate code.
        # Test for bug: http://bugs.python.org/issue5040

        content = handle[handle.find("<?xml"):]

        self.assertEqual(
            int(re.search('Content-Length: (\d+)', handle).group(1)),
            len(content))


class UseBuiltinTypesTestCase(unittest.TestCase):

    def test_use_builtin_types(self):
        # SimpleXMLRPCDispatcher.__init__ accepts use_builtin_types, which
        # makes all dispatch of binary data as bytes instances, and all
        # dispatch of datetime argument as datetime.datetime instances.
        self.log = []
        expected_bytes = b"my dog has fleas"
        expected_date = datetime.datetime(2008, 5, 26, 18, 25, 12)
        marshaled = xmlrpclib.dumps((expected_bytes, expected_date), 'foobar')
        def foobar(*args):
            self.log.extend(args)
        handler = xmlrpc.server.SimpleXMLRPCDispatcher(
            allow_none=True, encoding=None, use_builtin_types=True)
        handler.register_function(foobar)
        handler._marshaled_dispatch(marshaled)
        self.assertEqual(len(self.log), 2)
        mybytes, mydate = self.log
        self.assertEqual(self.log, [expected_bytes, expected_date])
        self.assertIs(type(mydate), datetime.datetime)
        self.assertIs(type(mybytes), bytes)

    def test_cgihandler_has_use_builtin_types_flag(self):
        handler = xmlrpc.server.CGIXMLRPCRequestHandler(use_builtin_types=True)
        self.assertTrue(handler.use_builtin_types)

    def test_xmlrpcserver_has_use_builtin_types_flag(self):
        server = xmlrpc.server.SimpleXMLRPCServer(("localhost", 0),
            use_builtin_types=True)
        server.server_close()
        self.assertTrue(server.use_builtin_types)


@support.reap_threads
def test_main():
    xmlrpc_tests = [XMLRPCTestCase, HelperTestCase, DateTimeTestCase,
         BinaryTestCase, FaultTestCase]
    xmlrpc_tests.append(UseBuiltinTypesTestCase)
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

    support.run_unittest(*xmlrpc_tests)

if __name__ == "__main__":
    test_main()
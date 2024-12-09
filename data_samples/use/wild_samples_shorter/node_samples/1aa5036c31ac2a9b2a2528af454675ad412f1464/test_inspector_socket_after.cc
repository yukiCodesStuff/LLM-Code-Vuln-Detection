  expect_failure_no_delegate(UPGRADE_REQUEST);
}

TEST_F(InspectorSocketTest, HostIPChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 10.0.2.555:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostNegativeIPChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 10.0.-23.255:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpOctetOutOfIntRangeChecked) {
  const std::string INVALID_HOST_IP_REQUEST =
      "GET /json HTTP/1.1\r\n"
      "Host: 127.0.0.4294967296:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpOctetFarOutOfIntRangeChecked) {
  const std::string INVALID_HOST_IP_REQUEST =
      "GET /json HTTP/1.1\r\n"
      "Host: 127.0.0.18446744073709552000:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpEmptyOctetStartChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: .0.0.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpEmptyOctetMidChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 127..0.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpEmptyOctetEndChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 127.0.0.:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpTooFewOctetsChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 127.0.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpTooManyOctetsChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 127.0.0.0.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

}  // anonymous namespace
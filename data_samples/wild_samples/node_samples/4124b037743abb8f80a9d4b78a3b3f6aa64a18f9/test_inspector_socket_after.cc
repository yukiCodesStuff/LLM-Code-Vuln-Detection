TEST_F(InspectorSocketTest, HostIpTooManyOctetsChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 127.0.0.0.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpInvalidOctalOctetStartChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 08.1.1.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpInvalidOctalOctetMidChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 1.09.1.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpInvalidOctalOctetEndChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 1.1.1.009:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpLeadingZeroStartChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 01.1.1.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpLeadingZeroMidChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 1.1.001.1:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIpLeadingZeroEndChecked) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 1.1.1.01:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPNonRoutable) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 0.0.0.0:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6NonRoutable) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [::]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6NonRoutableDual) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [::0.0.0.0]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv4InSquareBrackets) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [127.0.0.1]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6InvalidAbbreviation) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [:::1]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

}  // anonymous namespace
TEST_F(InspectorSocketTest, HostIPNonRoutable) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 0.0.0.0:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6NonRoutable) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [::]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6NonRoutableDual) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [::0.0.0.0]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv4InSquareBrackets) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [127.0.0.1]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPv6InvalidAbbreviation) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: [:::1]:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

}  // anonymous namespace
  expect_handshake_failure();
}

TEST_F(InspectorSocketTest, HostIPNonRoutable) {
  const std::string INVALID_HOST_IP_REQUEST = "GET /json HTTP/1.1\r\n"
                                              "Host: 0.0.0.0:9229\r\n\r\n";
  send_in_chunks(INVALID_HOST_IP_REQUEST.c_str(),
                 INVALID_HOST_IP_REQUEST.length());
  expect_handshake_failure();
}

}  // anonymous namespace
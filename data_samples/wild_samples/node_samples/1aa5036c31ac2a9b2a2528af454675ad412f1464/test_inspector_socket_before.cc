TEST_F(InspectorSocketTest, HostCheckedForUPGRADE) {
  const char UPGRADE_REQUEST[] = "GET /ws/path HTTP/1.1\r\n"
                                 "Host: nonlocalhost:9229\r\n"
                                 "Upgrade: websocket\r\n"
                                 "Connection: Upgrade\r\n"
                                 "Sec-WebSocket-Key: aaa==\r\n"
                                 "Sec-WebSocket-Version: 13\r\n\r\n";
  expect_failure_no_delegate(UPGRADE_REQUEST);
}

}  // anonymous namespace
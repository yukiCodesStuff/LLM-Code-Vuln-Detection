        )

    def test_errors(self):
        self.assertRaises(UnicodeDecodeError, codecs.utf_16_le_decode,
                          b"\xff", "strict", True)

    def test_nonbmp(self):
        self.assertEqual("\U00010203".encode(self.encoding),
                         b'\x00\xd8\x03\xde')
        )

    def test_errors(self):
        self.assertRaises(UnicodeDecodeError, codecs.utf_16_be_decode,
                          b"\xff", "strict", True)

    def test_nonbmp(self):
        self.assertEqual("\U00010203".encode(self.encoding),
                         b'\xd8\x00\xde\x03')
        )

    def test_errors(self):
        tests = [
            (b'\xff', '\ufffd'),
            (b'A\x00Z', 'A\ufffd'),
            (b'A\x00B\x00C\x00D\x00Z', 'ABCD\ufffd'),
            (b'\x00\xd8', '\ufffd'),
            (b'\x00\xd8A', '\ufffd'),
            (b'\x00\xd8A\x00', '\ufffdA'),
            (b'\x00\xdcA\x00', '\ufffdA'),
        ]
        for raw, expected in tests:
            self.assertRaises(UnicodeDecodeError, codecs.utf_16_le_decode,
                              raw, 'strict', True)
            self.assertEqual(raw.decode('utf-16le', 'replace'), expected)

    def test_nonbmp(self):
        self.assertEqual("\U00010203".encode(self.encoding),
                         b'\x00\xd8\x03\xde')
        )

    def test_errors(self):
        tests = [
            (b'\xff', '\ufffd'),
            (b'\x00A\xff', 'A\ufffd'),
            (b'\x00A\x00B\x00C\x00DZ', 'ABCD\ufffd'),
            (b'\xd8\x00', '\ufffd'),
            (b'\xd8\x00\xdc', '\ufffd'),
            (b'\xd8\x00\x00A', '\ufffdA'),
            (b'\xdc\x00\x00A', '\ufffdA'),
        ]
        for raw, expected in tests:
            self.assertRaises(UnicodeDecodeError, codecs.utf_16_be_decode,
                              raw, 'strict', True)
            self.assertEqual(raw.decode('utf-16be', 'replace'), expected)

    def test_nonbmp(self):
        self.assertEqual("\U00010203".encode(self.encoding),
                         b'\xd8\x00\xde\x03')
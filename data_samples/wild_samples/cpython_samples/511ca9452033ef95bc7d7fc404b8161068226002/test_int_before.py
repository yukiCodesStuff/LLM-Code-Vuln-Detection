    def test_issue31619(self):
        self.assertEqual(int('1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1_0_1', 2),
                         0b1010101010101010101010101010101)
        self.assertEqual(int('1_2_3_4_5_6_7_0_1_2_3', 8), 0o12345670123)
        self.assertEqual(int('1_2_3_4_5_6_7_8_9', 16), 0x123456789)
        self.assertEqual(int('1_2_3_4_5_6_7', 32), 1144132807)


if __name__ == "__main__":
    unittest.main()
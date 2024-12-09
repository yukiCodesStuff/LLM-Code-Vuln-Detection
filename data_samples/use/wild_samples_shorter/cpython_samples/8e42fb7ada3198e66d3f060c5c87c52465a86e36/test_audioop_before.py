
data = [gendata1(), gendata2(), gendata4()]


class TestAudioop(unittest.TestCase):

    def test_max(self):
        self.assertRaises(audioop.error,
            audioop.findmax, ''.join(chr(x) for x in range(256)), -2392392)

def test_main():
    run_unittest(TestAudioop)

if __name__ == '__main__':
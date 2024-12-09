
data = [gendata1(), gendata2(), gendata4()]

INVALID_DATA = [
    ('abc', 0),
    ('abc', 2),
    ('abc', 4),
]


class TestAudioop(unittest.TestCase):

    def test_max(self):
        self.assertRaises(audioop.error,
            audioop.findmax, ''.join( chr(x) for x in xrange(256)), -2392392)

    def test_issue7673(self):
        state = None
        for data, size in INVALID_DATA:
            size2 = size
            self.assertRaises(audioop.error, audioop.getsample, data, size, 0)
            self.assertRaises(audioop.error, audioop.max, data, size)
            self.assertRaises(audioop.error, audioop.minmax, data, size)
            self.assertRaises(audioop.error, audioop.avg, data, size)
            self.assertRaises(audioop.error, audioop.rms, data, size)
            self.assertRaises(audioop.error, audioop.avgpp, data, size)
            self.assertRaises(audioop.error, audioop.maxpp, data, size)
            self.assertRaises(audioop.error, audioop.cross, data, size)
            self.assertRaises(audioop.error, audioop.mul, data, size, 1.0)
            self.assertRaises(audioop.error, audioop.tomono, data, size, 0.5, 0.5)
            self.assertRaises(audioop.error, audioop.tostereo, data, size, 0.5, 0.5)
            self.assertRaises(audioop.error, audioop.add, data, data, size)
            self.assertRaises(audioop.error, audioop.bias, data, size, 0)
            self.assertRaises(audioop.error, audioop.reverse, data, size)
            self.assertRaises(audioop.error, audioop.lin2lin, data, size, size2)
            self.assertRaises(audioop.error, audioop.ratecv, data, size, 1, 1, 1, state)
            self.assertRaises(audioop.error, audioop.lin2ulaw, data, size)
            self.assertRaises(audioop.error, audioop.ulaw2lin, data, size)
            self.assertRaises(audioop.error, audioop.lin2alaw, data, size)
            self.assertRaises(audioop.error, audioop.alaw2lin, data, size)
            self.assertRaises(audioop.error, audioop.lin2adpcm, data, size, state)
            self.assertRaises(audioop.error, audioop.adpcm2lin, data, size, state)

def test_main():
    run_unittest(TestAudioop)

if __name__ == '__main__':
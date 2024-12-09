                with self.assertRaisesRegex(tarfile.ReadError, "unexpected end of data"):
                    tar.extractfile(t).read()

    def test_length_zero_header(self):
        # bpo-39017 (CVE-2019-20907): reading a zero-length header should fail
        # with an exception
        with self.assertRaisesRegex(tarfile.ReadError, "file could not be opened successfully"):
            with tarfile.open(support.findfile('recursion.tar')) as tar:
                pass

class MiscReadTestBase(CommonReadTest):
    def requires_name_attribute(self):
        pass

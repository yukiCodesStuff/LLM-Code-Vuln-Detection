                with self.assertRaisesRegex(tarfile.ReadError, "unexpected end of data"):
                    tar.extractfile(t).read()

class MiscReadTestBase(CommonReadTest):
    def requires_name_attribute(self):
        pass

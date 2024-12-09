                    str(cm.warning)
                )

    def bad_cert_test(self, certfile):
        """Check that trying to use the given client certificate fails"""
        certfile = os.path.join(os.path.dirname(__file__) or os.curdir,
                                   certfile)
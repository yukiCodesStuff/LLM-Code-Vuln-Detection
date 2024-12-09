        filename, _ = urllib.request.URLopener().retrieve(url)
        self.assertEqual(os.path.splitext(filename)[1], ".txt")


# Just commented them out.
# Can't really tell why keep failing in windows and sparc.
# Everywhere else they work ok, but on those machines, sometimes
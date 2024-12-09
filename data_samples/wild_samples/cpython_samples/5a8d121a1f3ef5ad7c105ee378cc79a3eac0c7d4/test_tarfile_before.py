                with self.assertRaisesRegex(tarfile.ReadError, "unexpected end of data"):
                    tar.extractfile(t).read()

class MiscReadTestBase(CommonReadTest):
    def requires_name_attribute(self):
        pass

    def test_no_name_argument(self):
        self.requires_name_attribute()
        with open(self.tarname, "rb") as fobj:
            self.assertIsInstance(fobj.name, str)
            with tarfile.open(fileobj=fobj, mode=self.mode) as tar:
                self.assertIsInstance(tar.name, str)
                self.assertEqual(tar.name, os.path.abspath(fobj.name))

    def test_no_name_attribute(self):
        with open(self.tarname, "rb") as fobj:
            data = fobj.read()
        fobj = io.BytesIO(data)
        self.assertRaises(AttributeError, getattr, fobj, "name")
        tar = tarfile.open(fileobj=fobj, mode=self.mode)
        self.assertIsNone(tar.name)

    def test_empty_name_attribute(self):
        with open(self.tarname, "rb") as fobj:
            data = fobj.read()
        fobj = io.BytesIO(data)
        fobj.name = ""
        with tarfile.open(fileobj=fobj, mode=self.mode) as tar:
            self.assertIsNone(tar.name)

    def test_int_name_attribute(self):
        # Issue 21044: tarfile.open() should handle fileobj with an integer
        # 'name' attribute.
        fd = os.open(self.tarname, os.O_RDONLY)
        with open(fd, 'rb') as fobj:
            self.assertIsInstance(fobj.name, int)
            with tarfile.open(fileobj=fobj, mode=self.mode) as tar:
                self.assertIsNone(tar.name)

    def test_bytes_name_attribute(self):
        self.requires_name_attribute()
        tarname = os.fsencode(self.tarname)
        with open(tarname, 'rb') as fobj:
            self.assertIsInstance(fobj.name, bytes)
            with tarfile.open(fileobj=fobj, mode=self.mode) as tar:
                self.assertIsInstance(tar.name, bytes)
                self.assertEqual(tar.name, os.path.abspath(fobj.name))

    def test_pathlike_name(self):
        tarname = pathlib.Path(self.tarname)
        with tarfile.open(tarname, mode=self.mode) as tar:
            self.assertIsInstance(tar.name, str)
            self.assertEqual(tar.name, os.path.abspath(os.fspath(tarname)))
        with self.taropen(tarname) as tar:
            self.assertIsInstance(tar.name, str)
            self.assertEqual(tar.name, os.path.abspath(os.fspath(tarname)))
        with tarfile.TarFile.open(tarname, mode=self.mode) as tar:
            self.assertIsInstance(tar.name, str)
            self.assertEqual(tar.name, os.path.abspath(os.fspath(tarname)))
        if self.suffix == '':
            with tarfile.TarFile(tarname, mode='r') as tar:
                self.assertIsInstance(tar.name, str)
                self.assertEqual(tar.name, os.path.abspath(os.fspath(tarname)))

    def test_illegal_mode_arg(self):
        with open(tmpname, 'wb'):
            pass
        with self.assertRaisesRegex(ValueError, 'mode must be '):
            tar = self.taropen(tmpname, 'q')
        with self.assertRaisesRegex(ValueError, 'mode must be '):
            tar = self.taropen(tmpname, 'rw')
        with self.assertRaisesRegex(ValueError, 'mode must be '):
            tar = self.taropen(tmpname, '')

    def test_fileobj_with_offset(self):
        # Skip the first member and store values from the second member
        # of the testtar.
        tar = tarfile.open(self.tarname, mode=self.mode)
        try:
            tar.next()
            t = tar.next()
            name = t.name
            offset = t.offset
            with tar.extractfile(t) as f:
                data = f.read()
        finally:
            tar.close()

        # Open the testtar and seek to the offset of the second member.
        with self.open(self.tarname) as fobj:
            fobj.seek(offset)

            # Test if the tarfile starts with the second member.
            with tar.open(self.tarname, mode="r:", fileobj=fobj) as tar:
                t = tar.next()
                self.assertEqual(t.name, name)
                # Read to the end of fileobj and test if seeking back to the
                # beginning works.
                tar.getmembers()
                self.assertEqual(tar.extractfile(t).read(), data,
                        "seek back did not work")

    def test_fail_comp(self):
        # For Gzip and Bz2 Tests: fail with a ReadError on an uncompressed file.
        self.assertRaises(tarfile.ReadError, tarfile.open, tarname, self.mode)
        with open(tarname, "rb") as fobj:
            self.assertRaises(tarfile.ReadError, tarfile.open,
                              fileobj=fobj, mode=self.mode)

    def test_v7_dirtype(self):
        # Test old style dirtype member (bug #1336623):
        # Old V7 tars create directory members using an AREGTYPE
        # header with a "/" appended to the filename field.
        tarinfo = self.tar.getmember("misc/dirtype-old-v7")
        self.assertEqual(tarinfo.type, tarfile.DIRTYPE,
                "v7 dirtype failed")

    def test_xstar_type(self):
        # The xstar format stores extra atime and ctime fields inside the
        # space reserved for the prefix field. The prefix field must be
        # ignored in this case, otherwise it will mess up the name.
        try:
            self.tar.getmember("misc/regtype-xstar")
        except KeyError:
            self.fail("failed to find misc/regtype-xstar (mangled prefix?)")

    def test_check_members(self):
        for tarinfo in self.tar:
            self.assertEqual(int(tarinfo.mtime), 0o7606136617,
                    "wrong mtime for %s" % tarinfo.name)
            if not tarinfo.name.startswith("ustar/"):
                continue
            self.assertEqual(tarinfo.uname, "tarfile",
                    "wrong uname for %s" % tarinfo.name)

    def test_find_members(self):
        self.assertEqual(self.tar.getmembers()[-1].name, "misc/eof",
                "could not find all members")

    @unittest.skipUnless(hasattr(os, "link"),
                         "Missing hardlink implementation")
    @support.skip_unless_symlink
    def test_extract_hardlink(self):
        # Test hardlink extraction (e.g. bug #857297).
        with tarfile.open(tarname, errorlevel=1, encoding="iso8859-1") as tar:
            tar.extract("ustar/regtype", TEMPDIR)
            self.addCleanup(support.unlink, os.path.join(TEMPDIR, "ustar/regtype"))

            tar.extract("ustar/lnktype", TEMPDIR)
            self.addCleanup(support.unlink, os.path.join(TEMPDIR, "ustar/lnktype"))
            with open(os.path.join(TEMPDIR, "ustar/lnktype"), "rb") as f:
                data = f.read()
            self.assertEqual(sha256sum(data), sha256_regtype)

            tar.extract("ustar/symtype", TEMPDIR)
            self.addCleanup(support.unlink, os.path.join(TEMPDIR, "ustar/symtype"))
            with open(os.path.join(TEMPDIR, "ustar/symtype"), "rb") as f:
                data = f.read()
            self.assertEqual(sha256sum(data), sha256_regtype)

    def test_extractall(self):
        # Test if extractall() correctly restores directory permissions
        # and times (see issue1735).
        tar = tarfile.open(tarname, encoding="iso8859-1")
        DIR = os.path.join(TEMPDIR, "extractall")
        os.mkdir(DIR)
        try:
            directories = [t for t in tar if t.isdir()]
            tar.extractall(DIR, directories)
            for tarinfo in directories:
                path = os.path.join(DIR, tarinfo.name)
                if sys.platform != "win32":
                    # Win32 has no support for fine grained permissions.
                    self.assertEqual(tarinfo.mode & 0o777,
                                     os.stat(path).st_mode & 0o777)
                def format_mtime(mtime):
                    if isinstance(mtime, float):
                        return "{} ({})".format(mtime, mtime.hex())
                    else:
                        return "{!r} (int)".format(mtime)
                file_mtime = os.path.getmtime(path)
                errmsg = "tar mtime {0} != file time {1} of path {2!a}".format(
                    format_mtime(tarinfo.mtime),
                    format_mtime(file_mtime),
                    path)
                self.assertEqual(tarinfo.mtime, file_mtime, errmsg)
        finally:
            tar.close()
            support.rmtree(DIR)

    def test_extract_directory(self):
        dirtype = "ustar/dirtype"
        DIR = os.path.join(TEMPDIR, "extractdir")
        os.mkdir(DIR)
        try:
            with tarfile.open(tarname, encoding="iso8859-1") as tar:
                tarinfo = tar.getmember(dirtype)
                tar.extract(tarinfo, path=DIR)
                extracted = os.path.join(DIR, dirtype)
                self.assertEqual(os.path.getmtime(extracted), tarinfo.mtime)
                if sys.platform != "win32":
                    self.assertEqual(os.stat(extracted).st_mode & 0o777, 0o755)
        finally:
            support.rmtree(DIR)

    def test_extractall_pathlike_name(self):
        DIR = pathlib.Path(TEMPDIR) / "extractall"
        with support.temp_dir(DIR), \
             tarfile.open(tarname, encoding="iso8859-1") as tar:
            directories = [t for t in tar if t.isdir()]
            tar.extractall(DIR, directories)
            for tarinfo in directories:
                path = DIR / tarinfo.name
                self.assertEqual(os.path.getmtime(path), tarinfo.mtime)

    def test_extract_pathlike_name(self):
        dirtype = "ustar/dirtype"
        DIR = pathlib.Path(TEMPDIR) / "extractall"
        with support.temp_dir(DIR), \
             tarfile.open(tarname, encoding="iso8859-1") as tar:
            tarinfo = tar.getmember(dirtype)
            tar.extract(tarinfo, path=DIR)
            extracted = DIR / dirtype
            self.assertEqual(os.path.getmtime(extracted), tarinfo.mtime)

    def test_init_close_fobj(self):
        # Issue #7341: Close the internal file object in the TarFile
        # constructor in case of an error. For the test we rely on
        # the fact that opening an empty file raises a ReadError.
        empty = os.path.join(TEMPDIR, "empty")
        with open(empty, "wb") as fobj:
            fobj.write(b"")

        try:
            tar = object.__new__(tarfile.TarFile)
            try:
                tar.__init__(empty)
            except tarfile.ReadError:
                self.assertTrue(tar.fileobj.closed)
            else:
                self.fail("ReadError not raised")
        finally:
            support.unlink(empty)

    def test_parallel_iteration(self):
        # Issue #16601: Restarting iteration over tarfile continued
        # from where it left off.
        with tarfile.open(self.tarname) as tar:
            for m1, m2 in zip(tar, tar):
                self.assertEqual(m1.offset, m2.offset)
                self.assertEqual(m1.get_info(), m2.get_info())

class MiscReadTest(MiscReadTestBase, unittest.TestCase):
    test_fail_comp = None

class GzipMiscReadTest(GzipTest, MiscReadTestBase, unittest.TestCase):
    pass

class Bz2MiscReadTest(Bz2Test, MiscReadTestBase, unittest.TestCase):
    def requires_name_attribute(self):
        self.skipTest("BZ2File have no name attribute")

class LzmaMiscReadTest(LzmaTest, MiscReadTestBase, unittest.TestCase):
    def requires_name_attribute(self):
        self.skipTest("LZMAFile have no name attribute")


class StreamReadTest(CommonReadTest, unittest.TestCase):

    prefix="r|"

    def test_read_through(self):
        # Issue #11224: A poorly designed _FileInFile.read() method
        # caused seeking errors with stream tar files.
        for tarinfo in self.tar:
            if not tarinfo.isreg():
                continue
            with self.tar.extractfile(tarinfo) as fobj:
                while True:
                    try:
                        buf = fobj.read(512)
                    except tarfile.StreamError:
                        self.fail("simple read-through using "
                                  "TarFile.extractfile() failed")
                    if not buf:
                        break

    def test_fileobj_regular_file(self):
        tarinfo = self.tar.next() # get "regtype" (can't use getmember)
        with self.tar.extractfile(tarinfo) as fobj:
            data = fobj.read()
        self.assertEqual(len(data), tarinfo.size,
                "regular file extraction failed")
        self.assertEqual(sha256sum(data), sha256_regtype,
                "regular file extraction failed")

    def test_provoke_stream_error(self):
        tarinfos = self.tar.getmembers()
        with self.tar.extractfile(tarinfos[0]) as f: # read the first member
            self.assertRaises(tarfile.StreamError, f.read)

    def test_compare_members(self):
        tar1 = tarfile.open(tarname, encoding="iso8859-1")
        try:
            tar2 = self.tar

            while True:
                t1 = tar1.next()
                t2 = tar2.next()
                if t1 is None:
                    break
                self.assertIsNotNone(t2, "stream.next() failed.")

                if t2.islnk() or t2.issym():
                    with self.assertRaises(tarfile.StreamError):
                        tar2.extractfile(t2)
                    continue

                v1 = tar1.extractfile(t1)
                v2 = tar2.extractfile(t2)
                if v1 is None:
                    continue
                self.assertIsNotNone(v2, "stream.extractfile() failed")
                self.assertEqual(v1.read(), v2.read(),
                        "stream extraction failed")
        finally:
            tar1.close()

class GzipStreamReadTest(GzipTest, StreamReadTest):
    pass

class Bz2StreamReadTest(Bz2Test, StreamReadTest):
    pass

class LzmaStreamReadTest(LzmaTest, StreamReadTest):
    pass


class DetectReadTest(TarTest, unittest.TestCase):
    def _testfunc_file(self, name, mode):
        try:
            tar = tarfile.open(name, mode)
        except tarfile.ReadError as e:
            self.fail()
        else:
            tar.close()

    def _testfunc_fileobj(self, name, mode):
        try:
            with open(name, "rb") as f:
                tar = tarfile.open(name, mode, fileobj=f)
        except tarfile.ReadError as e:
            self.fail()
        else:
            tar.close()

    def _test_modes(self, testfunc):
        if self.suffix:
            with self.assertRaises(tarfile.ReadError):
                tarfile.open(tarname, mode="r:" + self.suffix)
            with self.assertRaises(tarfile.ReadError):
                tarfile.open(tarname, mode="r|" + self.suffix)
            with self.assertRaises(tarfile.ReadError):
                tarfile.open(self.tarname, mode="r:")
            with self.assertRaises(tarfile.ReadError):
                tarfile.open(self.tarname, mode="r|")
        testfunc(self.tarname, "r")
        testfunc(self.tarname, "r:" + self.suffix)
        testfunc(self.tarname, "r:*")
        testfunc(self.tarname, "r|" + self.suffix)
        testfunc(self.tarname, "r|*")

    def test_detect_file(self):
        self._test_modes(self._testfunc_file)

    def test_detect_fileobj(self):
        self._test_modes(self._testfunc_fileobj)

class GzipDetectReadTest(GzipTest, DetectReadTest):
    pass

class Bz2DetectReadTest(Bz2Test, DetectReadTest):
    def test_detect_stream_bz2(self):
        # Originally, tarfile's stream detection looked for the string
        # "BZh91" at the start of the file. This is incorrect because
        # the '9' represents the blocksize (900,000 bytes). If the file was
        # compressed using another blocksize autodetection fails.
        with open(tarname, "rb") as fobj:
            data = fobj.read()

        # Compress with blocksize 100,000 bytes, the file starts with "BZh11".
        with bz2.BZ2File(tmpname, "wb", compresslevel=1) as fobj:
            fobj.write(data)

        self._testfunc_file(tmpname, "r|*")

class LzmaDetectReadTest(LzmaTest, DetectReadTest):
    pass


class MemberReadTest(ReadTest, unittest.TestCase):

    def _test_member(self, tarinfo, chksum=None, **kwargs):
        if chksum is not None:
            with self.tar.extractfile(tarinfo) as f:
                self.assertEqual(sha256sum(f.read()), chksum,
                        "wrong sha256sum for %s" % tarinfo.name)

        kwargs["mtime"] = 0o7606136617
        kwargs["uid"] = 1000
        kwargs["gid"] = 100
        if "old-v7" not in tarinfo.name:
            # V7 tar can't handle alphabetic owners.
            kwargs["uname"] = "tarfile"
            kwargs["gname"] = "tarfile"
        for k, v in kwargs.items():
            self.assertEqual(getattr(tarinfo, k), v,
                    "wrong value in %s field of %s" % (k, tarinfo.name))

    def test_find_regtype(self):
        tarinfo = self.tar.getmember("ustar/regtype")
        self._test_member(tarinfo, size=7011, chksum=sha256_regtype)

    def test_find_conttype(self):
        tarinfo = self.tar.getmember("ustar/conttype")
        self._test_member(tarinfo, size=7011, chksum=sha256_regtype)

    def test_find_dirtype(self):
        tarinfo = self.tar.getmember("ustar/dirtype")
        self._test_member(tarinfo, size=0)

    def test_find_dirtype_with_size(self):
        tarinfo = self.tar.getmember("ustar/dirtype-with-size")
        self._test_member(tarinfo, size=255)

    def test_find_lnktype(self):
        tarinfo = self.tar.getmember("ustar/lnktype")
        self._test_member(tarinfo, size=0, linkname="ustar/regtype")

    def test_find_symtype(self):
        tarinfo = self.tar.getmember("ustar/symtype")
        self._test_member(tarinfo, size=0, linkname="regtype")

    def test_find_blktype(self):
        tarinfo = self.tar.getmember("ustar/blktype")
        self._test_member(tarinfo, size=0, devmajor=3, devminor=0)

    def test_find_chrtype(self):
        tarinfo = self.tar.getmember("ustar/chrtype")
        self._test_member(tarinfo, size=0, devmajor=1, devminor=3)

    def test_find_fifotype(self):
        tarinfo = self.tar.getmember("ustar/fifotype")
        self._test_member(tarinfo, size=0)

    def test_find_sparse(self):
        tarinfo = self.tar.getmember("ustar/sparse")
        self._test_member(tarinfo, size=86016, chksum=sha256_sparse)

    def test_find_gnusparse(self):
        tarinfo = self.tar.getmember("gnu/sparse")
        self._test_member(tarinfo, size=86016, chksum=sha256_sparse)

    def test_find_gnusparse_00(self):
        tarinfo = self.tar.getmember("gnu/sparse-0.0")
        self._test_member(tarinfo, size=86016, chksum=sha256_sparse)

    def test_find_gnusparse_01(self):
        tarinfo = self.tar.getmember("gnu/sparse-0.1")
        self._test_member(tarinfo, size=86016, chksum=sha256_sparse)

    def test_find_gnusparse_10(self):
        tarinfo = self.tar.getmember("gnu/sparse-1.0")
        self._test_member(tarinfo, size=86016, chksum=sha256_sparse)

    def test_find_umlauts(self):
        tarinfo = self.tar.getmember("ustar/umlauts-"
                                     "\xc4\xd6\xdc\xe4\xf6\xfc\xdf")
        self._test_member(tarinfo, size=7011, chksum=sha256_regtype)

    def test_find_ustar_longname(self):
        name = "ustar/" + "12345/" * 39 + "1234567/longname"
        self.assertIn(name, self.tar.getnames())

    def test_find_regtype_oldv7(self):
        tarinfo = self.tar.getmember("misc/regtype-old-v7")
        self._test_member(tarinfo, size=7011, chksum=sha256_regtype)

    def test_find_pax_umlauts(self):
        self.tar.close()
        self.tar = tarfile.open(self.tarname, mode=self.mode,
                                encoding="iso8859-1")
        tarinfo = self.tar.getmember("pax/umlauts-"
                                     "\xc4\xd6\xdc\xe4\xf6\xfc\xdf")
        self._test_member(tarinfo, size=7011, chksum=sha256_regtype)


class LongnameTest:

    def test_read_longname(self):
        # Test reading of longname (bug #1471427).
        longname = self.subdir + "/" + "123/" * 125 + "longname"
        try:
            tarinfo = self.tar.getmember(longname)
        except KeyError:
            self.fail("longname not found")
        self.assertNotEqual(tarinfo.type, tarfile.DIRTYPE,
                "read longname as dirtype")

    def test_read_longlink(self):
        longname = self.subdir + "/" + "123/" * 125 + "longname"
        longlink = self.subdir + "/" + "123/" * 125 + "longlink"
        try:
            tarinfo = self.tar.getmember(longlink)
        except KeyError:
            self.fail("longlink not found")
        self.assertEqual(tarinfo.linkname, longname, "linkname wrong")

    def test_truncated_longname(self):
        longname = self.subdir + "/" + "123/" * 125 + "longname"
        tarinfo = self.tar.getmember(longname)
        offset = tarinfo.offset
        self.tar.fileobj.seek(offset)
        fobj = io.BytesIO(self.tar.fileobj.read(3 * 512))
        with self.assertRaises(tarfile.ReadError):
            tarfile.open(name="foo.tar", fileobj=fobj)

    def test_header_offset(self):
        # Test if the start offset of the TarInfo object includes
        # the preceding extended header.
        longname = self.subdir + "/" + "123/" * 125 + "longname"
        offset = self.tar.getmember(longname).offset
        with open(tarname, "rb") as fobj:
            fobj.seek(offset)
            tarinfo = tarfile.TarInfo.frombuf(fobj.read(512),
                                              "iso8859-1", "strict")
            self.assertEqual(tarinfo.type, self.longnametype)


class GNUReadTest(LongnameTest, ReadTest, unittest.TestCase):

    subdir = "gnu"
    longnametype = tarfile.GNUTYPE_LONGNAME

    # Since 3.2 tarfile is supposed to accurately restore sparse members and
    # produce files with holes. This is what we actually want to test here.
    # Unfortunately, not all platforms/filesystems support sparse files, and
    # even on platforms that do it is non-trivial to make reliable assertions
    # about holes in files. Therefore, we first do one basic test which works
    # an all platforms, and after that a test that will work only on
    # platforms/filesystems that prove to support sparse files.
    def _test_sparse_file(self, name):
        self.tar.extract(name, TEMPDIR)
        filename = os.path.join(TEMPDIR, name)
        with open(filename, "rb") as fobj:
            data = fobj.read()
        self.assertEqual(sha256sum(data), sha256_sparse,
                "wrong sha256sum for %s" % name)

        if self._fs_supports_holes():
            s = os.stat(filename)
            self.assertLess(s.st_blocks * 512, s.st_size)

    def test_sparse_file_old(self):
        self._test_sparse_file("gnu/sparse")

    def test_sparse_file_00(self):
        self._test_sparse_file("gnu/sparse-0.0")

    def test_sparse_file_01(self):
        self._test_sparse_file("gnu/sparse-0.1")

    def test_sparse_file_10(self):
        self._test_sparse_file("gnu/sparse-1.0")

    @staticmethod
    def _fs_supports_holes():
        # Return True if the platform knows the st_blocks stat attribute and
        # uses st_blocks units of 512 bytes, and if the filesystem is able to
        # store holes of 4 KiB in files.
        #
        # The function returns False if page size is larger than 4 KiB.
        # For example, ppc64 uses pages of 64 KiB.
        if sys.platform.startswith("linux"):
            # Linux evidentially has 512 byte st_blocks units.
            name = os.path.join(TEMPDIR, "sparse-test")
            with open(name, "wb") as fobj:
                # Seek to "punch a hole" of 4 KiB
                fobj.seek(4096)
                fobj.write(b'x' * 4096)
                fobj.truncate()
            s = os.stat(name)
            support.unlink(name)
            return (s.st_blocks * 512 < s.st_size)
        else:
            return False


class PaxReadTest(LongnameTest, ReadTest, unittest.TestCase):

    subdir = "pax"
    longnametype = tarfile.XHDTYPE

    def test_pax_global_headers(self):
        tar = tarfile.open(tarname, encoding="iso8859-1")
        try:
            tarinfo = tar.getmember("pax/regtype1")
            self.assertEqual(tarinfo.uname, "foo")
            self.assertEqual(tarinfo.gname, "bar")
            self.assertEqual(tarinfo.pax_headers.get("VENDOR.umlauts"),
                             "\xc4\xd6\xdc\xe4\xf6\xfc\xdf")

            tarinfo = tar.getmember("pax/regtype2")
            self.assertEqual(tarinfo.uname, "")
            self.assertEqual(tarinfo.gname, "bar")
            self.assertEqual(tarinfo.pax_headers.get("VENDOR.umlauts"),
                             "\xc4\xd6\xdc\xe4\xf6\xfc\xdf")

            tarinfo = tar.getmember("pax/regtype3")
            self.assertEqual(tarinfo.uname, "tarfile")
            self.assertEqual(tarinfo.gname, "tarfile")
            self.assertEqual(tarinfo.pax_headers.get("VENDOR.umlauts"),
                             "\xc4\xd6\xdc\xe4\xf6\xfc\xdf")
        finally:
            tar.close()

    def test_pax_number_fields(self):
        # All following number fields are read from the pax header.
        tar = tarfile.open(tarname, encoding="iso8859-1")
        try:
            tarinfo = tar.getmember("pax/regtype4")
            self.assertEqual(tarinfo.size, 7011)
            self.assertEqual(tarinfo.uid, 123)
            self.assertEqual(tarinfo.gid, 123)
            self.assertEqual(tarinfo.mtime, 1041808783.0)
            self.assertEqual(type(tarinfo.mtime), float)
            self.assertEqual(float(tarinfo.pax_headers["atime"]), 1041808783.0)
            self.assertEqual(float(tarinfo.pax_headers["ctime"]), 1041808783.0)
        finally:
            tar.close()


class WriteTestBase(TarTest):
    # Put all write tests in here that are supposed to be tested
    # in all possible mode combinations.

    def test_fileobj_no_close(self):
        fobj = io.BytesIO()
        with tarfile.open(fileobj=fobj, mode=self.mode) as tar:
            tar.addfile(tarfile.TarInfo("foo"))
        self.assertFalse(fobj.closed, "external fileobjs must never closed")
        # Issue #20238: Incomplete gzip output with mode="w:gz"
        data = fobj.getvalue()
        del tar
        support.gc_collect()
        self.assertFalse(fobj.closed)
        self.assertEqual(data, fobj.getvalue())

    def test_eof_marker(self):
        # Make sure an end of archive marker is written (two zero blocks).
        # tarfile insists on aligning archives to a 20 * 512 byte recordsize.
        # So, we create an archive that has exactly 10240 bytes without the
        # marker, and has 20480 bytes once the marker is written.
        with tarfile.open(tmpname, self.mode) as tar:
            t = tarfile.TarInfo("foo")
            t.size = tarfile.RECORDSIZE - tarfile.BLOCKSIZE
            tar.addfile(t, io.BytesIO(b"a" * t.size))

        with self.open(tmpname, "rb") as fobj:
            self.assertEqual(len(fobj.read()), tarfile.RECORDSIZE * 2)


class WriteTest(WriteTestBase, unittest.TestCase):

    prefix = "w:"

    def test_100_char_name(self):
        # The name field in a tar header stores strings of at most 100 chars.
        # If a string is shorter than 100 chars it has to be padded with '\0',
        # which implies that a string of exactly 100 chars is stored without
        # a trailing '\0'.
        name = "0123456789" * 10
        tar = tarfile.open(tmpname, self.mode)
        try:
            t = tarfile.TarInfo(name)
            tar.addfile(t)
        finally:
            tar.close()

        tar = tarfile.open(tmpname)
        try:
            self.assertEqual(tar.getnames()[0], name,
                    "failed to store 100 char filename")
        finally:
            tar.close()

    def test_tar_size(self):
        # Test for bug #1013882.
        tar = tarfile.open(tmpname, self.mode)
        try:
            path = os.path.join(TEMPDIR, "file")
            with open(path, "wb") as fobj:
                fobj.write(b"aaa")
            tar.add(path)
        finally:
            tar.close()
        self.assertGreater(os.path.getsize(tmpname), 0,
                "tarfile is empty")

    # The test_*_size tests test for bug #1167128.
    def test_file_size(self):
        tar = tarfile.open(tmpname, self.mode)
        try:
            path = os.path.join(TEMPDIR, "file")
            with open(path, "wb"):
                pass
            tarinfo = tar.gettarinfo(path)
            self.assertEqual(tarinfo.size, 0)

            with open(path, "wb") as fobj:
                fobj.write(b"aaa")
            tarinfo = tar.gettarinfo(path)
            self.assertEqual(tarinfo.size, 3)
        finally:
            tar.close()

    def test_directory_size(self):
        path = os.path.join(TEMPDIR, "directory")
        os.mkdir(path)
        try:
            tar = tarfile.open(tmpname, self.mode)
            try:
                tarinfo = tar.gettarinfo(path)
                self.assertEqual(tarinfo.size, 0)
            finally:
                tar.close()
        finally:
            support.rmdir(path)

    # mock the following:
    #  os.listdir: so we know that files are in the wrong order
    def test_ordered_recursion(self):
        path = os.path.join(TEMPDIR, "directory")
        os.mkdir(path)
        open(os.path.join(path, "1"), "a").close()
        open(os.path.join(path, "2"), "a").close()
        try:
            tar = tarfile.open(tmpname, self.mode)
            try:
                with unittest.mock.patch('os.listdir') as mock_listdir:
                    mock_listdir.return_value = ["2", "1"]
                    tar.add(path)
                paths = []
                for m in tar.getmembers():
                    paths.append(os.path.split(m.name)[-1])
                self.assertEqual(paths, ["directory", "1", "2"]);
            finally:
                tar.close()
        finally:
            support.unlink(os.path.join(path, "1"))
            support.unlink(os.path.join(path, "2"))
            support.rmdir(path)

    def test_gettarinfo_pathlike_name(self):
        with tarfile.open(tmpname, self.mode) as tar:
            path = pathlib.Path(TEMPDIR) / "file"
            with open(path, "wb") as fobj:
                fobj.write(b"aaa")
            tarinfo = tar.gettarinfo(path)
            tarinfo2 = tar.gettarinfo(os.fspath(path))
            self.assertIsInstance(tarinfo.name, str)
            self.assertEqual(tarinfo.name, tarinfo2.name)
            self.assertEqual(tarinfo.size, 3)

    @unittest.skipUnless(hasattr(os, "link"),
                         "Missing hardlink implementation")
    def test_link_size(self):
        link = os.path.join(TEMPDIR, "link")
        target = os.path.join(TEMPDIR, "link_target")
        with open(target, "wb") as fobj:
            fobj.write(b"aaa")
        try:
            os.link(target, link)
        except PermissionError as e:
            self.skipTest('os.link(): %s' % e)
        try:
            tar = tarfile.open(tmpname, self.mode)
            try:
                # Record the link target in the inodes list.
                tar.gettarinfo(target)
                tarinfo = tar.gettarinfo(link)
                self.assertEqual(tarinfo.size, 0)
            finally:
                tar.close()
        finally:
            support.unlink(target)
            support.unlink(link)

    @support.skip_unless_symlink
    def test_symlink_size(self):
        path = os.path.join(TEMPDIR, "symlink")
        os.symlink("link_target", path)
        try:
            tar = tarfile.open(tmpname, self.mode)
            try:
                tarinfo = tar.gettarinfo(path)
                self.assertEqual(tarinfo.size, 0)
            finally:
                tar.close()
        finally:
            support.unlink(path)

    def test_add_self(self):
        # Test for #1257255.
        dstname = os.path.abspath(tmpname)
        tar = tarfile.open(tmpname, self.mode)
        try:
            self.assertEqual(tar.name, dstname,
                    "archive name must be absolute")
            tar.add(dstname)
            self.assertEqual(tar.getnames(), [],
                    "added the archive to itself")

            with support.change_cwd(TEMPDIR):
                tar.add(dstname)
            self.assertEqual(tar.getnames(), [],
                    "added the archive to itself")
        finally:
            tar.close()

    def test_filter(self):
        tempdir = os.path.join(TEMPDIR, "filter")
        os.mkdir(tempdir)
        try:
            for name in ("foo", "bar", "baz"):
                name = os.path.join(tempdir, name)
                support.create_empty_file(name)

            def filter(tarinfo):
                if os.path.basename(tarinfo.name) == "bar":
                    return
                tarinfo.uid = 123
                tarinfo.uname = "foo"
                return tarinfo

            tar = tarfile.open(tmpname, self.mode, encoding="iso8859-1")
            try:
                tar.add(tempdir, arcname="empty_dir", filter=filter)
            finally:
                tar.close()

            # Verify that filter is a keyword-only argument
            with self.assertRaises(TypeError):
                tar.add(tempdir, "empty_dir", True, None, filter)

            tar = tarfile.open(tmpname, "r")
            try:
                for tarinfo in tar:
                    self.assertEqual(tarinfo.uid, 123)
                    self.assertEqual(tarinfo.uname, "foo")
                self.assertEqual(len(tar.getmembers()), 3)
            finally:
                tar.close()
        finally:
            support.rmtree(tempdir)

    # Guarantee that stored pathnames are not modified. Don't
    # remove ./ or ../ or double slashes. Still make absolute
    # pathnames relative.
    # For details see bug #6054.
    def _test_pathname(self, path, cmp_path=None, dir=False):
        # Create a tarfile with an empty member named path
        # and compare the stored name with the original.
        foo = os.path.join(TEMPDIR, "foo")
        if not dir:
            support.create_empty_file(foo)
        else:
            os.mkdir(foo)

        tar = tarfile.open(tmpname, self.mode)
        try:
            tar.add(foo, arcname=path)
        finally:
            tar.close()

        tar = tarfile.open(tmpname, "r")
        try:
            t = tar.next()
        finally:
            tar.close()

        if not dir:
            support.unlink(foo)
        else:
            support.rmdir(foo)

        self.assertEqual(t.name, cmp_path or path.replace(os.sep, "/"))


    @support.skip_unless_symlink
    def test_extractall_symlinks(self):
        # Test if extractall works properly when tarfile contains symlinks
        tempdir = os.path.join(TEMPDIR, "testsymlinks")
        temparchive = os.path.join(TEMPDIR, "testsymlinks.tar")
        os.mkdir(tempdir)
        try:
            source_file = os.path.join(tempdir,'source')
            target_file = os.path.join(tempdir,'symlink')
            with open(source_file,'w') as f:
                f.write('something\n')
            os.symlink(source_file, target_file)
            with tarfile.open(temparchive, 'w') as tar:
                tar.add(source_file)
                tar.add(target_file)
            # Let's extract it to the location which contains the symlink
            with tarfile.open(temparchive) as tar:
                # this should not raise OSError: [Errno 17] File exists
                try:
                    tar.extractall(path=tempdir)
                except OSError:
                    self.fail("extractall failed with symlinked files")
        finally:
            support.unlink(temparchive)
            support.rmtree(tempdir)

    def test_pathnames(self):
        self._test_pathname("foo")
        self._test_pathname(os.path.join("foo", ".", "bar"))
        self._test_pathname(os.path.join("foo", "..", "bar"))
        self._test_pathname(os.path.join(".", "foo"))
        self._test_pathname(os.path.join(".", "foo", "."))
        self._test_pathname(os.path.join(".", "foo", ".", "bar"))
        self._test_pathname(os.path.join(".", "foo", "..", "bar"))
        self._test_pathname(os.path.join(".", "foo", "..", "bar"))
        self._test_pathname(os.path.join("..", "foo"))
        self._test_pathname(os.path.join("..", "foo", ".."))
        self._test_pathname(os.path.join("..", "foo", ".", "bar"))
        self._test_pathname(os.path.join("..", "foo", "..", "bar"))

        self._test_pathname("foo" + os.sep + os.sep + "bar")
        self._test_pathname("foo" + os.sep + os.sep, "foo", dir=True)

    def test_abs_pathnames(self):
        if sys.platform == "win32":
            self._test_pathname("C:\\foo", "foo")
        else:
            self._test_pathname("/foo", "foo")
            self._test_pathname("///foo", "foo")

    def test_cwd(self):
        # Test adding the current working directory.
        with support.change_cwd(TEMPDIR):
            tar = tarfile.open(tmpname, self.mode)
            try:
                tar.add(".")
            finally:
                tar.close()

            tar = tarfile.open(tmpname, "r")
            try:
                for t in tar:
                    if t.name != ".":
                        self.assertTrue(t.name.startswith("./"), t.name)
            finally:
                tar.close()

    def test_open_nonwritable_fileobj(self):
        for exctype in OSError, EOFError, RuntimeError:
            class BadFile(io.BytesIO):
                first = True
                def write(self, data):
                    if self.first:
                        self.first = False
                        raise exctype

            f = BadFile()
            with self.assertRaises(exctype):
                tar = tarfile.open(tmpname, self.mode, fileobj=f,
                                   format=tarfile.PAX_FORMAT,
                                   pax_headers={'non': 'empty'})
            self.assertFalse(f.closed)

class GzipWriteTest(GzipTest, WriteTest):
    pass

class Bz2WriteTest(Bz2Test, WriteTest):
    pass

class LzmaWriteTest(LzmaTest, WriteTest):
    pass


class StreamWriteTest(WriteTestBase, unittest.TestCase):

    prefix = "w|"
    decompressor = None

    def test_stream_padding(self):
        # Test for bug #1543303.
        tar = tarfile.open(tmpname, self.mode)
        tar.close()
        if self.decompressor:
            dec = self.decompressor()
            with open(tmpname, "rb") as fobj:
                data = fobj.read()
            data = dec.decompress(data)
            self.assertFalse(dec.unused_data, "found trailing data")
        else:
            with self.open(tmpname) as fobj:
                data = fobj.read()
        self.assertEqual(data.count(b"\0"), tarfile.RECORDSIZE,
                        "incorrect zero padding")

    @unittest.skipUnless(sys.platform != "win32" and hasattr(os, "umask"),
                         "Missing umask implementation")
    def test_file_mode(self):
        # Test for issue #8464: Create files with correct
        # permissions.
        if os.path.exists(tmpname):
            support.unlink(tmpname)

        original_umask = os.umask(0o022)
        try:
            tar = tarfile.open(tmpname, self.mode)
            tar.close()
            mode = os.stat(tmpname).st_mode & 0o777
            self.assertEqual(mode, 0o644, "wrong file permissions")
        finally:
            os.umask(original_umask)

class GzipStreamWriteTest(GzipTest, StreamWriteTest):
    pass

class Bz2StreamWriteTest(Bz2Test, StreamWriteTest):
    decompressor = bz2.BZ2Decompressor if bz2 else None

class LzmaStreamWriteTest(LzmaTest, StreamWriteTest):
    decompressor = lzma.LZMADecompressor if lzma else None


class GNUWriteTest(unittest.TestCase):
    # This testcase checks for correct creation of GNU Longname
    # and Longlink extended headers (cp. bug #812325).

    def _length(self, s):
        blocks = len(s) // 512 + 1
        return blocks * 512

    def _calc_size(self, name, link=None):
        # Initial tar header
        count = 512

        if len(name) > tarfile.LENGTH_NAME:
            # GNU longname extended header + longname
            count += 512
            count += self._length(name)
        if link is not None and len(link) > tarfile.LENGTH_LINK:
            # GNU longlink extended header + longlink
            count += 512
            count += self._length(link)
        return count

    def _test(self, name, link=None):
        tarinfo = tarfile.TarInfo(name)
        if link:
            tarinfo.linkname = link
            tarinfo.type = tarfile.LNKTYPE

        tar = tarfile.open(tmpname, "w")
        try:
            tar.format = tarfile.GNU_FORMAT
            tar.addfile(tarinfo)

            v1 = self._calc_size(name, link)
            v2 = tar.offset
            self.assertEqual(v1, v2, "GNU longname/longlink creation failed")
        finally:
            tar.close()

        tar = tarfile.open(tmpname)
        try:
            member = tar.next()
            self.assertIsNotNone(member,
                    "unable to read longname member")
            self.assertEqual(tarinfo.name, member.name,
                    "unable to read longname member")
            self.assertEqual(tarinfo.linkname, member.linkname,
                    "unable to read longname member")
        finally:
            tar.close()

    def test_longname_1023(self):
        self._test(("longnam/" * 127) + "longnam")

    def test_longname_1024(self):
        self._test(("longnam/" * 127) + "longname")

    def test_longname_1025(self):
        self._test(("longnam/" * 127) + "longname_")

    def test_longlink_1023(self):
        self._test("name", ("longlnk/" * 127) + "longlnk")

    def test_longlink_1024(self):
        self._test("name", ("longlnk/" * 127) + "longlink")

    def test_longlink_1025(self):
        self._test("name", ("longlnk/" * 127) + "longlink_")

    def test_longnamelink_1023(self):
        self._test(("longnam/" * 127) + "longnam",
                   ("longlnk/" * 127) + "longlnk")

    def test_longnamelink_1024(self):
        self._test(("longnam/" * 127) + "longname",
                   ("longlnk/" * 127) + "longlink")

    def test_longnamelink_1025(self):
        self._test(("longnam/" * 127) + "longname_",
                   ("longlnk/" * 127) + "longlink_")


class DeviceHeaderTest(WriteTestBase, unittest.TestCase):

    prefix = "w:"

    def test_headers_written_only_for_device_files(self):
        # Regression test for bpo-18819.
        tempdir = os.path.join(TEMPDIR, "device_header_test")
        os.mkdir(tempdir)
        try:
            tar = tarfile.open(tmpname, self.mode)
            try:
                input_blk = tarfile.TarInfo(name="my_block_device")
                input_reg = tarfile.TarInfo(name="my_regular_file")
                input_blk.type = tarfile.BLKTYPE
                input_reg.type = tarfile.REGTYPE
                tar.addfile(input_blk)
                tar.addfile(input_reg)
            finally:
                tar.close()

            # devmajor and devminor should be *interpreted* as 0 in both...
            tar = tarfile.open(tmpname, "r")
            try:
                output_blk = tar.getmember("my_block_device")
                output_reg = tar.getmember("my_regular_file")
            finally:
                tar.close()
            self.assertEqual(output_blk.devmajor, 0)
            self.assertEqual(output_blk.devminor, 0)
            self.assertEqual(output_reg.devmajor, 0)
            self.assertEqual(output_reg.devminor, 0)

            # ...but the fields should not actually be set on regular files:
            with open(tmpname, "rb") as infile:
                buf = infile.read()
            buf_blk = buf[output_blk.offset:output_blk.offset_data]
            buf_reg = buf[output_reg.offset:output_reg.offset_data]
            # See `struct posixheader` in GNU docs for byte offsets:
            # <https://www.gnu.org/software/tar/manual/html_node/Standard.html>
            device_headers = slice(329, 329 + 16)
            self.assertEqual(buf_blk[device_headers], b"0000000\0" * 2)
            self.assertEqual(buf_reg[device_headers], b"\0" * 16)
        finally:
            support.rmtree(tempdir)


class CreateTest(WriteTestBase, unittest.TestCase):

    prefix = "x:"

    file_path = os.path.join(TEMPDIR, "spameggs42")

    def setUp(self):
        support.unlink(tmpname)

    @classmethod
    def setUpClass(cls):
        with open(cls.file_path, "wb") as fobj:
            fobj.write(b"aaa")

    @classmethod
    def tearDownClass(cls):
        support.unlink(cls.file_path)

    def test_create(self):
        with tarfile.open(tmpname, self.mode) as tobj:
            tobj.add(self.file_path)

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

    def test_create_existing(self):
        with tarfile.open(tmpname, self.mode) as tobj:
            tobj.add(self.file_path)

        with self.assertRaises(FileExistsError):
            tobj = tarfile.open(tmpname, self.mode)

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

    def test_create_taropen(self):
        with self.taropen(tmpname, "x") as tobj:
            tobj.add(self.file_path)

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

    def test_create_existing_taropen(self):
        with self.taropen(tmpname, "x") as tobj:
            tobj.add(self.file_path)

        with self.assertRaises(FileExistsError):
            with self.taropen(tmpname, "x"):
                pass

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn("spameggs42", names[0])

    def test_create_pathlike_name(self):
        with tarfile.open(pathlib.Path(tmpname), self.mode) as tobj:
            self.assertIsInstance(tobj.name, str)
            self.assertEqual(tobj.name, os.path.abspath(tmpname))
            tobj.add(pathlib.Path(self.file_path))
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

    def test_create_taropen_pathlike_name(self):
        with self.taropen(pathlib.Path(tmpname), "x") as tobj:
            self.assertIsInstance(tobj.name, str)
            self.assertEqual(tobj.name, os.path.abspath(tmpname))
            tobj.add(pathlib.Path(self.file_path))
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])

        with self.taropen(tmpname) as tobj:
            names = tobj.getnames()
        self.assertEqual(len(names), 1)
        self.assertIn('spameggs42', names[0])


class GzipCreateTest(GzipTest, CreateTest):
    pass


class Bz2CreateTest(Bz2Test, CreateTest):
    pass


class LzmaCreateTest(LzmaTest, CreateTest):
    pass


class CreateWithXModeTest(CreateTest):

    prefix = "x"

    test_create_taropen = None
    test_create_existing_taropen = None


@unittest.skipUnless(hasattr(os, "link"), "Missing hardlink implementation")
class HardlinkTest(unittest.TestCase):
    # Test the creation of LNKTYPE (hardlink) members in an archive.

    def setUp(self):
        self.foo = os.path.join(TEMPDIR, "foo")
        self.bar = os.path.join(TEMPDIR, "bar")

        with open(self.foo, "wb") as fobj:
            fobj.write(b"foo")

        try:
            os.link(self.foo, self.bar)
        except PermissionError as e:
            self.skipTest('os.link(): %s' % e)

        self.tar = tarfile.open(tmpname, "w")
        self.tar.add(self.foo)

    def tearDown(self):
        self.tar.close()
        support.unlink(self.foo)
        support.unlink(self.bar)

    def test_add_twice(self):
        # The same name will be added as a REGTYPE every
        # time regardless of st_nlink.
        tarinfo = self.tar.gettarinfo(self.foo)
        self.assertEqual(tarinfo.type, tarfile.REGTYPE,
                "add file as regular failed")

    def test_add_hardlink(self):
        tarinfo = self.tar.gettarinfo(self.bar)
        self.assertEqual(tarinfo.type, tarfile.LNKTYPE,
                "add file as hardlink failed")

    def test_dereference_hardlink(self):
        self.tar.dereference = True
        tarinfo = self.tar.gettarinfo(self.bar)
        self.assertEqual(tarinfo.type, tarfile.REGTYPE,
                "dereferencing hardlink failed")


class PaxWriteTest(GNUWriteTest):

    def _test(self, name, link=None):
        # See GNUWriteTest.
        tarinfo = tarfile.TarInfo(name)
        if link:
            tarinfo.linkname = link
            tarinfo.type = tarfile.LNKTYPE

        tar = tarfile.open(tmpname, "w", format=tarfile.PAX_FORMAT)
        try:
            tar.addfile(tarinfo)
        finally:
            tar.close()

        tar = tarfile.open(tmpname)
        try:
            if link:
                l = tar.getmembers()[0].linkname
                self.assertEqual(link, l, "PAX longlink creation failed")
            else:
                n = tar.getmembers()[0].name
                self.assertEqual(name, n, "PAX longname creation failed")
        finally:
            tar.close()

    def test_pax_global_header(self):
        pax_headers = {
                "foo": "bar",
                "uid": "0",
                "mtime": "1.23",
                "test": "\xe4\xf6\xfc",
                "\xe4\xf6\xfc": "test"}
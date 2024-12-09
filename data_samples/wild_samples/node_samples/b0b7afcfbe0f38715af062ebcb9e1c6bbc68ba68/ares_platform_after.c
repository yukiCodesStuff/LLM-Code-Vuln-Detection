{
  OSVERSIONINFOEX OsvEx;

  memset(&OsvEx, 0, sizeof(OsvEx));
  OsvEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996) /* warning C4996: 'GetVersionExW': was declared deprecated */
#endif
  if (!GetVersionEx((void *)&OsvEx))
    {
      memset(&OsvEx, 0, sizeof(OsvEx));
      OsvEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if (!GetVersionEx((void *)&OsvEx))
        return WIN_UNKNOWN;
    }
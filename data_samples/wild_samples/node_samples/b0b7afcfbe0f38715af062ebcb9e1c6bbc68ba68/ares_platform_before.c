{
  OSVERSIONINFOEX OsvEx;

  memset(&OsvEx, 0, sizeof(OsvEx));
  OsvEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if (!GetVersionEx((void *)&OsvEx))
    {
      memset(&OsvEx, 0, sizeof(OsvEx));
      OsvEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if (!GetVersionEx((void *)&OsvEx))
        return WIN_UNKNOWN;
    }
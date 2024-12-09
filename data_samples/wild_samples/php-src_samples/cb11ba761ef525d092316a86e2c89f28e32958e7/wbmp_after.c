{
  int i;

  Wbmp *wbmp;
  if ((wbmp = (Wbmp *) gdMalloc (sizeof (Wbmp))) == NULL)
    return (NULL);

  if (overflow2(sizeof (int), width)) {
    gdFree(wbmp);
    return NULL;
  }
    {
      gdFree (wbmp);
      return (-1);
    }

#ifdef __DEBUG
  printf ("W: %d, H: %d\n", wbmp->width, wbmp->height);
#endif

  if (overflow2(sizeof (int), wbmp->width) ||
    overflow2(sizeof (int) * wbmp->width, wbmp->height))
    {
      gdFree(wbmp);
      return (-1);
    }
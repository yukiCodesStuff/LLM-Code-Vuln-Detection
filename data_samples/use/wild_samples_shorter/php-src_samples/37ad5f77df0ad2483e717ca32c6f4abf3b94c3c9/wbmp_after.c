  if ((wbmp = (Wbmp *) gdMalloc (sizeof (Wbmp))) == NULL)
    return (NULL);

  if (overflow2(sizeof (int), width)) {
    gdFree(wbmp);
    return NULL;
  }
  if (overflow2(sizeof (int) * width, height)) {
    gdFree(wbmp);
    return NULL;
  }

  if ((wbmp->bitmap = (int *) safe_emalloc(sizeof(int), width * height, 0)) == NULL)
    {
      gdFree (wbmp);
      return (NULL);
  printf ("W: %d, H: %d\n", wbmp->width, wbmp->height);
#endif

  if (overflow2(sizeof (int), wbmp->width) ||
    overflow2(sizeof (int) * wbmp->width, wbmp->height))
    {
      gdFree(wbmp);
      return (-1);
    }

  if ((wbmp->bitmap = (int *) safe_emalloc(wbmp->width * wbmp->height, sizeof(int), 0)) == NULL)
    {
      gdFree (wbmp);
      return (-1);
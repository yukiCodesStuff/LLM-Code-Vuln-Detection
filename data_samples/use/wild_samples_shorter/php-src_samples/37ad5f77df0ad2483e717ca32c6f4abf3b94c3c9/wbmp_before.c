  if ((wbmp = (Wbmp *) gdMalloc (sizeof (Wbmp))) == NULL)
    return (NULL);

  if ((wbmp->bitmap = (int *) safe_emalloc(sizeof(int), width * height, 0)) == NULL)
    {
      gdFree (wbmp);
      return (NULL);
  printf ("W: %d, H: %d\n", wbmp->width, wbmp->height);
#endif

  if ((wbmp->bitmap = (int *) safe_emalloc(wbmp->width * wbmp->height, sizeof(int), 0)) == NULL)
    {
      gdFree (wbmp);
      return (-1);
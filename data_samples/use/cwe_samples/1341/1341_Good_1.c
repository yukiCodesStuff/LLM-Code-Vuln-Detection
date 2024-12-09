
char b[2000];
int f_flg = 0;
FILE *f = fopen("dbl_cls.c", "r");
if (f)
{
	f_flg = 1;
	b[0] = 0;
	fread(b, 1, sizeof(b) - 1, f);
	printf("%s\n'", b);
	if (f_flg)
	{
		int r1 = fclose(f);
		f_flg = 0;
		printf("\n-----------------\n1 close done '%d'\n", r1);
	}
	if (f_flg)
	{
		int r2 = fclose(f);	// Double close
		f_flg = 0;
		printf("2 close done '%d'\n", r2);
	}
}
					
				
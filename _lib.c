
#ifdef NEED_MEMSET
void *memset(void *dest, int c, size_t count)
{
	char *bytes = (char *)dest;
	while (count--)
	{
		*bytes++ = (char)c;
	}
	return dest;
}
#endif


#ifdef NEED_STRCHR
char *
strchr (s, c)
	const char *s;
	int c;
{
  for (;;)
    {
      if (*s == c)
	return (char *) s;
      if (*s == 0)
	return 0;
      s++;
    }
}
#endif


#ifdef NEED_STRPBRK
char *
strpbrk(const char *s1, const char *s2)
{
  const char *p;
  while (*s1)
    {
      for (p = s2; *p; p++)
	if (*s1 == *p)
	  return (char *)s1;
      s1++;
    }
  return 0;
}
#endif


#ifdef NEED_STRLEN
size_t
strlen (const char *s)
{
  size_t i;

  i = 0;
  while (s[i] != 0)
    i++;

  return i;
}
#endif

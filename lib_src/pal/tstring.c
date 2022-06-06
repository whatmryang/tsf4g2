/*
**  @file $RCSfile: tstring.c,v $
**  general description of this module
**  $Id: tstring.c,v 1.1 2009-01-23 09:35:03 kent Exp $
**  @author $Author: kent $
**  @date $Date: 2009-01-23 09:35:03 $
**  @version $Revision: 1.1 $
**  @note Editor: Vim 6.1, Gcc 4.0.1, tab=4
**  @note Platform: Linux
*/
#include "pal/pal.h"
#include "pal/tstring.h"

#if defined (_WIN32) || defined (_WIN64)

void bzero(void *s, size_t n)
{
	memset(s, 0, n);
}

void *tmem_char(char *buff ,char c)
{
	while(*buff != c)
		buff ++;
	return buff;
}

char * strtok_r (char *s, const char *delim, char **save_ptr)
{
  char *token;

  if (s == NULL)
    s = *save_ptr;

  /* Scan leading delimiters.  */
  s += strspn (s, delim);
  if (*s == '\0')
    {
      *save_ptr = s;
      return NULL;
    }

  /* Find the end of the token.  */
  token = s;
  s = strpbrk (token, delim);
  if (s == NULL)
    /* This token finishes the string.  */
    *save_ptr = tmem_char (token, '\0');
  else
    {
      /* Terminate the token and make *SAVE_PTR point past it.  */
      *s = '\0';
      *save_ptr = s + 1;
    }
  return token;
}

#else

int stricmp(const char *string1, const char *string2)
{
	return strcasecmp(string1, string2);
}

int strnicmp(const char *string1, const char *string2,size_t count)
{
	return strncasecmp(string1, string2, count);
}

char *strupr(char *str)
{
	int i=0;

	while(str[i])
	{
		str[i]	=	(char)toupper(str[i]);
		i++;
	}

	return str;
}

char *strlwr(char *str)
{
	int i=0;

	while(str[i])
	{
		str[i]	=	(char)tolower(str[i]);
		i++;
	}

	return str;
}

#endif



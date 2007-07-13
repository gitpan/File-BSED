/*
* $Id: bsedlib.c,v 1.4 2007/07/13 15:42:38 ask Exp $
* $Source: /opt/CVS/File-BSED/bsedlib.c,v $
* $Author: ask $
* $HeadURL$
* $Revision: 1.4 $
* $Date: 2007/07/13 15:42:38 $
*/

/* bsedlib - Library version of binary stream editor.
   written 2007 by Ask Solem <ask@0x61736b.net>

   Lots of changes added to make it work with perl/XS.
   No longer just a program, but reusable as well.
*/

/*
 * bsed - binary stream editor.  Written Feb 1987 by David W. Dykstra
 * Copyright (C) 1987-2002 by Dave Dykstra and Lucent Technologies
 *
 * So what's with the copyright notice if this is GPL?
 * Anyway, monsterously hacked by feedface 2006
 * Now you must enter hex strings (without preceding 0x)
 * but you can do wildcard search/replace using ?? as a placeholder
 * ie ?? in the search string represents any byte
 * in the replace string ?? means do not replace that byte
 *
 */

/* If given a search=replace string, both input and output file names 	*/
/*   must be present; else only the input file name must be present.	*/
/* Both input and output file names can be "-" to signify standard 	*/
/*   input or standard output respectively.				*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "bsedlib.h"

#define REPLACESIZE	1024	/* maximum replacement string size */

#define CTXTSIZE	5	/* size of left and right context  */

int isb_errno = 0;

#define mygetc()	((topstack == 0) ? getc(ifile) : stack[--topstack])
#define myungetc(c)	(stack[topstack++] = c)
/* myputc saves the character in the ltxt arrray for context printing */
#ifdef lint
#define myputc(c) putc(c,ofile)
#else
#define myputc(c)	do { \
			    register unsigned char *p; \
			    for(p = &ltxt[0]; p < &ltxt[CTXTSIZE-1]; p++)\
				*p = *(p+1); \
			    *p = c; \
			    if (ltlen < CTXTSIZE) \
				ltlen++; \
			    if (ofile != NULL) \
				putc(c,ofile); \
			} while(0);
#endif

//void fmyputc(int *stack, int topstack, register int c) {


const char* isb_errtostr(int isb_errno_val) {
    const char *retval;

    switch (isb_errno_val) {
        case ISB_ESEARCH_TOO_LONG:
            retval = "Search string too long.";
            break;
        case ISB_ENULL_SEARCH:
            retval = "Missing search string.";
            break;
        case ISB_EREPLACE_TOO_LONG:
            retval = "Replace string too long.";
            break;
        case ISB_ENULL_REPLACE:
            retval = "Missing replace string.";
            break;
        case ISB_EMISSING_INPUT:
            retval = "Missing input filename.";
            break;
        case ISB_EMISSING_OUTPUT:
            retval = "Missing outupt filename.";
            break;

        default:
            retval = NULL;
            break;
    }

    return retval;
}

int bsr(UCHAR *searchin, UCHAR *replacein, UCHAR *ifilenm, UCHAR *ofilenm, int
minmatch, int maxmatch, int opt_noreplace) {

    register int c;
    register long cnt;
    register unsigned char *s;
    unsigned char *search;
    unsigned char *replace;
    int slen, rlen  = 0;
    int match       = 0;
    unsigned char ltxt[CTXTSIZE+1],rtxt[CTXTSIZE+1];
    int ltlen = 0;
    int rtlen = 0;
    int stack[REPLACESIZE+CTXTSIZE+1]; /* saved character stack */
    int topstack = 0;

    search  = NULL;
    replace = NULL;

    isb_errno = 0;

    unsigned char sbuf[REPLACESIZE+1]; /* search string buffer */
    unsigned char rbuf[REPLACESIZE+1]; /* replace string buffer */

    FILE *ifile,*ofile;		/* input and output files */
    ifile = NULL; 
    ofile = NULL;    /* the output FILE struct */
    
    /* check the search string */
    if ((slen = convert(searchin, sbuf)) == -1) {
	    isb_errno = ISB_ESEARCH_TOO_LONG;
        return ISB_ERROR;
    }
    search = &sbuf[0];
    if (slen == 0) {
	    isb_errno = ISB_ENULL_SEARCH;
	    return ISB_ERROR;
    }

    if (opt_noreplace)
        replace = NULL;
    
    /* if we have the replace option */
    if (replacein != NULL) {

	    if ((rlen = convert(replacein, rbuf)) == -1) {
	        isb_errno = ISB_EREPLACE_TOO_LONG;
            return ISB_ERROR;
	    }

	    replace = &rbuf[0];

	    if (rlen == 0) {
	        isb_errno = ISB_ENULL_REPLACE;
            return ISB_ERROR;
	    }
	    if ((slen != rlen)) {
	        fprintf(stderr,
		        "WARNING: search and replace strings not same length\n"
			);
        }
    }

    if (ifilenm == NULL) {
        isb_errno = ISB_EMISSING_INPUT;
        return ISB_ERROR;
    }
    if (opt_noreplace == 1 && ofilenm == NULL) {
        //isb_errno = ISB_EMISSING_OUTPUT;
        //return ISB_ERROR;
    }
    
    /* input if filename is '-' (dash), we get input from STDIN */
    if (strcmp((char *)ifilenm,"-") == 0) {
	    ifile = stdin;
    }
    else {
        /* any other input filename is opened into FILE ifile */
	    if ((ifile = fopen((char *)ifilenm,"r")) == NULL) {
	        fprintf(stderr,"ERROR: cannot open %s for input\n",ifilenm);
	        exit(3);
	    }
    }

    if (ofilenm != NULL) {
        
        /* if output filename is '-' (dash), we send output to STDOUT */
	    if (strcmp((char *)ofilenm,"-") == 0) {
	        ofile = stdout;
        }
	    else {
            /* any other output filename is opened as FILE ofile */
	        if ((ofile = fopen((char *)ofilenm,"w")) == NULL) {
		        fprintf(stderr,"ERROR: cannot open %s for output\n",ofilenm);
		        exit(3);
	        }
	    }
    }

    cnt = 0;
    s = search;
    do
    {
	cnt++;
	
	if (((c = mygetc()) == *s) && ((maxmatch < 0 ) || (match < maxmatch)))
	{
	    register long savcnt;
	    register unsigned char *end;
	    int savbuf[REPLACESIZE];
	    int savlen;


	    savcnt = cnt;
	    savlen = 0;
	    savbuf[savlen++] = c;
	    end = s + slen;

	    while(1) /*matches */
	    {
		if (++s == end) /* gone right through s */
		{
		    register int i;

		    match++;
		    if (match < minmatch)
		    {
			for (i = 0; i < savlen; i++)
			    myputc(savbuf[i]);
		    }
		    else
		    {
			if (replace == NULL)
			{
			    /* for the sake of the context prints; ofile is */
			    /*   NULL so nothing will actually be written   */
			    for (i = 0; i < savlen; i++)
				myputc(savbuf[i]);
			}
			else
			{
			    for (i = 0; i < rlen; i++)
				{
		//		fprintf(stderr,"replacein\t%c\n",*(replacein + i*2+1)); fprintf(stderr,"replace\t%s\n",dump((replace+i),1));
				if (*(replacein + i*2) == '?')
				{myputc(savbuf[i]);}
				else
				{myputc(*(replace+i));}
				}
			}
		    }
		    break;
		}
		else /* not finished checking, and not failed */
		{
		    cnt++;
		    c = mygetc();
		    savbuf[savlen++] = c; /* save for possible later use */
		    if ((c != *s) && (*s != '?') && (*(searchin + savlen*2) != '?')) /* no match, no wildchar */
		    {
					register int i;

					/* non-match, restore saved chars */
					for (i = savlen-1; i >= 1; i--)
					    myungetc(savbuf[i]);
					/* and write out the first one */
					c = savbuf[0];
					myputc(c);
					cnt = savcnt;
					break;
			}
		}
		}
	    s = search;
	}
	else if (c != EOF)
	    myputc(c);

    } while (c != EOF);

    /* flush the buffer */
    fflush(ofile);

    /* close files */
    fclose(ifile);
    if (ofilenm != NULL) {
        fclose(ofile);
    }

    match -= (minmatch - 1);

    return match;
}

/***********************************************************************
*
* convert: convert a hex/ascii string into a string of characters
*
* arguments: s - pointer to string to convert
*	     o - pointer to output string
*
* returns: number of bytes convert
*
* algorithm: If string begins with a digit (0-9), assume a hex number is 
*	beginning.  Continue converting hex digits (0-f) into binary
*	until a non-hex digit begins, unless the string began with
*	"0x" or "0x": in that case just throw away the 0x.  Every
*	2 hex digits makes a byte; if there is an odd number then
*	the last digit is taken as the low nibble of a byte.  Anytime
*	a backslash (\) is encountered, consider it to be a break
*	in the conversion process and treat it as if starting a new
*	string.  If another backslash is immediately following it,
*	insert a backslash.  Once converting an ascii string, continue
*	converting it until a backslash or end of string.
*	NOTE: be sure to escape the backslash from the shell.

hacked out the text business. this is a Binary sed after all.
if the char '?' is present, this is a placeholder (wildcard) for a nybble,
and its position needs to be separately noted in an array, 
or we need to read through the converted and unconverted strings at the same time:
this is simpler, and maybe easier. 

Note also 0x is not required and using it may bugger the app up!

Note nothing other than hex or '?' is allowed, and will cause death (currently)
*
***********************************************************************/

#define isnum(c)	(((c) >= '0') && ((c) <= '9'))
#define ishex(c)	(isnum(c) || (((c) >= 'a') && ((c) <= 'f')) || \
				     (((c) >= 'A') && ((c) <= 'F')))
#define hexval(c)	(isnum(c) ? ((c) & 0xf) : (((c) & 0xf) + 9))
#define iswild(c)     ((c) == '?')

int convert(register unsigned char *s, register unsigned char *o)
{
    register unsigned char *p;
    register unsigned char c,t;
    register unsigned char *end;

    p = o;
    end = p + REPLACESIZE;
    c = *s++;
    while (c != '\0')
    {
	if (ishex(c))
	{
	    t = hexval(c);
	    if (c == '0')
	    {
		if (((c = *s++) == 'x') || (c == 'X'))
		{
		    c = *s++;
		    if (!ishex(c))
			continue;
		    t = hexval(c);
		    c = *s++;
		}
	    }
	    else
		c = *s++;
	    while (ishex(c))
	    {
		t = (t << 4) + hexval(c);
		c = *s++;
		if (ishex(c))
		{
		    *p++ = t;
		    if (p >= end)
			return(-1);
		    t = hexval(c);
		    c = *s++;
		}
	    }
	    *p++ = t;
	}
	else if (iswild(c))
	{
		/* put the '?' straight in, it is irrelevant in this context */
		*p++ = c;
		c = *s++;
		if (!(iswild(c))) /* complain, we only accept wild bytes, not nybbles */
		{ fprintf(stderr, " Only wild bytes are allowed, not nybbles\n");
		exit(2);
		}
		c = *s++; /*move to the beginning of the next byte */
		continue;
	}
	else
	{
		fprintf(stderr,
		"Only HEX values or the wilcard ('?') are allowed, you included:%c\n",c);
		exit(2);
	}
	if (p >= end)
	    return(-1);
    }
    *p = '\0';
    
    return(p - o);
}

/***********************************************************************
*
* dump: hex/ascii dump a string
*
* arguments: str - pointer to string to dump
*	     len - number of bytes to dump (string may contain nulls)
*
* returns: pointer to string to dump
*
***********************************************************************/

#define NUMDUMP REPLACESIZE
#define ascval(c)	(((c) < 10) ? ((c) + '0') : ((c) - 10 + 'a'))
#define isprint(c)	(((c) >= ' ') && ((c) <= '~'))

unsigned char* dump(unsigned char *str,register int len)
{
    static unsigned char buf[NUMDUMP*4+1];
    register unsigned char c,*p,*s,*end;

    if (len > NUMDUMP)
	len = NUMDUMP;
    end = str + len;
    p = &buf[0];

    for (s = str; s < end; s++)
    {
	c = (*s) >> 4;
	*p++ = ascval(c);
	c = (*s) & 0xf;
	*p++ = ascval(c);
	*p++ = ' ';
    }

    for (s = str; s < end; s++)
    {
	c = *s;
	if (isprint(c))
	    *p++ = c;
	else
	    *p++ = '.';
    }
    *p++ = '\0';
    return(&buf[0]);
}


/*
# Local Variables:
#   mode: cperl
#   indent-level: 4
#   fill-column: 78
# End:
# vim: expandtab tabstop=4 shiftwidth=4 shiftround
*/

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

typedef unsigned char UCHAR;
int bsr(UCHAR *, UCHAR *, UCHAR *, UCHAR *, int, int, int);
int convert(register unsigned char *, register unsigned char *);
unsigned char* dump(unsigned char *str, register int len);
const char* isb_errtostr(int);

#define ISB_ERROR                -1
#define ISB_ESEARCH_TOO_LONG     0x1
#define ISB_ENULL_SEARCH         0x2
#define ISB_EREPLACE_TOO_LONG    0x3
#define ISB_ENULL_REPLACE        0x4
#define ISB_EMISSING_INPUT       0x5
#define ISB_EMISSING_OUTPUT      0x6

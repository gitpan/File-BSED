#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include <stdio.h>
#include "bsedlib.h"

extern int isb_errno;

MODULE = File::BSED		PACKAGE = File::BSED
PROTOTYPES: DISABLE

int
_binary_file_matches (search, infile)
    unsigned char *search;
    unsigned char *infile;

    CODE:
        unsigned char *replace;
        unsigned char *outfile;
        int min  =  1;
        int max  = -1;
        int no_r =  1;
        replace  = NULL;
        outfile  = NULL;


    RETVAL = bsr(search, replace, infile, outfile, min, max, no_r);


    OUTPUT:
    RETVAL

int
_bsed (search, replace, infile, outfile, min, max)
    unsigned char *search;
    unsigned char *replace;
    unsigned char *infile;
    unsigned char *outfile;
    int min;
    int max;

    CODE:
        int no_r =  0; 
    
    RETVAL = bsr(search, replace, infile, outfile, min, max, no_r);

    OUTPUT:
    RETVAL

int
_isb_errno ()
    CODE:
        RETVAL = isb_errno;
    OUTPUT:
        RETVAL

void
_set_isb_errno (new_errno)
    int new_errno;

    CODE:
        isb_errno = new_errno;
        

SV *
_errtostr(isb_errno_val)
    int isb_errno_val;

    CODE:
    const char *isb_errstr;

    isb_errstr = isb_errtostr(isb_errno_val);
    if (isb_errstr == NULL) {
        RETVAL = &PL_sv_undef;
    }
    else {
        RETVAL = newSVpvn((char *)isb_errstr, (STRLEN)strlen((char *)isb_errstr));
    }

    OUTPUT:
    RETVAL
    
            

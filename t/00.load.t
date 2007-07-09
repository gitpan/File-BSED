#!/usr/local/bin/perl
use strict;
use warnings;
use Test::More;
use FindBin qw($Bin);
use File::Spec;
use File::Temp qw(:POSIX);
use English    qw(-no_match_vars);
use vars qw($THIS_TEST_HAS_TESTS $THIS_BLOCK_HAS_TESTS);

$THIS_TEST_HAS_TESTS = 7;

plan( tests => $THIS_TEST_HAS_TESTS );

    use_ok('File::BSED');
File::BSED->import(qw(bsed binary_file_matches));

my $test_binary = File::Spec->catfile($Bin, 'testbinary');

is( binary_file_matches('0xff', $test_binary), 743,
    'binary_file_matches'
);
ok(!File::BSED->errno,           'File::BSED->errno');
is( File::BSED->errtostr, undef, 'File::BSED->errtostr');

my $tempfile = tmpnam();

$THIS_BLOCK_HAS_TESTS = 3;
SKIP: {
    eval qq{
        open my \$fh, '>', "$tempfile" or die \$OS_ERROR;
        unlink "$tempfile"
    };
    if ($EVAL_ERROR) {
        skip(
            "Couldn't write to $tempfile: $EVAL_ERROR",
            $THIS_BLOCK_HAS_TESTS
        );
    }

    my $matches = bsed({
        infile  => $test_binary,
        outfile => $tempfile,
        
        search  => '0xff',
        replace => '0x00',
    });

    is( $matches, 743, 'bsed({ ... })');
    ok(!File::BSED->errno,              'File::BSED->errno'     );
    is( File::BSED->errtostr, undef,    'File::BSED->errtostr'  );

    unlink $tempfile;

}
    
    








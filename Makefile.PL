use strict;
use lib '.';
use inc::Module::Install;
use Cwd;


# module metadata
name 'File-BSED';
license 'gpl';
all_from 'lib/File/BSED.pm';

# Compiling.
cc_inc_paths(getcwd());
cc_files('bsedlib.c', 'BSED.c');


# for META.yml, CPAN etc.
no_index(directory => 'inc');
no_index(directory => 't');
no_index(directory => 'devel');


# Dependencies
requires('Getopt::LL'             => 0);
requires('File::Temp'             => 0);

build_requires('Test::More'             => 0);
build_requires('ExtUtils::ParseXS'      => 0);

install_script 'bin/plbsed';

# Write Makefile
can_cc() or die "This module requires a C compiler";
sign();
WriteAll();

#!/usr/bin/perl
#
#  The file genVersion.pl is part of the "MatrixFileReader XOP".
#  It is licensed under the LGPLv3 with additional permissions,
#  see License.txt in the source folder for details.
#
use strict;
use warnings FATAL => qw( all );

my $git = "c:\\Program Files (x86)\\Git\\bin\\git.exe";
my $git_version=`"$git" rev-parse --short HEAD`;
#my $git_version=`"$git" describe --match "MFR_public_release*" --dirty --always`;
$git_version=~ s/\s+//g;

open(WRITER, ">gitversion.hpp");
print WRITER "#pragma once\n\n";
print WRITER "const char GIT_VERSION[] = \"$git_version\";\n";
close WRITER;

exit 0;

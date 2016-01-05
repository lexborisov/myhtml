#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my $utils = MyHTML::Base->new();

my $enum = q~
xlink:actuate	xlink	actuate	XLink namespace
xlink:arcrole	xlink	arcrole	XLink namespace
xlink:href	xlink	href	XLink namespace
xlink:role	xlink	role	XLink namespace
xlink:show	xlink	show	XLink namespace
xlink:title	xlink	title	XLink namespace
xlink:type	xlink	type	XLink namespace
xml:lang	xml	lang	XML namespace
xml:space	xml	space	XML namespace
xmlns	(none)	xmlns	XMLNS namespace
xmlns:xlink	xmlns	xlink	XMLNS namespace

~;

my @list; my $count = 0;
foreach my $line (split /\n/, $enum) {
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        next if $line eq "";
        
        my ($key, $prefix, $value, $namespace) = split /\t/, $line;
        
        print "{\"$key\", ", length($key), ", \"$value\", ", length($value), "},\n";
}


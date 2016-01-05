#!/usr/bin/perl -w

use utf8;
use strict;
use Encode;

use JSON::XS;
use MyHTML::Base;

my $filename = "../docs/named_character_references.json";

open my $fh, "<", $filename or die "Oh God, can't open $filename: $!\n";
binmode $fh, ":utf8";

my $json_obj = JSON::XS->new->utf8->decode(join "", <$fh>);
my ($min_to, $max_to, $min_from, $max_from) = (undef, 0, undef, 0);
my $count = 0;

foreach my $key (sort {$a cmp $b} keys %$json_obj) {
        use bytes;
        
        my $char = $json_obj->{$key}->{characters};
        my @chars = map {sprintf '\x%X', ord } split //, $char;
        
        my $len_key = length($key);
        my $len_chars = scalar(@chars);
        
        $min_to   = $len_chars if !defined $min_to || $min_from > $len_chars;
        $max_to   = $len_chars if $max_to < $len_chars;
        $min_from = $len_key if !defined $min_from || $min_from > $len_key;
        $max_from = $len_key if $max_from < $len_key;
        
        print $key, ', "', join("", @chars), '", ', $len_chars, "\n";
        
        $count++;
}

close $fh;

print "Min length From: $min_from\n";
print "Max length From: $max_from\n";
print "Min length To: $min_to\n";
print "Max length To: $max_to\n";
print "Total entities: $count\n";

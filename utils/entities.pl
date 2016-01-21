#!/usr/bin/perl -w

use bytes;
use strict;
use Encode;

use JSON::XS;
use MyHTML::Base;

my $filename = "../docs/named_character_references.json";

open my $fh, "<", $filename or die "Oh God, can't open $filename: $!\n";
binmode $fh, ":utf8";
my $json_obj = JSON::XS->new->utf8->decode(join "", <$fh>);
close $fh;

my $tree = work_now($json_obj, 0);
# try find
my $data = find_value_by_key($tree, "AMP;");

print $data->[3], "\n";

sub work_now {
	my ($json_obj, $print) = @_;
	
	my ($min_to, $max_to, $min_from, $max_from) = (undef, 0, undef, 0);
	
	my $count = 0;
	my $prep = {};
	
	#struct charef_entry {
	#    char ch;
	#    size_t next;
	#    size_t cur_pos;
	#    const char *data;
	#  	 size_t data_len;
	#}
	#typedef charef_entry_t;
	
	foreach my $key (sort {$a cmp $b} keys %$json_obj) {
		my $real_key = $key;
		$real_key =~ s/^\&//;
		
		my $char = $json_obj->{$key}->{characters};
		my @chars = map {sprintf '\x%X', ord } split //, $char;
		
		my $len_key = length($real_key);
		my $len_chars = scalar(@chars);
		
		$min_to   = $len_chars if !defined $min_to || $min_from > $len_chars;
		$max_to   = $len_chars if $max_to < $len_chars;
		$min_from = $len_key   if !defined $min_from || $min_from > $len_key;
		$max_from = $len_key   if $max_from < $len_key;
		
		my $ref_pref = $prep;
		foreach my $char (split //, $real_key)
		{
			$ref_pref->{$char} = {} unless exists $ref_pref->{$char};
			$ref_pref = $ref_pref->{$char};
		}
		
		$ref_pref->{value} = $json_obj->{$key}->{characters};
		$ref_pref->{chars} = join "", @chars;
		$ref_pref->{key} = $real_key;
		
		$count++;
	}
	
	my $tree = {};
	my $links = convert_first($tree, $prep, 256);
	
	if ($print) {
		#foreach my $name (sort {$a cmp $b} keys %$links) {
		#	my $conv = convert_variable_name($name);
		#	print "static const char* $conv = \"$links->{$name}\";\n";
		#}
		#
		#print "\n";
		
		my $i = 1;
		foreach my $pos (sort {$a <=> $b} keys %$tree) {
			print "{'$tree->{$pos}[0]', $tree->{$pos}[1], $tree->{$pos}[2], $tree->{$pos}[3], $tree->{$pos}[4]},";
			
			unless ($i % 3) {
				print "\n";
			}
			
			$i++;
		}
		
		print "\n\n";
	
		print "Min length From: $min_from\n";
		print "Max length From: $max_from\n";
		print "Min length To: $min_to\n";
		print "Max length To: $max_to\n";
		print "Total entities: $count\n";
	}
	
	$tree;
}

sub convert {
	my ($tree, $zav, $ref, $offset) = @_;
	
	my $i = 0;
	my @entries;
	foreach my $char (sort {ord($a) <=> ord($b)} keys %$ref)
	{
		next unless length($char) == 1;
		
		if(exists $ref->{$char}->{chars}) {
			push @entries, [$char, 0, $offset, '"'. $ref->{$char}->{chars} .'"', length($ref->{$char}->{value})];
		}
		else {
			push @entries, [$char, 0, $offset, 'NULL', 0];
		}
		
		$offset++;
		$i++;
	}
	
	# create last null offset
	$tree->{$offset} = ['\0', 0, $offset, 'NULL', 0];
	$offset++;
	
	foreach my $entry (@entries)
	{
		$tree->{$entry->[2]} = $entry;
		
		if (exists $ref->{$entry->[0]}->{value}) {
			$zav->{ $ref->{$entry->[0]}->{key} } = $ref->{$entry->[0]}->{chars};
		}
		
		if (keys %{$ref->{$entry->[0]}} != 3 || !exists $ref->{$entry->[0]}->{value})
		{
			$entry->[1] = $offset;
			$offset = convert($tree, $zav, $ref->{$entry->[0]}, $offset);
		}
	}
	
	$offset;
}

sub convert_first {
	my ($tree, $prep, $offset) = @_;
	
	my @entries;
	foreach my $first_char_id (0..255)
	{
		if (exists $prep->{chr($first_char_id)}) {
			push @entries, [chr($first_char_id), 0, $first_char_id, 'NULL', 0];
		}
		else {
			$tree->{$first_char_id} = ['\0', 0, $first_char_id, 'NULL', 0];
		}
	}
	
	$tree->{$offset} = ['\0', 0, $offset, 'NULL', 0];
	$offset++;
	
	my $zav = {};
	
	foreach my $entry (@entries)
	{
		$entry->[1] = $offset;
		$offset = convert($tree, $zav, $prep->{$entry->[0]}, $offset);
		
		$tree->{$entry->[2]} = $entry;
	}
	
	$zav;
}

sub convert_variable_name {
	my ($name) = @_;
	$name =~ s/[^A-Za-z0-9]/_t/g;
	"myhtml_charef_values_$name";
}

sub find_value_by_key {
	my ($tree, $key) = @_;
	
	my $data = $tree;
	my @chars = split //, $key;
	my $idx = 0;
	my $pos = ord($chars[$idx]);
	
	while ($pos) {
		$data = $tree->{$pos};
		
		if (ord($chars[$idx]) == ord($data->[0])) {
			$pos = $data->[1];
			$idx++;
		}
		elsif (ord($chars[$idx]) > ord($data->[0])) {
			$pos++;
		}
		else {
			return 0;
		}
	}
	
	$data;
}



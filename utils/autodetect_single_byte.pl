#!/usr/bin/perl -w

use utf8;
use strict;
use Encode;
use JSON::XS;
use MyHTML::Base;


binmode STDOUT, ":utf8";

my $base_path = "tmpl/encoding_for_detector";

my $trigram = read_trigrams("tmpl/encoding_for_detector/trigram.data");
for_txt_files($trigram, $base_path);

sub for_txt_files {
	my ($trigram, $base_path) = @_;
	
	my $myhtml_encoding_list = {};
	my @names;
	
	opendir my $dh, $base_path || die "can't opendir $base_path: $!";
	foreach my $name (readdir($dh)) {
		next if $name !~ /txt$/;
		
		my $res = read_file("$base_path/$name");
		
		push @names, check_res($res, $trigram, convert_name($name));
	}
	closedir $dh;
	
	print "total list:\n";
	print join("\n", @names), "\n";
}

sub check_res {
	my ($res, $trigram, $name) = @_;
	
	my $name_full = "myhtml_encoding_trigram_t myhtml_encoding_detect_trigrams_$name";
	
	print "static const $name_full", "[] = {\n";
	
	foreach my $tname (sort {$trigram->{$b} <=> $trigram->{$a}} keys %$trigram) {
		last if $trigram->{$tname} < 10;
		
		my @chars = split //, $tname;
		
		my @comp;
		foreach my $char (@chars) {
			my $val = $res->{char}->{$char}->[0] | 0x80;
			
			push @comp, sprintf("0x%X", $val);
		}
		
		print "\t{{", join(", ", @comp), "}, ", $trigram->{$tname}, "}, /* $tname */\n";
	}
	
	print "};\n\n";
	
	$name_full;
}

sub read_file {
	my ($filepath) = @_;
	
	my $index = {};
	open my $fh, "<", $filepath or die "can't open file $filepath: $!";
	binmode $fh, ":utf8";
	
	while (my $line = <$fh>) {
		next if $line =~ /^\#/;
		
		$line =~ s/^\s+//;
		$line =~ s/\s+\z//;
		next unless $line;
		
		my ($pos, $data, $char) = split /\s+/, $line, 4;
		next unless $data;
		
		$index->{pos}->{$pos} = [$pos, $data, $char];
		$index->{char}->{$char} = $index->{pos}->{$pos};
	}
	
	close $fh;
	
	$index;
}

sub read_trigrams {
	my ($filepath) = @_;
	
	my $index = {};
	open my $fh, "<", $filepath or die "can't open file $filepath: $!";
	binmode $fh, ":utf8";
	
	while (my $line = <$fh>) {
		next if $line =~ /^\#/;
		$line =~ s/^\s+//;
		$line =~ s/\s+$//;
		
		my ($key, $val) = split /\s+/, $line;
		
		$index->{$key} = $val;
	}
	
	close $fh;
	
	$index;
}

sub convert_name {
	my ($name) = @_;
	
	$name =~ s/\.txt$//;
	$name =~ s/[-\s]+/_/g;
	
	$name;
}

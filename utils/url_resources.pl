#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my $simple_encode = {
	# A C0 control is a code point in the range U+0000 to U+001F, inclusive
	# The simple encode set are C0 controls and all code points greater than U+007
};

my $default_encode = {
	" " => '',
	'"' => '',
	"#" => '',
	"<" => '',
	">" => '',
	"?" => '',
	"`" => '',
	"{" => '',
	"}" => ''
};

my $userinfo_encode = {
	"/" => '',
	":" => '',
	";" => '',
	"=" => '',
	"@" => '',
	"[" => '',
	"\\" => '',
	"]" => '',
	"^" => '',
	"|" => ''
};

my $utils = MyHTML::Base->new(dirs => {source => "../../Modest/source/myhtml/url", template => "tmpl"});

my $utils_data = $utils->read_tmpl("url_resources.h");
$utils->save_src("resources.h", $utils_data,
	{
		BODY =>
			get_text_data(creare_for_default(), "myhtml_url_resources_static_map_default") .
			get_text_data(creare_for_simple(), "myhtml_url_resources_static_map_simple") .
			get_text_data(creare_for_userinfo(), "myhtml_url_resources_static_map_userinfo")
	}
);

sub creare_for_default {
	my @data;
	
	for my $codepoint (0..255) {
		my $char = chr($codepoint);
		
		if (exists $default_encode->{$char} ||
			$codepoint <= 0x1F || $codepoint > 0x7E)
		{
			push @data, "0x00"
		}
		else {
			push @data, sprintf("0x%02x", $codepoint);
		}
	}
	
	return \@data;
}

sub creare_for_simple {
	my @data;
	
	for my $codepoint (0..255) {
		my $char = chr($codepoint);
		
		if ($codepoint <= 0x1F || $codepoint > 0x7E)
		{
			push @data, "0x00"
		}
		else {
			push @data, sprintf("0x%02x", $codepoint);
		}
	}
	
	return \@data;
}

sub creare_for_userinfo {
	my @data;
	
	for my $codepoint (0..255) {
		my $char = chr($codepoint);
		
		if (exists $default_encode->{$char} ||
			exists $userinfo_encode->{$char} ||
			$codepoint <= 0x1F || $codepoint > 0x7E)
		{
			push @data, "0x00"
		}
		else {
			push @data, sprintf("0x%02x", $codepoint);
		}
	}
	
	return \@data;
}

sub get_text_data {
	my ($data, $name) = @_;
	
	my @return;
	
	push @return, "static const unsigned char $name\[] =\n{";
	
	my $max = $#$data;
	foreach my $num (0..$max) {
		push @return, "\n\t" unless $num % 10;
		push @return, $data->[$num]. ($num != $max ? ", " : "");
	}
	
	push @return, "\n};\n\n";
	
	join "", @return;
}

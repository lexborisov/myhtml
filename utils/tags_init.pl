#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my $utils = MyHTML::Base->new(dirs => {source => "../myhtml", template => "tmpl"});

my $data = $utils->read_tmpl("mytags_init.c");
my $data_const = $utils->read_tmpl("mytags_const.h");
my $tags = $utils->read_tmpl("mytags.txt");

my @body;

my $count = 0; my @list;
foreach my $line (@$tags) {
        next unless $line;
        
        my $parse_tag_data = "MyHTML_TOKENIZER_STATE_DATA";
        
        my ($tagname, $args) = split /\:\s*/, $line, 2;
        my @options = split /\s*,\s*/, $args;
        
        $parse_tag_data = $options[0] if exists $options[0];
        $count++;
        
        $tagname = lc($tagname);
        
        push @body, "\t". qq~mytags_add(tags, "$tagname", ~. length($tagname) .qq~, $parse_tag_data);~;
        
        if($tagname eq "!--") {
                $tagname = "_comment";
        }
        
        $tagname =~ s/[\-!?]/_/g;
        $tagname = uc($tagname);
        
        push @list, ["MyTAGS_TAG_$tagname", sprintf("0x%03x", $count)];
}

$count++;

push @list, ["MyTAGS_TAG_FIRST_ENTRY", $list[0]->[0]];
push @list, ["MyTAGS_TAG_LAST_ENTRY", sprintf("0x%03x", $count)];

my $res = $utils->format_list_text(\@list, "= ");

my $args = {BODY => \@body};
my $args_const = {BODY => "\t". join ",\n\t", @$res};

$utils->save_src("mytags_const.h", $data_const, $args_const);
$utils->save_src("mytags_init.c", $data, $args);



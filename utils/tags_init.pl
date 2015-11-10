#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my $utils = MyHTML::Base->new(dirs => {source => "../myhtml", template => "tmpl"});

my $data = $utils->read_tmpl("mytags_init.c");
my $data_const = $utils->read_tmpl("mytags_const.h");
my $tags = $utils->read_tmpl("mytags.txt");

my @body;
my @body_const;

my $count = 0;
foreach my $line (@$tags) {
        next unless $line;
        
        my $parse_tag_data = "MyHTML_PARSE_STATE_DATA";
        
        my ($tagname, $args) = split /\:\s*/, $line, 2;
        my @options = split /\s*,\s*/, $args;
        
        $parse_tag_data = $options[0] if exists $options[0];
        $count++;
        
        $tagname = lc($tagname);
        
        push @body, "\t". qq~mytags_add(tags, "$tagname", ~. length($tagname) .qq~, $parse_tag_data);~;
        
        if($tagname eq "!--") {
                push @body_const, "// comment tag <!-->";
                $tagname = "_comment";
        }
        
        $tagname =~ s/[\-!?]/_/g;
        $tagname = uc($tagname);
        
        push @body_const, "\t". qq~MyTAGS_TAG_$tagname = $count~;
}

$count++;

push @body_const, "\t". "MyTAGS_TAG_FIRST_ENTRY = 1";
push @body_const, "\t". "MyTAGS_TAG_LAST_ENTRY = $count";

my $args = {BODY => \@body};
my $args_const = {BODY => join ",\n", @body_const};

$utils->save_src("mytags_const.h", $data_const, $args_const);
$utils->save_src("mytags_init.c", $data, $args);



#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my $utils = MyHTML::Base->new(dirs => {source => "../source/myhtml", template => "tmpl"});

my $data = $utils->read_tmpl("tag_init.c");
my $data_const = $utils->read_tmpl("tag_const.h");
my $tags = $utils->read_tmpl("tags.txt");

my (@body, @cats, @list);
my $count = 0;
my $last_tag_idx;

foreach my $line (@$tags) {
        next unless $line;
        
        if ($line =~ s/^=cats:\s*// && $last_tag_idx)
        {
                my ($namespace_pos, $cats) = split /\:\s*/, $line, 2;
                
                push @cats, "\t". qq~myhtml_tag_set_category(tags, $last_tag_idx, $namespace_pos, $cats);~;
        }
        else {
                my $parse_tag_data = "MyHTML_TOKENIZER_STATE_DATA";
                
                my ($tagname, $args) = split /\:\s*/, $line, 2;
                my @options = split /\s*,\s*/, $args;
                
                $count++;
                
                $parse_tag_data = $options[0] if defined $options[0] && $options[0] ne "";
                
                push @body, "\t". qq~myhtml_tag_add(tags, "$tagname", ~. length($tagname) .qq~, $parse_tag_data, myfalse);~;
                
                if($tagname eq "!--") {
                        $tagname = "_comment";
                }
                
                $tagname =~ s/[\-!?]/_/g;
                $tagname = uc($tagname);
                
                $last_tag_idx = "MyHTML_TAG_$tagname";
                
                push @list, [$last_tag_idx, sprintf("0x%03x", $count)];
        }
}

$count++;

push @list, ["MyHTML_TAG_FIRST_ENTRY", $list[0]->[0]];
push @list, ["MyHTML_TAG_LAST_ENTRY", sprintf("0x%03x", $count)];

unshift @list, ["MyHTML_TAG__UNDEF", "0x000"];

my $res = $utils->format_list_text(\@list, "= ");

my $args = {BODY => \@body, CATS => \@cats};
my $args_const = {BODY => "\t". join ",\n\t", @$res};

$utils->save_src("tag_const.h", $data_const, $args_const);
$utils->save_src("tag_init.c", $data, $args);



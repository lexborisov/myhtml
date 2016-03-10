#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

my %namespaces = (
        "MyHTML_NAMESPACE_UNDEF"      => 0x00,
        "MyHTML_NAMESPACE_HTML"       => 0x01,
        "MyHTML_NAMESPACE_MATHML"     => 0x02,
        "MyHTML_NAMESPACE_SVG"        => 0x03,
        "MyHTML_NAMESPACE_XLINK"      => 0x04,
        "MyHTML_NAMESPACE_XML"        => 0x05,
        "MyHTML_NAMESPACE_XMLNS"      => 0x06,
);

my $static_list_index_length = 479;

my $utils = MyHTML::Base->new(dirs => {source => "../source/myhtml", template => "tmpl"});

my $data = $utils->read_tmpl("tag_init.c");
my $data_const = $utils->read_tmpl("tag_const.h");
my $tags = $utils->read_tmpl("tags.txt", 1);

my $result = {};

my $count = 0;
my $last_tag_idx;
my $tag_exists = {};

foreach my $line (@$tags) {
        next unless $line;
        
        if ($line =~ s/^=cats:\s*// && $last_tag_idx)
        {
                my ($namespace_pos, $cats) = split /\:\s*/, $line, 2;
                
                $result->{$last_tag_idx}->{cats}->{$namespace_pos} = $cats;
        }
        else {
                my $parse_tag_data = "MyHTML_TOKENIZER_STATE_DATA";
                
                my ($tagname, $args) = split /\:\s*/, $line, 2;
                my @options = split /\s*,\s*/, $args;
                
                $parse_tag_data = $options[0] if defined $options[0] && $options[0] ne "";
                
                if($tagname eq "!--") {
                        $tagname = "_comment";
                }
                
                my $origin_tagname = $tagname;
                $tagname =~ s/[\-!?]/_/g;
                
                $last_tag_idx = "MyHTML_TAG_". uc($tagname);
                
                if ($tag_exists->{$last_tag_idx}) {
                        print "Conflict! Tag exists: $last_tag_idx\n";
				}
                else {
                        $tag_exists->{$last_tag_idx} = 1;
                }
                
                $result->{$last_tag_idx}->{name} = $origin_tagname;
                $result->{$last_tag_idx}->{data} = $parse_tag_data;
                $result->{$last_tag_idx}->{id}   = sprintf("0x%03x", $count);
                
                {use bytes; $result->{$last_tag_idx}->{name_len} = length($tagname);}
                
                $count++;
        }
}

sub create_cats_for_tag {
        my ($cats) = @_;
        
        my @res;
        foreach my $ns (sort {$namespaces{$a} <=> $namespaces{$b}} keys %namespaces) {
				push @res, $cats->{$ns} || "MyHTML_TAG_CATEGORIES_ORDINARY";
        }
        
        my $out = "\n\t\t{\n\t\t\t";
        foreach my $i (0..$#res) {
                $out .= $res[$i];
                $out .= ", " if $i != $#res;
                $out .= "\n\t\t\t" unless ($i + 1) % 2;
        }
        
        $out. "\n\t\t}\n";
}

sub get_index_id {
        use bytes;
        
        my @chars = split //, $_[0];
        my $f = ord(lc($chars[0]));
        my $l = ord(lc($chars[-1]));
        
        ((($f * $l * scalar(@chars))) % $_[1]) + 1;
}

sub create_const_structure {
        my ($result) = @_;
        
        my $index = {};
        
        my (@res, @list); my $max = {};
        foreach my $te_name (sort {$result->{$a}{id} cmp $result->{$b}{id}} keys %$result) {
                my $tag = $result->{$te_name};
                my $len = length($tag->{name});
                
                push @res, "\n\t{$te_name, \"$tag->{name}\", $len, $tag->{data}, ". create_cats_for_tag($tag->{cats}). "\t}";
                push @list, [$te_name, $tag->{id}];
                
                my $position = get_index_id($tag->{name}, $static_list_index_length);
                $tag->{pos} = $position;
                
                if (exists  $index->{$position}->{tag_id}) {
						my $cur = $index->{$position}->{tag_id};
                        
                        if ($result->{$cur}{name_len} > $tag->{name_len}) {
                                push @{$index->{$position}->{list}}, $cur;
								$index->{$position}->{tag_id} = $te_name;
						}
                        else {
                                push @{$index->{$position}->{list}}, $te_name;
                        }
				}
				else {
                        $index->{$position}->{tag_id} = $te_name;
                        $index->{$position}->{list} = [];
                }
        }
        
        ("static const myhtml_tag_context_t myhtml_tag_base_list[MyHTML_TAG_LAST_ENTRY] = \n{" . join(",", @res) . "\n};", \@list, $index);
}

sub create_sub_static_list_index {
        my ($result, $list, $struct, $offset) = @_;
        
        my @list_sorted = sort {$result->{$a}->{name_len} <=> $result->{$b}->{name_len}} @$list;
        
        foreach my $i (0..$#list_sorted) {
                my $cur = $offset;
                $offset++;
                push @$struct, "\t{&myhtml_tag_base_list[$list_sorted[$i]], ". ($i < $#list_sorted ? $offset : 0) .", $cur},\n";
        }
        
        $offset;
}

sub create_static_list_index {
        my ($result, $index) = @_;
        
        my @res;
        my $struct = [];
        my $offset = $static_list_index_length + 1;
        
        foreach my $i (0..$static_list_index_length)
        {
                if (exists $index->{$i}) {
                        my $id = 0;
                        if(@{$index->{$i}->{list}}) {
                                $offset = create_sub_static_list_index($result, $index->{$i}->{list}, $struct, $offset);
                                
                                $id = $offset - @{$index->{$i}->{list}};
                        }
                        
                        push @res, "\t{&myhtml_tag_base_list[". $index->{$i}->{tag_id} ."], $id, $i},\n";
				}
				else {
                        push @res, "\t{NULL, 0},\n";
                }
        }
        
        "static const myhtml_tag_static_list_t myhtml_tag_static_list_index[] = \n{\n". join("", @res, @$struct) ."\n};\n"
}

my ($struct_res, $list, $index) = create_const_structure($result);

my $static_list = create_static_list_index($result, $index);

my $defs = "#define MyHTML_BASE_STATIC_SIZE $static_list_index_length\n";
push @$list, ["MyHTML_TAG_FIRST_ENTRY", $list->[1]->[0]];
push @$list, ["MyHTML_TAG_LAST_ENTRY", sprintf("0x%03x", $count)];

my $res = $utils->format_list_text($list, "= ");
my $args_const = {DEFINED => $defs, BODY => "\t". join ",\n\t", @$res};

$utils->save_src("tag_const.h", $data_const, $args_const);
$utils->save_src("tag_init.c", $data, {CONST => $struct_res, STATIC_LIST => $static_list});





#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;
use JSON::XS;

my $static_list_index_length = 419;

my $utils = MyHTML::Base->new(dirs => {source => "../source/myhtml", template => "tmpl"});
my $raw_data = $utils->read_tmpl_raw("encoding/encodings.json");
my $encodings = decode_json(join("", @$raw_data));

#test_result();

my $result = create_result($encodings, $static_list_index_length);
my $static_list = create_static_list_index($result);

print $static_list, "\n";

sub create_result {
        my ($encodings, $static_list_index_length) = @_;
        my $result = {};
        
        foreach my $entry (@$encodings) {
                foreach my $encoding (@{$entry->{encodings}}) {
                        next if $encoding->{name} =~ /replacement/i;
                        
                        foreach my $label (sort {$a cmp $b} @{$encoding->{labels}}) {
                                my $id = get_index_id($label, $static_list_index_length);
                                
                                push @{$result->{$id}}, [$encoding->{name}, $label, length($label), length($encoding->{name})];
                        }
                }
        }
        
        $result;
}

sub test_result {
        my $op = [0, undef];
        
        foreach my $idx (1..1024) {
                my $result = create_result($encodings, $idx);
                my $res_max = test_result_max_value($result, 0);
                
                if(!defined($op->[1]) || $op->[1] > $res_max) {
                        $op->[0] = $idx;
                        $op->[1] = $res_max;
                }
        }
        
        print "Best:\n";
        print $op->[0], ": ", $op->[1], "\n";
}

sub test_result_max_value {
        my ($res, $is_print) = @_;
        
        my $max = 0;
        foreach my $id (sort {scalar(@{$res->{$a}}) <=> scalar(@{$res->{$b}})} keys %$res) {
                print "$id: ", scalar(@{$res->{$id}}), "\n" if $is_print;
                
                $max = scalar(@{$res->{$id}}) if $max < scalar(@{$res->{$id}});
        }
        
        $max;
}

sub get_index_id {
        use bytes;
        
        my @chars = split //, $_[0];
        my $f = ord(lc($chars[0]));
        my $l = ord(lc($chars[-1]));
        
        ((($f * $l * scalar(@chars))) % $_[1]) + 1;
}

sub create_sub_static_list_index {
        my ($result, $struct, $offset) = @_;
        
        my @list_sorted = sort {$a->[2] <=> $b->[2]} @$result[1..$#$result];
        
        foreach my $i (0..$#list_sorted) {
                my $cur = $offset;
                $offset++;
                push @$struct, "\t{".
                '"'. $list_sorted[$i]->[0] .'", '. $list_sorted[$i]->[3] .', '.
                '"'. $list_sorted[$i]->[1] .'", '. $list_sorted[$i]->[2] .', '.
                name_to_myhtml_encoding($list_sorted[$i]->[0]), ', '.
                ($i < $#list_sorted ? $offset : 0) .", $cur},\n";
        }
        
        $offset;
}

sub create_static_list_index {
        my ($result) = @_;
        
        my @res;
        my $struct = [];
        my $offset = $static_list_index_length + 1;
        
        foreach my $i (0..$static_list_index_length)
        {
                if(exists $result->{$i}) {
                        my $id = 0;
                        
                        if(scalar @{$result->{$i}} > 1) {
                                $offset = create_sub_static_list_index($result->{$i}, $struct, $offset);
                                
                                $id = $offset - (@{$result->{$i}} - 1);
                        }
                        
                        push @res, "\t{".
                        '"'. $result->{$i}->[0]->[0] .'", '. $result->{$i}->[0]->[3] .', '.
                        '"'.$result->{$i}->[0]->[1] .'", '. $result->{$i}->[0]->[2] .', '.
                        name_to_myhtml_encoding($result->{$i}->[0]->[0]), ', '.
                        "$id, $i},\n";
				}
				else {
                        push @res, "\t{NULL, 0, NULL, 0, 0, 0, 0},\n";
                }
        }
        
        "static const myhtml_encoding_detect_name_entry_t myhtml_encoding_detect_name_entry_static_list_index[] = \n{\n". join("", @res, @$struct) ."};\n"
}

sub name_to_myhtml_encoding {
        my ($name) = @_;
        
        $name =~ s/[-]+/_/g;
        $name = "MyHTML_ENCODING_". uc($name);
        
        $name;
}






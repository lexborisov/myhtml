#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;
use JSON::XS;

my $prefix = 'MyHTML_URL_SCHEME';
my $static_list_index_length = 31;
my $data = {
        'ftp'        => [21, 'MyHTML_URL_SCHEME_TYPE_SPECIAL|MyHTML_URL_SCHEME_TYPE_NETWORK'],
        'file'       => [0, 'MyHTML_URL_SCHEME_TYPE_SPECIAL|MyHTML_URL_SCHEME_TYPE_FETCH'],
        'gopher'     => [70, 'MyHTML_URL_SCHEME_TYPE_SPECIAL'],
        'http'       => [80, 'MyHTML_URL_SCHEME_TYPE_SPECIAL|MyHTML_URL_SCHEME_TYPE_HTTP_S'],
        'https'      => [443, 'MyHTML_URL_SCHEME_TYPE_SPECIAL|MyHTML_URL_SCHEME_TYPE_HTTP_S'],
        'ws'         => [80, 'MyHTML_URL_SCHEME_TYPE_SPECIAL'],
        'wss'        => [443, 'MyHTML_URL_SCHEME_TYPE_SPECIAL'],
        
        'about'      => [0, 'MyHTML_URL_SCHEME_TYPE_LOCAL|MyHTML_URL_SCHEME_TYPE_FETCH'],
        'blob'       => [0, 'MyHTML_URL_SCHEME_TYPE_LOCAL|MyHTML_URL_SCHEME_TYPE_FETCH'],
        'data'       => [0, 'MyHTML_URL_SCHEME_TYPE_LOCAL|MyHTML_URL_SCHEME_TYPE_FETCH'],
        'filesystem' => [0, 'MyHTML_URL_SCHEME_TYPE_LOCAL|MyHTML_URL_SCHEME_TYPE_FETCH']
};

$static_list_index_length = test_result($data);

my $utils = MyHTML::Base->new(dirs => {source => "../../Modest/source/myhtml/url", template => "tmpl"});

my $result = create_result($data, $static_list_index_length);
my ($enum, $enum_count) = create_enum($data, sub { return "MyHTML_URL_SCHEME_ID_". uc($_[0]) }, "myhtml_url_scheme_id");

# save const
my $utils_const_data = $utils->read_tmpl("url_scheme_const.h");
$utils->save_src("scheme_const.h", $utils_const_data, {BODY => $enum});

my $static_list = create_static_list_index($result,
sub {
        unless($_[0]) {
            return "$prefix\_ID_UNDEF, 0, MyHTML_URL_SCHEME_TYPE_UNDEF";
        }
        
        return $prefix ."_ID_". uc($_[0]->[0]) .", ". uc($_[0]->[2][0]) .", ". $_[0]->[2][1];
},
"myhtml_url_scheme_entry_t myhtml_url_scheme_entry_static_index[]");

# save resources
my $utils_resources_data = $utils->read_tmpl("url_scheme_resources.h");
$utils->save_src("scheme_resources.h", $utils_resources_data,
                 {BODY => $static_list, DEFINE => "MyHTML_URL_SCHEME_STATIC_INDEX_LENGTH $static_list_index_length"}
);

#print $enum, "\n";
#print $static_list, "\n";

sub create_result {
        my ($list, $static_list_index_length) = @_;
        my $result = {};
        
        foreach my $key (sort {$a cmp $b} keys %$list) {
                my $id = get_index_id($key, $static_list_index_length);
                
                push @{$result->{$id}}, [$key, length($key), $list->{$key}];
        }
        
        $result;
}

sub test_result {
        my ($list) = @_;
        my $op = [0, undef];
        
        foreach my $idx (1..1024) {
                my $result = create_result($list, $idx);
                my $res_max = test_result_max_value($result, 0);
                
                if(!defined($op->[1]) || $op->[1] > $res_max) {
                        $op->[0] = $idx;
                        $op->[1] = $res_max;
                }
        }
        
        print "Best:\n";
        print $op->[0], ": ", $op->[1], "\n";
        
        $op->[0];
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
        my ($result, $struct, $offset, $sub) = @_;
        
        my @list_sorted = sort {$a->[1] <=> $b->[1]} @$result[0..$#$result];
        
        foreach my $i (1..$#list_sorted) {
                my $cur = $offset;
                $offset++;
                push @$struct, "\t{".
                '"'. $list_sorted[$i]->[0] .'", '. $list_sorted[$i]->[1] .', '.
                $sub->($list_sorted[$i]), ', '.
                ($i < $#list_sorted ? $offset : 0) .", $cur},\n";
        }
        
        $offset;
}

sub create_static_list_index {
        my ($result, $sub, $static_name) = @_;
        
        my @res;
        my $struct = [];
        my $offset = $static_list_index_length + 1;
        
        foreach my $i (0..$static_list_index_length)
        {
                if(exists $result->{$i}) {
                        my $id = 0;
                        
                        if(scalar @{$result->{$i}} > 1) {
                                $offset = create_sub_static_list_index($result->{$i}, $struct, $offset, $sub);
                                
                                $id = $offset - (@{$result->{$i}} - 1);
                        }
                        
                        my @list_sorted = sort {$a->[1] <=> $b->[1]} @{$result->{$i}}[0..$#{$result->{$i}}];
                        
                        push @res, "\t{".
                        '"'. $list_sorted[0]->[0] .'", '. $list_sorted[0]->[1] .', '.
                        $sub->($list_sorted[0]), ', '.
                        "$id, $i},\n";
				}
				else {
                        push @res, "\t{NULL, 0, ". $sub->(undef) .", 0, 0},\n";
                }
        }
        
        "static const $static_name =\n{\n". join("", @res, @$struct) ."};\n"
}

sub create_enum {
        my ($list, $sub, $enum_name) = @_;
        
        my @res;
        my $count = 1;
        
        push @res, [$sub->("UNDEF"), sprintf("0x%04x", 0)];
        
        foreach my $key (sort {$a cmp $b} keys %$list) {
                push @res, [$sub->($key), sprintf("0x%04x", $count)];
                $count++;
        }
        
        push @res, [$sub->("LAST_ENTRY"), sprintf("0x%04x", $count)];
        $count++;
        
        my $norm_res = MyHTML::Base->format_list_text(\@res, "= ");
        ("enum $enum_name {\n\t". join(",\n\t", @$norm_res) ."\n}\ntypedef $enum_name\_t;\n", $count);
}

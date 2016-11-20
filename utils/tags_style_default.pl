#!/usr/bin/perl -w

use utf8;
use strict;
use MyHTML::Base;

#my $utils = MyHTML::Base->new(dirs => {source => "../source/myhtml", template => "tmpl"});
my $utils = MyHTML::Base->new(dirs => {source => "/new/C-git/Modest/source/modest/style", template => "tmpl"});

my $data = $utils->read_tmpl("default_resources.h");
my $tags = $utils->read_tmpl("tags.txt", 1);

my $result = {};

my $count = 0;
my $last_tag_idx;
my $tag_exists = {};

foreach my $line (@$tags) {
        next unless $line && $line !~ /^=cats:\s*/;
        
        my ($tagname, $args) = split /\:\s*/, $line, 2;
        my @options = split /\s*,\s*/, $args;
        
        if ($tag_exists->{$tagname}) {
                print "Conflict! Tag exists: $tagname\n";
        }
        else {
                $tag_exists->{$tagname} = 1;
        }
        
        if($tagname eq "!--") {
                $tagname = "_comment";
        }
        elsif($tagname =~ /-end-of-file/) {
                $tagname = "_undef";
        }
        
        my $origin_tagname = $tagname;
        $tagname =~ s/[\-!?]/_/g;
        
        $last_tag_idx = "MyHTML_TAG_". uc($tagname);
        
        $result->{$last_tag_idx}->{name} = $tagname;
        $result->{$last_tag_idx}->{id}   = sprintf("0x%03x", $count);
        
        {use bytes; $result->{$last_tag_idx}->{name_len} = length($tagname);}
        
        $count++;
}

$result->{"MyHTML_TAG__UNDEF"}->{id} = 0;
$result->{"MyHTML_TAG__UNDEF"}->{name} = "_undef";

sub create_map {
        my ($result) = @_;
        
        my @res;
        my @header;
        
        foreach my $te_name (sort {$result->{$a}{id} cmp $result->{$b}{id}} keys %$result) {
                push @res, "modest_style_default_declaration_by_html_node_". $result->{$te_name}->{name};
        }
        
        my $const =
        "static const modest_style_default_by_html_node_f modest_style_default_function_of_declarations\[MyHTML_TAG_LAST_ENTRY] = \n{\n\t".
        join(",\n\t", @res).
        "\n};\n\n";
        
        my @head;
        my @body;
        my $head_args = "(modest_t * modest, myhtml_tree_node_t* node, mycss_property_type_t type)";
        
        my $exists = {};
        foreach my $name (@res) {
                next if exists $exists->{$name};
                $exists->{$name} = 1;
                
                push @head, "mycss_declaration_entry_t * $name$head_args";
                push @body, $head[-1]. "\n{\n";
                $body[-1] .= "\treturn NULL;\n";
                $body[-1] .= "}\n";
                
                $head[-1] .= ";";
        }
        

        return ($const, join("\n", @head), join("\n", @body));
}

my ($struct_map, $head, $body) = create_map($result);
print $head, "\n\n\n\n";
print $body, "\n";

$utils->save_src("default_resources.h", $data, {BODY => $struct_map});

print "Done;";



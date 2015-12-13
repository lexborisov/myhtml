package MyHTML::Base;

use utf8;
use strict;

sub new {
        my ($class, %args) = @_;
        bless \%args, $class;
}

sub save_src {
        my ($self, $filename, $data, $args) = @_;
        
        my $msub = sub {
                my $tmp = shift;
                
                if(exists $args->{$tmp}) {
                        return join "\n", @{$args->{$tmp}} if ref $args->{$tmp} eq "ARRAY";
                        return $args->{$tmp};
                }
                
                "";
        };
        
        $filename = $self->{dirs}->{source}."/$filename";
        
        open my $fh, ">", $filename or die "Can't open $filename: $!";
        binmode $fh, ":utf8";
        
        foreach my $line (@$data) {
                $line =~ s/\%([^\%]+)\%/$msub->($1)/ge;
                
                print $fh $line, "\n";
        }
        
        close $fh;
}

sub read_tmpl {
        my ($self, $filename) = @_;
        
        $filename = $self->{dirs}->{template}."/$filename";
        
        open my $fh, "<",  $filename or die "Can't open $filename: $!";
        binmode $fh, ":utf8";

        my @data;

        while (my $line = <$fh>)
        {
                $line =~ s/\s+$//;
                $line =~ s/^\s+//;
                
                push @data, $line;
        }

        close $fh;

        \@data;
}

sub format_list_text {
        my ($self, $list, $join_val) = @_;
        
        my ($max, $len) = (0, 0);
        foreach my $struct (@$list) {
                $len = length($struct->[0]);
                $max = $len if $len > $max;
        }
        
        my @res;
        foreach my $struct (@$list) {
                $len = $max - length($struct->[0]);
                push @res, sprintf("%s%$len"."s %s%s", $struct->[0], ($len ? " " : ""), $join_val, $struct->[1]);
        }
        
        \@res;
}

1;

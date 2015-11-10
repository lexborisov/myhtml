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
                chop $line;
                
                push @data, $line;
        }

        close $fh;

        \@data;
}


1;


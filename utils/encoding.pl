#!/usr/bin/perl -w

use bytes;
use strict;
use Encode;
use JSON::XS;
use MyHTML::Base;

my $base_path = "tmpl/encoding";

#for_txt_files($base_path);
#for_json("$base_path/indexes.json");
for_single_byte("$base_path/encodings.json");

sub for_json {
	my ($filename) = @_;
	
	my $myhtml_encoding_list = {};
	
	open my $fh, "<", $filename or die "Oh God, can't open $filename: $!\n";
	binmode $fh, ":utf8";
	my $json_obj = JSON::XS->new->utf8->decode(join "", <$fh>);
	close $fh;
	
	foreach my $name (sort {$a cmp $b} keys %$json_obj) {
		print_res($json_obj->{$name}, $name, $myhtml_encoding_list);
	}
	
	print_links($myhtml_encoding_list);
}

sub for_single_byte {
	my ($filename) = @_;
	
	my $myhtml_encoding_list = {};
	
	open my $fh, "<", $filename or die "Oh God, can't open $filename: $!\n";
	binmode $fh, ":utf8";
	my $json_obj = JSON::XS->new->utf8->decode(join "", <$fh>);
	close $fh;
	
	foreach my $entry (@$json_obj)
	{
		if($entry->{heading} =~ /single-byte/)
		{
			foreach my $encode (@{$entry->{encodings}}) {
				print_single_byte_func($encode->{name});
			}
			
			last;
		}
	}
	
	foreach my $entry (@$json_obj)
	{
		if($entry->{heading} =~ /single-byte/)
		{
			foreach my $encode (@{$entry->{encodings}}) {
				convert_name($encode->{name});
				print "enum myhtml_encoding_status myhtml_encoding_decode_$encode->{name}(unsigned const char data, myhtml_encoding_result_t *res);\n"
			}
			last;
		}
	}
}

sub print_single_byte_func {
	my ($name) = @_;
	
	convert_name($name);
	
print "enum myhtml_encoding_status myhtml_encoding_decode_$name(unsigned const char data, myhtml_encoding_result_t *res)
{
	if(data >= 0x00 && data <= 0x7F)
		(res->result = data);
	else
		(res->result = myhtml_encoding_map_$name"."[(data - 0x80)]);
	
	return MyHTML_ENCODING_STATUS_OK;
}\n\n";
}

sub for_txt_files {
	my ($base_path) = @_;
	
	my $myhtml_encoding_list = {};
	
	opendir my $dh, $base_path || die "can't opendir $base_path: $!";
	foreach my $name (readdir($dh)) {
		next if $name !~ /txt$/;
		
		my $res = read_file("$base_path/$name");
		
		print_res($res, $name, $myhtml_encoding_list);
		#print "$name\n";
	}
	closedir $dh;
}

sub print_links {
	my ($myhtml_encoding_list) = @_;
	
	print "enum myhtml_encoding_list {\n";
    print "\tMyHTML_ENCODING_UNDEF = 0x00,\n";
    print "\tMyHTML_ENCODING_UTF_8 = 0x01,\n";
	
	my $i = 2; my @links = (0, 0);
	foreach my $id (sort {$a cmp $b} keys %$myhtml_encoding_list) {
		print "\t$id = ", sprintf("0x%02x", $i), ",\n";
		$i++;
		
		push @links, $myhtml_encoding_list->{$id};
	}
	
	print "\tMyHTML_ENCODING_LAST_ENTRY = ", sprintf("0x%02x\n", $i);
	print "};\n\n";
	
	print "static const unsigned long *myhtml_encoding_var_index[] = {\n";
	foreach my $i (0..$#links) {
		print $links[$i], ", ";
		print "\n" unless ($i + 1) % 2;
	}
	print "\n};\n";
}

sub read_file {
	my ($filepath) = @_;
	
	my $res = [];
	open my $fh, "<", $filepath or die "can't open file $filepath: $!";
	
	while (my $line = <$fh>) {
		next if $line =~ /^\#/;
		
		$line =~ s/^\s+//;
		$line =~ s/\s+\z//;
		next unless $line;
		
		my ($pos, $data, undef) = split /\s+/, $line, 3;
		next unless $data;
		
		$res->[$pos] = $data;
	}
	
	close $fh;
	
	$res;
}

sub print_res {
	my ($res, $name, $myhtml_encoding_list) = @_;
	
	convert_name($name);
	
	my $var_name = "myhtml_encoding_map_$name";
	
	if($name =~ /gb18030_ranges/) {
		print "static const unsigned long $var_name"."[][2] = {\n";
	}
	else {
		print "static const unsigned long $var_name"."[] = {\n";
		
		$myhtml_encoding_list->{"MyHTML_ENCODING_". uc($name)} = $var_name;
	}
	
	foreach my $key (0..$#$res) {
		if (ref $res->[$key]) {
			print "\t" unless $key % 5;
			print "{$res->[$key][0], $res->[$key][1]}, ";
			print "\n" unless ($key + 1) % 5;
		}
		else {
			print "\t" unless $key % 10;
			print $res->[$key]||'0x0000', ", ";
			#print sprintf("0x%02x\n", $num);
			print "\n" unless ($key + 1) % 10;
		}
	}
	
	print "\n};\n\n";
}

sub convert_name {
	$_[0] =~ s/\.txt$//;
	$_[0] =~ s/[-\s]+/_/g;
}



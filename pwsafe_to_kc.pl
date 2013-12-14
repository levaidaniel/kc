#!/usr/bin/perl -w

use strict;
use warnings;
use HTML::Entities;


if ($#ARGV < 1  or  $ARGV[0] eq '-h'  or  $ARGV[0] eq '--help') {
	print "Usage:\n$0 <pwsafe export file> <kc xml output file>\n";
	print "\nConverts an exported pwsafe database to a kc compatible XML database.\n";
	print "\nThere were some shallow tests using pwsafe 0.2.0 with database version 2.\nPlease report any misbehaviour.\n";
	print "\n$0 was written by LEVAI Daniel <leva\@ecentrum.hu>\nSource, information, bugs:\nhttps://github.com/levaidaniel/kc\n";
	exit(1);
}

my $PWSAFE_EXPORT_FILE = shift;
my $KC_XML_FILE = shift;

print "opening ${PWSAFE_EXPORT_FILE} for reading.\n";
open(INPUT, '<', $PWSAFE_EXPORT_FILE)  or  die "couldn't open pwsafe export file '${PWSAFE_EXPORT_FILE}': $!\n";

print "opening ${KC_XML_FILE} for writing.\n";
open(OUTPUT, '>', $KC_XML_FILE)  or  die "couldn't open kc xml output file '${KC_XML_FILE}': $!\n";
print OUTPUT '<?xml version="1.0" encoding="UTF-8"?>' . "\n" . '<kc>' . "\n";

my $items = 0;
my $keychains = 0;
my $chain_prev = '';
my $i = 0;

print "Converting...\n";

print "--[No.\t[Chain]\tKey:\tValue]--\n";

while (<INPUT>) {
	chomp;

	! /^"/  &&  next;

	$i++;

	(undef, my $chain, undef, my $key, my $value, undef) = split;
	$chain =~ s/^"(.*)"$/$1/;
	$chain ||= 'default';
	$key =~ s/^"(.*)"$/$1/;
	$value =~ s/^"(.*)"$/$1/;

	print "$i.:\t[${chain}] ${key}: ${value}\n";

	# Somehow after exporting from pwsafe there is a possibility that from some strings
	# wich contain HTML character entities (&gt; &lt; &quot; etc...) the ';' (semicolon)
	# character is missing at the end.
	# This is not permitted with XML, and the resulting string would be read back without
	# decoding the unclosed character entity and without the characters after it, until
	# the string's end or a new character entity is reached (a starting '&' character) with
	# a proper closing semicolon.
	# So, we decode the string and then reencode it, because it seems the HTML::Entities
	# module deals well with unclosed HTML character entities.
	for my $stuff (\$chain, \$key, \$value) {
		if ($$stuff =~ m/&[a-z]+;/) {
			$$stuff = encode_entities(decode_entities($$stuff));
		}
	}

	if ($chain ne $chain_prev) {
		$keychains++;

		if ($i != 1) {
			print OUTPUT "\t" . '</keychain>' . "\n";
		}

		print OUTPUT "\t" . '<keychain name="' . ($chain eq '' ? 'default' : $chain) . '">' . "\n";
	}

	$items++;
	print OUTPUT "\t\t" . '<key name="' . $key . '"' . ' value="' . $value . '"' . ' created="' . time() . '"' . ' modified="' . time() . '"' . '/>' . "\n";

	$chain_prev = $chain;
}

print OUTPUT "\t" . '</keychain>' . "\n";
print OUTPUT '</kc>' . "\n";

close(INPUT);
close(OUTPUT);

printf("Converted %d keychains with %d items.\n", $keychains, $items);

#!/usr/bin/perl

use String::Random;

my $s = String::Random->new;
my %map;

my $header = << '<end>'; 
#include "../general.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
	int result;
	char *value;
	puts("BEGINNING TEST OF 'general' MODULE");
	map_s test_map;

    map_init(&test_map);
	
<end>


my $insertTests = "\tputs(\"---EXECUTING MAP INSERT TESTS---\");\n";
for(my $i = 0; $i < 1000; $i++) {
	my $key = $s->randregex('\d*\w*\s*');
	my $val = $s->randregex('\d+\w+\s+');
	$insertTests .= "\tresult = map_insert(&test_map, \"$key\", \"$val\");\n";
	if(not exists $map{$key}) {
		$map{$key} = $val;
		$insertTests .= "\tassert(result == 0);\n";
	}
	else {
		$insertTests .= "\tassert(result == -1);\n";
	}
}

while(my($key, $val) = each %map) {
	$insertTests .= "\tresult = map_insert(&test_map, \"$key\", \"$val\");\n";
	$insertTests .= "\tassert(result == -1);\n";
}

my $getTests = "\tputs(\"---EXECUTING MAP GET TESTS---\");\n";
while(my($key, $val) = each %map) {
	$getTests .= "\tvalue = map_get(&test_map, \"$key\");\n";
	$getTests .= "\tassert(!strcmp(value, \"$val\"));\n";
}

my $deleteTests = "\tputs(\"---EXECUTING MAP DELETE TESTS---\");\n";
while(my($key, $val) = each %map) {
	$deleteTests .= "\tvalue = map_delete(&test_map, \"$key\");\n";
	$deleteTests .= "\tassert(!strcmp(value, \"$val\"));\n";
}

while(my($key, $val) = each %map) {
	$deleteTests .= "\tvalue = map_get(&test_map, \"$key\");\n";
	$deleteTests .= "\tassert(value == NULL);\n";
}

my $footer = << '<end>';

	map_dealloc(&test_map);
	puts("SUCCESS: ENDING TEST OF 'general' MODULE");
	return 0;
}

<end>

print "$header$insertTests$getTests$deleteTests$footer";


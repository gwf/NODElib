#!/usr/bin/perl

# Program to generate ASCII text for HTML
# Created by  James R. Davis, July 15 1994

# get directory where this file is.  
{$0 =~ /^(.*)\/.*$/;  $my_dir = $1; 
 if ($my_dir !~ ?^/?) {$my_dir = $ENV{PWD} . "/" . $my_dir;}
 if ($my_dir =~ ?/$?) {chop ($my_dir);}}
push(@INC, $my_dir);

# Parse command line arguments.
$file = "";

while ($#ARGV >=0) {
    $arg = shift;
    if ($arg =~ /^-./) {
	if ($arg =~ /-width/) {
	    if ($#ARGV == -1) {die "Missing value for $arg\nStopped ";}
	    $columns_per_line= shift;}
	else {
	    die "Unrecognized switch $arg.\nStopped";}}
    else {
	$file = $arg;
    }}

if ($file eq "") {
    die "Missing argument (HTML input file)\n";}

# HTML parser
# Jim Davis, July 15 1994

# This is an HTML parser not an SGML parser.  It does not parse a DTD,
# The DTD is implicit in the code, and specific to HTML.  
# The processing of the HTML can be customized by the user by
#   1) Defining routines to be called for various tags (see Begin and End arrays)
#   2) Defining routines html_content and html_whitespace

# This is not a validating parser.   It does not check the content model
# eg you can use DT outside a DL and it won't know.  It is too liberal in
# what tags are allowed to minimize what other tags.

# modified 3 Aug to add a bunch of HTML 2.0 tags
# modified 3 Sept to print HTML stack to STDERR not STDOUT, to add new
#  routines html_begin_doc and html_end_doc for application specific cleanup
#  and to break parse_html into two pieces.
# modified 30 Sept 94.  parse_attributes now handles tag attributes that
#   don't have values.  thanks to  Bill Simpson-Young <bill@syd.dit.csiro.au>
#   for the code.
# modified 17 Apr 95 to support FORMS tags.
# modified 3 Dec 95 to support U, tolerate TABLE, TD, TR, TH
# modified 17 May 96 to support &# syntax for entities.  Thanks to
#  Robert Brown, dummy@c2.org

$debug = 0;

$whitespace_significant = 0;

# global variables: 
#  $line_buffer is line buffer
#  $line_count is input line number.

$line_buffer = "";
$line_count = 0;

sub parse_html {
    local ($file) = @_;
    open (HTML, $file) || die "Could not open $file: $!\nStopped";
    &parse_html_stream ();
    close (HTML);}

# Global input HTML is the handle to the stream of HTML
sub parse_html_stream {
    local ($token, $new);

    ## initialization
    @stack=();
    $line_count = 0;
    $line_buffer = "";

    ## application specific initialization
    &html_begin_doc();
  main:
    while (1) {

	# if whitespace does not matter, trim any leading space.
	if (! $whitespace_significant) {
	    $line_buffer =~ s/^\s+//;}

	# now dispatch on the type of token

	if ($line_buffer =~ /^(\s+)/) {
	    $token = $1;
	    $line_buffer = $';
	    &html_whitespace ($token);}

	# This will lose if there is more than one comment on the line!
	elsif ($line_buffer =~ /^(\<!--.*-->)/) {
	    $token = $1;
	    $line_buffer = $';
	    &html_comment ($token);}

	elsif ($line_buffer =~ /^(\<![^-][^\>]*\>)/) {
	    $token = $1;
	    $line_buffer = $';
	    &html_comment ($token);}

	elsif ($line_buffer =~ /^(\<\/[^\>]*\>)/) {
	    $token = $1;
	    $line_buffer = $';
	    &html_etag ($token);}

	elsif ($line_buffer =~ /^(\<[^!\/][^\>]*\>)/) {
	    $token = $1;
	    $line_buffer = $';
	    &html_tag ($token);}

	elsif ($line_buffer =~ /^([^\s<]+)/) {
	    $token = $1;
	    $line_buffer = $';
	    $token = &substitute_entities($token);
	    &html_content ($token); }

	else {
	    # No valid token in buffer.  Maybe it's empty, or maybe there's an
	    # incomplete tag.  So get some more data.
	    $new = <HTML>;
	    if (! defined ($new)) {last main;}
	    # if we're trying to find a match for a tag, then get rid of embedded newline
	    # this is, I think, a kludge
	    if ($line_buffer =~ /^\</ && $line_buffer =~ /\n$/) {
		chop $line_buffer;
		$line_buffer .= " ";}
	    $line_buffer .= $new;
	    $line_count++;}
    }

    ## cleanup
    &html_end_doc();

    if ($#stack > -1) {
	print STDERR "Stack not empty at end of document\n";
	&print_html_stack();}
}


sub html_tag {
    local ($tag) = @_;
    local ($element) = &tag_element ($tag);
    local (%attributes) = &tag_attributes ($tag);

    # the tag might minimize (be an implicit end) for the previous tag
    local ($prev_element);
    while (&Minimizes(&stack_top_element(), $element)) {
	$prev_element = &stack_pop_element ();
	if ($debug)  {
	    print STDERR "MINIMIZING $prev_element with $element on $line_count\n";}
	&html_end ($prev_element, 0);}

    push (@stack, $tag);

    &html_begin ($element, $tag, *attributes);

    if (&Empty($element)) {
	pop(@stack);
	&html_end ($element, 0);}
}

sub html_etag {
    local ($tag) = @_;
    local ($element) = &tag_element ($tag);

    # pop stack until find matching tag.  This is probably a bad idea,
    # or at least too general.
    local ( $prev_element) = &stack_pop_element();
    until ($prev_element eq $element) {
	if ($debug) {
	    print STDERR "MINIMIZING $prev_element with /$element on $line_count \n";}
	&html_end ($prev_element, 0);

	if ($#stack == -1) {
	    print STDERR "No match found for /$element.  You will lose\n";
	    last;}
	$prev_element = &stack_pop_element();}

    &html_end ($element, 1);
}


# For each element, the names of elements which minimize it.
# This is of course totally HTML dependent and probably I have it wrong too
$Minimize{"DT"} = "DT:DD";
$Minimize{"DD"} = "DT";
$Minimize{"LI"} = "LI";
$Minimize{"P"} = "P:DT:LI:H1:H2:H3:H4:BLOCKQUOTE:UL:OL:DL";
$Minimize{"TR"} = "TR:TD:TH";
$Minimize{"TD"} = "TD:TH";
$Minimize{"TH"} = "TD:TH";

# Does element E2 minimize E1?
sub Minimizes {
    local ($e1, $e2) = @_;
    local ($value) = 0;
    foreach $elt (split (":", $Minimize{$e1})) {
	if ($elt eq $e2) {$value = 1;}}
    $value;}

$Empty{"BASE"} = 1;
$Empty{"BR"} = 1;
$Empty{"HR"} = 1;
$Empty{"IMG"} = 1;
$Empty{"ISINDEX"} = 1;
$Empty{"LINK"} = 1;
$Empty{"META"} = 1;
$Empty{"NEXTID"} = 1;
$Empty{"INPUT"} = 1;

# Empty tags have no content and hence no end tags
sub Empty {
    local ($element) = @_;
    $Empty{$element};}


sub print_html_stack {
    print STDERR "\n  ==\n";
    foreach $elt (reverse @stack) {print STDERR "  $elt\n";}
    print STDERR "  ==========\n";}

# The element on top of stack, if any.
sub stack_top_element {
    if ($#stack >= 0) {	&tag_element ($stack[$#stack]);}}

sub stack_pop_element {
    &tag_element (pop (@stack));}

# The element from the tag, normalized.
sub tag_element {
    local ($tag) = @_;
    $tag =~ /<\/?([^\s>]+)/;
    local ($element) = $1;
    $element =~ tr/a-z/A-Z/;
    $element;}

# associative array of the attributes of a tag.
sub tag_attributes {
    local ($tag) = @_;
    $tag =~ /^<[A-Za-z]+\s+(.*)>$/;
    &parse_attributes($1);}

# string should be something like
# KEY="value" KEY2="longer value"  KEY3="tags o doom"
# output is an associative array (like a lisp property list)
# attributes names are not case sensitive, do I downcase them
# Maybe (probably) I should substitute for entities when parsing attributes.

sub parse_attributes {
    local ($string) = @_;
    local (%attributes);
    local ($name, $val);
  get: while (1) {
      if ($string =~ /^ *([A-Za-z]+)=\"([^\"]*)\"/) {
	  $name = $1;
	  $val = $2;
	  $string = $';
	  $name =~ tr/A-Z/a-z/;
	  $attributes{$name} = $val; }
      elsif ($string =~ /^ *([A-Za-z]+)=(\S*)/) {
	  $name = $1;
	  $val = $2;
	  $string = $';
	  $name =~ tr/A-Z/a-z/;
	  $attributes{$name} = $val;}
      elsif ($string =~ /^ *([A-Za-z]+)/) {
	  $name = $1;
	  $val = "";
	  $string = $';
	  $name =~ tr/A-Z/a-z/;
	  $attributes{$name} = $val;}
      else {last;}}
    %attributes;}

sub substitute_entities {
    local ($string) = @_;
    $string =~ s/&amp;/&/g;
    $string =~ s/&lt;/</g;
    $string =~ s/&gt;/>/g;
    $string =~ s/&quot;/\"/g;
    local($ch);
    while (/&#([^;]*);/g) {
	   $ch=sprintf("%c", $1);
	   $string =~ s/&#[^;]*;/$ch/;
       }
    $string;}


@HTML_elements = (
		  "A",
		  "ADDRESS",
		  "B",
		  "BASE",
		  "BLINK",	#  Netscape addition :-(
		  "BLOCKQUOTE",
		  "BODY",
		  "BR",
		  "CITE",
		  "CENTER",	# Netscape addition :-(
		  "CODE",
		  "DD",
		  "DIR",
		  "DFN",
		  "DL",
		  "DT",
		  "EM",
		  "FORM",
		  "H1", "H2", "H3", "H4", "H5", "H6",
		  "HEAD",
		  "HR",
		  "HTML",
		  "I",
		  "ISINDEX",
		  "IMG",
		  "INPUT",
		  "KBD",
		  "LI",
		  "LINK",
		  "MENU",
		  "META",
		  "NEXTID",
		  "OL",
		  "OPTION",
		  "P",
		  "PRE",
		  "SAMP",
		  "SELECT",
		  "STRIKE",
		  "STRONG",
		  "TABLE",
		  "TD",
		  "TH",
		  "TR",
		  "TITLE",
		  "TEXTAREA",
		  "TT",
		  "U",
		  "UL",
		  "VAR",
		  );

sub define_element {
    local ($element) = @_;
    $Begin{$element} = "Noop";
    $End{$element} = "Noop";}

foreach $element (@HTML_elements) {&define_element($element);}

# do nothing
sub Noop {
    local ($element, $xxx) = @_;}

# called when a tag begins.  Dispatches using Begin
sub html_begin {
    local ($element, $tag, *attributes) = @_;

    local ($routine) = $Begin{$element};
    if ($routine eq "") {
	print STDERR "Unknown HTML element $element ($tag) on line $line_count\n";}
    else {eval "&$routine;"}}

# called when a tag ends.  Explicit is 0 if tag end is because of minimization
# not that you should care.
sub html_end {
    local ($element, $explicit) = @_;
    local ($routine) = $End{$element};
    if ($routine eq "") {
	print STDERR "Unknown HTML element \"$element\" (END $explicit) on line $line_count\n";}
    else {eval "&$routine(\"$element\", $explicit)";}}

sub html_content {
    local ($word) = @_;
}

sub html_whitespace {
    local ($whitespace) = @_;}

sub html_comment {
    local ($tag) = @_;}

# redefine these for application-specific initialization and cleanup

sub html_begin_doc {}

sub html_end_doc {}

# return a "true value" when loaded by perl.
1;




#!/usr/bin/perl
# Routines for HTML to ASCII.
# (fixed width font, no font changes for size, bold, etc) with a little

# BUGS AND MISSING FEATURES
#  font tags (e.g. CODE, EM) cause an extra whitespace 
#   e.g. <TT>foo</TT>, -> foo ,

# Jim Davis July 15 1994
# modified 3 Aug 94 to support MENU and DIR

# Simple text formatter
# Jim Davis 17 July 94

# current page, line, and column numbers.
$page = 1;
$line = 1;
$column = 1;

$left_margin = 1;
$right_margin = 72;
# lines on page before footer.  or 0 if no limit.
$bottom_margin = 58;

# add newlines to make page be full length?
$fill_page_length = 1;

sub print_word_wrap {
    local ($word) = @_;
    if (($column + ($whitespace_significant ? 0 : 1)
	 + length ($word) ) > ($right_margin + 1)) {
	&fresh_line();}
    if ($column > $left_margin && !$whitespace_significant) {
	print " ";
	$column++;}
    print $word;
    $column += length ($word);}
					 

sub print_whitespace {
    local ($char) = @_;
    if ($char eq " ") {
	$column++;
	print " ";}
    elsif ($char eq "\t") {
	&get_to_column (&tab_column($column));}
    elsif ($char eq "\n") {
	&new_line();}
    else {
	die "Unknown whitespace character \"$char\"\nStopped";}
    }

sub tab_column {
    local ($c) = @_;
    (int (($c-1) / 8) + 1) * 8 + 1;}

sub fresh_line {
    if ($column > $left_margin) {&new_line();}
    while ($column < $left_margin) {
	print " "; $column++;}}				 

sub finish_page {
    # Add extra newlines to finish page. 
    # You might not want to do this on the last page.
    if ($fill_page_length) {
	while ($line < $bottom_margin) {&cr();}}
    &do_footer ();
    $line = 1; $column = 1;}

sub start_page {
    if ($page != 1) {
	&do_header ();}}

sub print_n_chars {
    local ($n, $char) = @_;
    local ($i);
    for ($i = 1; $i <= $n; $i++) {print $char;}
    $column += $n;}

# need one NL to end current line, and then N to get N blank lines.
sub skip_n_lines {
    local ($n, $room_left) = @_;
    if ($bottom_margin > 0 && $line + $room_left >= $bottom_margin) {
	&finish_page();
	&start_page();}
    else {
	local ($i);
	for ($i = 0; $i <= $n; $i++) {&new_line();}}}

sub new_line {
    if ($bottom_margin > 0 && $line >= $bottom_margin) {
	&finish_page();
	&start_page();}
    else {&cr();}
    &print_n_chars ($left_margin - 1, " ");}

# used in footer and header where we don't respect the bottom margin.
sub print_blank_lines {
    local ($n) = @_;
    local ($i);
    for ($i = 0; $i < $n; $i++) {&cr();}}
    

sub cr {
    print "\n";
    $line++;
    $column = 1;}


# left, center, and right tabbed items

sub print_lcr_line {
    local ($left, $center, $right) = @_;
    &print_tab_left (1, $left);
    &print_tab_center (($right_margin - $left_margin) / 2, $center);
    &print_tab_right ($right_margin, $right);
    &cr();}

sub print_tab_left {
    local ($tab_column, $string) = @_;
    &get_to_column ($tab_column);
    print  $string;     $column += length ($string);
}

sub print_tab_center {
    local ($tab_column, $string) = @_;
    &get_to_column ($tab_column - (length($string) / 2));
    print $string;     $column += length ($string);
}

sub print_tab_right {
    local ($tab_column, $string) = @_;
    &get_to_column ($tab_column - length($string));
    print $string;
    $column += length ($string);
}


sub get_to_column {
    local ($goal_column) = @_;
    if ($column > $goal_column) {print " "; $column++;}
    else {
	while ($column < $goal_column)  {
	    print " "; $column++;}}}



# Can be set by command line arg
if (! defined($columns_per_line)) {
    $columns_per_line = 72;}

if (! defined($flush_last_page)) {
    $flush_last_page = 1;}

# amount to add to indentation
$indent_left = 4;
$indent_right = 4;

# ignore contents inside HEAD.
$ignore_text = 0;

# Set variables in tformat
$left_margin = 1;
$right_margin = $columns_per_line;
$bottom_margin = 0;

## Routines called by html.pl
$Begin{"HEAD"} = "begin_head";
$End{"HEAD"} = "end_head";

sub begin_head {
    local ($element, $tag) = @_;
    $ignore_text = 1;}

sub end_head {
    local ($element) = @_;
    $ignore_text = 0;}

$Begin{"BODY"} = "begin_document";

sub begin_document {
    local ($element, $tag) = @_;
    &start_page();}

$End{"BODY"} = "end_document";

sub end_document {
    local ($element) = @_;
    &fresh_line();}

## Headers

$Begin{"H1"} = "begin_header";
$End{"H1"} = "end_header";

$Begin{"H2"} = "begin_header";
$End{"H2"} = "end_header";

$Begin{"H3"} = "begin_header";
$End{"H3"} = "end_header";

$Begin{"H4"} = "begin_header";
$End{"H4"} = "end_header";

$Skip_Before{"H1"} = 1;
$Skip_After{"H1"} = 1;

$Skip_Before{"H2"} = 1;
$Skip_After{"H2"} = 1;

$Skip_Before{"H3"} = 1;
$Skip_After{"H3"} = 0;

sub begin_header {
    local ($element, $tag) = @_;
    &skip_n_lines ($Skip_Before{$element}, 5);}

sub end_header {
    local ($element) = @_;
    &skip_n_lines ($Skip_After{$element});}

$Begin{"BR"} = "line_break";

sub line_break {
    local ($element, $tag) = @_;
    &fresh_line();}

$Begin{"P"} = "begin_paragraph";

# if fewer than this many lines left on page, start new page
$widow_cutoff = 5;

sub begin_paragraph {
    local ($element, $tag) = @_;
    &skip_n_lines (1, $widow_cutoff);}

$Begin{"BLOCKQUOTE"} = "begin_blockquote";
$End{"BLOCKQUOTE"} = "end_blockquote";

sub begin_blockquote {
    local ($element, $tag) = @_;
    $left_margin += $indent_left;
    $right_margin = $columns_per_line - $indent_right;
    &skip_n_lines (1);}

sub end_blockquote {
    local ($element) = @_;
    $left_margin -= $indent_left;
    $right_margin = $columns_per_line;
    &skip_n_lines (1);}

$Begin{"PRE"} = "begin_pre";
$End{"PRE"} = "end_pre";

sub begin_pre {
    local ($element, $tag) = @_;
    $whitespace_significant = 1;}

sub end_pre {
    local ($element) = @_;
    $whitespace_significant = 0;}

$Begin{"INPUT"} = "form_input";

sub form_input {
    local ($element, $tag, *attributes) = @_;
    if ($attributes{"value"} ne "") {
	&print_word_wrap($attributes{"value"});}}

$Begin{"HR"} = "horizontal_rule";

sub horizontal_rule {
    local ($element, $tag) = @_;
    &fresh_line ();
    &print_n_chars ($right_margin - $left_margin + 1, "-");}

# Add code for IMG (use ALT attribute)
# Ignore I, B, EM, TT, CODE (no font changes)

## List environments

$Begin{"UL"} = "begin_itemize";
$End{"UL"} = "end_list_env";

$Begin{"OL"} = "begin_enumerated";
$End{"OL"} = "end_list_env";

$Begin{"MENU"} = "begin_menu";
$End{"MENU"} = "end_list_env";

$Begin{"DIR"} = "begin_dir";
$End{"DIR"} = "end_list_env";

$Begin{"LI"} = "begin_list_item";

# application-specific initialization routine
sub html_begin_doc {
    @list_stack = ();
    $list_type = "bullet";
    $list_counter = 0;}

sub push_list_env {
    push (@list_stack, join (":", $list_type, $list_counter));}

sub pop_list_env {
    ($list_type, $list_counter) = split (":", pop (@list_stack));
    $left_margin -= $indent_left;}

sub begin_itemize {
    local ($element, $tag) = @_;
    &push_list_env();
    $left_margin += $indent_left;
    $list_type = "bullet";
    $list_counter = "*";}

sub begin_menu {
    local ($element, $tag) = @_;
    &push_list_env();
    $left_margin += $indent_left;
    $list_type = "bullet";
    $list_counter = "*";}

sub begin_dir {
    local ($element, $tag) = @_;
    &push_list_env();
    $left_margin += $indent_left;
    $list_type = "bullet";
    $list_counter = "*";}

sub begin_enumerated {
    local ($element, $tag) = @_;
    &push_list_env();
    $left_margin += $indent_left;
    $list_type = "enumerated";
    $list_counter = 1;}

sub end_list_env {
    local ($element) = @_;
    &pop_list_env();
#    &fresh_line();
}

sub begin_list_item {
    local ($element, $tag) = @_;
    $left_margin -= 2;
    &fresh_line();
    &print_word_wrap("$list_counter ");
    if ($list_type eq "enumerated") {$list_counter++;}
    $left_margin += 2;}

$Begin{"DL"} = "begin_dl";

sub begin_dl {
    local ($element, $tag) = @_;
    &skip_n_lines(1,5);}
    
$Begin{"DT"} = "begin_defined_term";
$Begin{"DD"} = "begin_defined_definition";
$End{"DD"} = "end_defined_definition";

sub begin_defined_term {
    local ($element, $tag) = @_;
    &fresh_line();}

sub begin_defined_definition {
    local ($element, $tag) = @_;
    $left_margin += $indent_left;
    &fresh_line();}

sub end_defined_definition {
    local ($element) = @_;
    $left_margin -= $indent_left;
    &fresh_line();}

$Begin{"META"} = "begin_meta";

# a META tag sets a value in the assoc array %Variable
# i.e. <META name="author" content="Rushdie"> sers $Variable{author} to "Rushdie"
sub begin_meta {
    local ($element, $tag, *attributes) = @_;
    local ($variable, $value);
    $variable = $attributes{name};
    $value = $attributes{content};
    $Variable{$variable} = $value;}

$Begin{"IMG"} = "begin_img";

sub begin_img {
    local ($element, $tag, *attributes) = @_;
    &print_word_wrap (($attributes{"alt"} ne "") ? $attributes{"alt"} : "[IMAGE]");}

# Content and whitespace.

sub html_content {
    local ($string) = @_;
    unless ($ignore_text) {
	&print_word_wrap ($string);}}

sub html_whitespace {
    local ($string) = @_;
    if (! $whitespace_significant) {
	die "Internal error, called html_whitespace when whitespace was not significant";}
    local ($i);
    for ($i = 0; $i < length ($string); $i++) {
	&print_whitespace (substr($string,$i,1));}}

# very minimal tables
$End{"TR"} = "end_table_row";
sub end_table_row {
    local ($element) = @_;
    &fresh_line();
}


    

# called by tformat.  Do nothing.
sub do_footer {
}

sub do_header {
}


1;


&parse_html ($file);


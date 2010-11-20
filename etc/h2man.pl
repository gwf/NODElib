
# Converts header files into man pages.  See this man page for
# details.  This is my first perl script ever, so please feel
# free to improve on it. -- Gary Flake.


######################################################################

# Returns the first location in a list of the occurance of a pattern.

sub first_loc {
  local($where);
  local($pat, *array) = ($_[0], $_[1]);
  undef $where;
  for ($[ .. $#array) {
    $where = $_, last if $array[$_] =~ /$pat/;
  }
  $where;
}

######################################################################

# Returns the first location in a list of the occurance of a pattern
# starting from some supplied starting point.

sub first_loc_from {
  local($where);
  local($pat, $beg, *array) = ($_[0], $_[1], $_[2]);
  undef $where;
  for ($beg .. $#array) {
    $where = $_, last if $array[$_] =~ /$pat/;
  }
  $where;
}

######################################################################

# Formats a chunk of text into nroff format.  Most of the work
# is in handling itemized lists.  The rest is just some simple
# reformatting.

sub do_text {
  local(*txt) = $_[0];
  local($item) = 0;
  local($verb) = 0;
  local($pref) = "";
  local($postf) = "";
  foreach(@txt) {
    s/^   //g;
    if(/\\begin{verbatim}/) {
      s/\\begin{verbatim}/.LP\n.nf\n/g;
      $verb = 1;
    }
    if(/\\end{verbatim}/) {
      s/\\end{verbatim}/.fi\n.LP/g;
      $verb = 0;
    }

    if(/\\bf *$/) {
      $pref = ".B \"";
      $postf = "\"";
      s/ *\\bf *[\n]//g;
    }
    elsif(/\\em *$/) {
      $pref = ".I \"";
      $postf = "\"";
      s/ *\\em *[\n]//g;
    }
    elsif(/\\rm *$/) {
      $pref = "";
      $postf = "";
      s/ *\\rm *[\n]//g;
    }

    s/^(..*)$/$pref\1$postf/g;
    s/^ *\n/.LP\n/g;

    s/ *\\begin{itemize} */.RS\n/g;
    s/ *\\end{itemize} *\n*/.RE\n/g;

    s/\\indent *(.*)/\n.RS\n\1\n.RE\n/g;
    s/^ *\\item */.br\n/g;
    s/\\it/\\em/g;
    s/\\newline/\n.br\n/g;
    s/\\section{([^}]*)}/.SS \1/g;

    # convert zzz\em{xxx}yyy to zzz\c\n.I xxx\c\nyyy
    s/([^ ])\\em\{([^\}]*)}([^ \n][^ \n]*)/\1\\\\c\\n.I \2\\\\c\\n\3/g;
    # convert \em{xxx}yyy to \n.I xxx\c\nyyy
    s/\\em\{([^\}]*)}([^ \n][^ \n]*)/\\n.I \1\\\\c\\n\2/g;
    # convert \em{xxx} to \n.I xxx\n
    s/\\em\{([^\}]*)}/\\n.I \1\\n/g;

    # convert zzz\bf{xxx}yyy to zzz\c\n.B xxx\c\nyyy
    s/([^ ])\\bf\{([^\}]*)}([^ \n][^ \n]*)/\1\\\\c\\n.B \2\\\\c\\n\3/g;
    # convert \bf{xxx}\n to \n.B xxx\n
    s/\\bf\{([^\}]*)}\\n/\\n.B \1\\n/g;
    # convert \bf{xxx}yyy to \n.B xxx\c\nyyy
    s/\\bf\{([^\}]*)}([^ \n][^ \n]*)/\\n.B \1\\\\c\\n\2/g;
    # convert \bf{xxx} to \n.B xxx\n
    s/\\bf\{([^\}]*)}/\\n.B \1\\n/g;

    # convert \url{anchor}{href}
    s/\\url\{([^\}]*)\}\{([^\}]*)\}/$1/g;

    # convert \qurl{href}
    s/\\qurl\{([^\}]*)\}/$1/g;

    s/\\n$//g;
    s/^\\n//g;
    s/\\n *\\n/\\n/g;
    s/(.)\\n *\\n/\1\\n/g;

    if($verb == 0) {
      s/^\s\s*([^\s].*)/\1/g;	# Remove leading spaces.
    }
    s/\\n\s*/\n/g;		# Insert requested newlines
    s/(.*[^\\])\\\n/\1/g;	# Remove "\\\n".
    if($verb == 0) {
      s/^\s\s*(.*\n)/\1/g;	# Remove leading white-space.
    }
    s/\\(.)/\1/g;		# Replace any remaining "\X" with "X".
    s/ - / \\- /g;		# Add "\" in front of "-".
    s/^ *\\-/\\-/g;

    # Fix broken period.
    s/\n\.  *(.*)\n/\n.RI . \n\1\n/g;
    s/\n\.  *\n/\n.RI . \n/g;
    s/\n\.$/\n.RI . \n/g;

    s/\n\n\n*/\n/g;

    print OUTFILE $_;
  }
}

######################################################################

# Formats a whole section and removes it from the header comment.

sub do_section {
  local($sname) = $_[0];

  $i = &first_loc("^$sname", *header);
  if(!defined($i)) { return 0; }
  @tmp = @header[$i+1 .. $#header];
  $j = &first_loc("^[A-Za-z]+", *tmp);
  if(!defined($j)) { $j = $#tmp + 1; }
  @tmp = splice(@header, $i, ($j + 1));
  print OUTFILE ".SH $tmp[0]";
  @tmp = @tmp[1 .. $#tmp];
  foreach(@tmp) { s/^/ /g; }
  $item = 0;
  &do_text(*tmp);
  return 1;
}

######################################################################

sub convert_sub_proto {
    local($_) = $_[0];
    if(/^\.BI.*/) {
	s/^\.BI //g;
	s/([^\\]) /\1/g;
	s/\\(.)/\1/g;
    }
    return($_);
}


######################################################################

# Converts a prototype into a form such that nroff will bold types,
# and underline arguments.  Yes, this is disgusting, but it
# apparently works, and that's good enough for me right now.

sub convert_proto {
  local($proto) = $_[0];
  local($munge) = 0;
  local(@plines);
  local($i);
  $_ = $proto;
  s/[ \t]+/ /g;
  # strip out function pointer arg lists.
#  s/(\(\*\w*\))\([^\)]*\)/\1\(\)/g;
  if(length($_) > 56) {
    $munge = $_;
#    $munge =~ s/^([^\(]*)(\s|\*)\w+\(.*/\1/g;
    $munge =~ s/^([^\(]*)\(.*/\1/g;
    $munge = length($munge);
  }

  s/[ \t]+/\\ /g;
  s/(\w+),/ \1 ,\\ /g;
  s/(\w+)\)/ \1 )/g;
  s/\( void \)/(void)/g;
  s/\\ \\ /\\ /g;

  if($munge != 0) {
    $munge = '\ ' x $munge;
    s/^([^\(]+\([^\(]+)\s*,\\\s*\\*\s*/\1 ,\n.br\n.BI $munge/g;
    s/ ,\\ ([^\)]+\);)/ ,\n.br\n.BI $munge\1/g;
    s/\)\s*,\\\s*/),\n.br\n.BI $munge/g;

    if(/.*\(\* \w+ \)\(.*/) {

      @plines = split("\n", $_);
      $i = 0;
      foreach(@plines) {
        $psub = &convert_sub_proto($_);
	if(length($psub) > 62 && /.*\(\* \w+ \)\(.*/) {
	    $munge2 = $psub;
#	    $munge2 =~ s/(.*\)\()[^\(]*/\1/g;
#	    $munge2 =~ s/([^\(]+\(\*).*/\1/g;
            $munge2 =~ s/([^\(]+\(\*\w+\)\().*/\1/g;
	    $munge2 = length($munge2);
	    $munge2 = '\ ' x $munge2;
	    $_ = $plines[$i];
	    while(/ ,\\ ([^\),]+)/) {
		s/ ,\\ ([^\),]+)/ ,\n.br\n.BI $munge2\1/g;
	    }
	    $plines[$i] = $_;
        }
        $i++;
      }
      $_ = join("\n", @plines);
      $_ = "$_\n";
    }
  }

  $_;
}

######################################################################

# Kills empty lines and selected lines.  Types of kills:
#
# Kills just the one line.
# anything ... /* h2man:ignore */ anything-else on this line...
#
# Kills just the comment.
# /* h2man:ignore anything-else inside the comment... */
#
# Kills the next function prototype;
# /* h2man:ignore */
# foo();
# 
# Kills the next typedef.
# /* h2man:ignore */
# typedef ...
#
# Kills a whole block.
# /* h2man:skipbeg */
# any number of lines of anything
# /* h2man:skipend */

sub clean_rest {
  local($beg, $skip);
  local($i) = 0;
  local($len) = $#rest;
  for($i = 0; $i < $len; $i++) { # join line with /*\*/ seperating them.
    $_ = $rest[$i];
    if(/\/\*\\\*\/\s*$/) {
      $rest[$i] =~ s/\/\*\\\*\/\s*//g;
      $rest[$i] = $rest[$i] . $rest[$i+1];
      splice(@rest, $i+1, 1);
      $_ = $rest[$i];
      if(!/\/\*\\\*\/\s*$/) {
	$i++;
      }
      else {
	$i--;
      }
    }
  }
  for($i = 0; $i < $len; $i++) { # kill borderlike comments.
    $_ = $rest[$i];
    if(/^\s*\/\*\W*\*\/\s*$/) {
      splice(@rest, $i, 1);
    }
  }
  undef $beg;
  while(1) { # clean up h2man:ignore directives.
    $beg = &first_loc('.*\/\*\s*h2man:ignore.*', *rest);
    if(!defined($beg)) { last; }
    if($rest[$beg] =~ /^\/\*\s*h2man:ignore\s*\*\/.*\S+.*/) {
      splice(@rest, $beg, 1);
    }
    elsif($rest[$beg] =~ /.*\S+.*\/\*\s*h2man:ignore\s*\*\/.*/) {
      splice(@rest, $beg, 1);
    }
    elsif($rest[$beg] =~ /^\/\*\s*h2man:ignore.*\w+.*/) {
      $end = &first_loc_from('^.*\*\/.*', $beg, *rest);
      splice(@rest, $beg, $end - $beg + 1);
    }
    elsif($rest[$beg + 1] =~ /^\s*typedef.*/) {
      if($rest[$beg + 1] =~ /.*;.*/) {
	$end = $beg + 1;
      }
      else {
	$end = &first_loc_from('^\s*\}\s*\w+\s*;.*', $beg + 1, *rest);
      }
      if(!defined($end)) {
	splice(@rest, $beg, 1);
      }
      else {
	splice(@rest, $beg, $end - $beg + 1);
      }
    }
    elsif($rest[$beg + 1] =~ /^(\w(\w|\s|\*)+)\([^\)]*\)\s*;/) {
      splice(@rest, $beg, 2);
    }
    else {
      splice(@rest, $beg, 2);
    }
    undef $beg; undef $end;
  }
  @tmp = ();
  $skip = 0;
  $inside = 0;
  foreach(@rest) { # kill blank lines.
    if($skip && /^\s*\/\*\s*h2man:skipend.*/) {
      $skip = 0;
    }
    elsif(/^\s*\/\*\s*h2man:skipbeg.*/) {
      $skip = 1;
    }
    elsif(/.*\/\*.*\*\/\s*/) {
      @tmp = (@tmp, $_);
    }
    elsif(/.*\/\*.*/) {
      $inside = 1;
      if($skip == 0) {
	@tmp = (@tmp, $_);
      }
    }
    elsif(/.*\*\/.*/) {
      $inside = 0;
      if($skip == 0) {
	@tmp = (@tmp, $_);
      }
    }
    elsif((!/^\s*$/) && $skip == 0) {
      @tmp = (@tmp, $_);
    }
    elsif(/^\s*$/ && $skip == 0 && $inside == 1) {
      @tmp = (@tmp, $_);
    }
  }
  @rest = @tmp;
}

######################################################################

# Processes all typedefs

sub do_typedefs {
  local($cur, $beg, $end, $i, @com, @td, $first);
  undef $beg;
  
  $cur = 0;
  $first = 0;
  while(1) {
    undef $beg;
    $beg = &first_loc_from('^typedef.*', $cur, *rest);
    if(!defined($beg)) { return; }
    if($rest[$beg] =~ /.*;.*/) {
      $end = $beg;
    }
    else {
      $end = &first_loc_from('^}\s*\w+\s*;\s*', $beg + 1, *rest);
    }

    if($rest[$beg-1] =~ /.*\*\/\s*$/) {
      @td = @rest[$beg .. $end];
      $i = $beg - 1;
      while(!($rest[$i] =~ /^\/\*.*/)) {
	$i--;
      }
      @com = @rest[$i .. $beg - 1];
      $com[0] =~ s/.*\/\*(.*\n)/  \1/g;
      $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
      if($first == 0) {
        print OUTFILE ".SS Type Declarations\n.LP\n";
	print OUTFILE "The following types are defined in the header ";
	print OUTFILE "file \\c\n.IR $inname .\n";
	$first = 1;
      }
      print OUTFILE ".LP\n.nf\n";
      foreach (@td) {
        chop($_);
        print OUTFILE ".BI \"$_\"\n";
      }
      print OUTFILE ".fi\n.LP\n";
      &do_text(*com);
    }
    $cur = $end + 1;
  }
}

######################################################################

# Processes all prototypes for the synopsis section.

sub do_protos {
  local($cur, $beg, $end, $i, @com, $proto, $first, $okay);
  undef $beg;
  
  $cur = 0;
  $first = 0;
  while(1) {
    undef $beg;
    $okay = 0;
    while($okay != 1) {
      $beg = &first_loc_from('^\w+(\s|\*)+\w+(\w|\s|\*)*\(.*\)\s*;',
			     $cur, *rest);
      if(!defined($beg)) { return; }
      if(@rest[$beg] =~ /^\s*typedef/) {
	$cur = $beg + 1;
      }
      else {
        $okay = 1;
      }
    }
    $end = $beg;
    if($rest[$beg-1] =~ /.*\*\/\s*$/) {
      $proto = $rest[$beg];
      $i = $beg - 1;
      while(!($rest[$i] =~ /^\/\*.*/)) {
	$i--;
      }
      @com = @rest[$i .. $beg - 1];
      $com[0] =~ s/.*\/\*(.*\n)/  \1/g;
      $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
      if($first == 0) {
        print OUTFILE ".SS Function Definitions\n.LP\n";
	print OUTFILE "The following function prototypes are given ";
	print OUTFILE "in the header file \\c\n.IR $inname .\n";
	$first = 1;
      }
      print OUTFILE ".LP\n.nf\n";
      $proto = &convert_proto($proto);
      print OUTFILE ".BI $proto.br\n.fi\n";
      &do_text(*com);
    }
    $cur = $end + 1;
  }
}

######################################################################

# Processes all macros.

sub do_macros {
  local($cur, $beg, $end, $i, @com, $macro, $first);
  undef $beg;
  
  $cur = 0;
  $first = 0;
  while(1) {
    undef $beg;
    $beg = &first_loc_from('^#define \w+\([^\)]*\)', $cur, *rest);
    if(!defined($beg)) { return; }
    $end = $beg;
    if($rest[$beg-1] =~ /.*\*\/\s*$/) {
      $macro = $rest[$beg];
      $i = $beg - 1;
      while(!($rest[$i] =~ /^\/\*.*/)) {
	$i--;
      }
      @com = @rest[$i .. $beg - 1];
      $com[0] =~ s/.*\/\*(.*\n)/  \1/g;
      $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
      if($first == 0) {
        print OUTFILE ".SS Macro Definitions\n.LP\n";
	print OUTFILE "The following macros are defined ";
	print OUTFILE "in the header file \\c\n.IR $inname .\n";
	$first = 1;
      }
      print OUTFILE ".LP\n.nf\n";
      $macro = &convert_macro($macro);
      print OUTFILE ".BI $macro.br\n.fi\n";
      &do_text(*com);
    }
    $cur = $end + 1;
  }
}

######################################################################

# Takes comments of the form: /* h2man:include ... */ and puts
# the entire contents into a misc subsection under description.
# use this for globals.

sub do_extras {
  local($cur, $beg, $end, $i, @com, $first);
  undef $beg;
  
  $cur = 0;
  $first = 0;
  while(1) {
    undef $beg;
    $beg = &first_loc_from('^\/\*\s*h2man:include.*', $cur, *rest);
    if(!defined($beg)) { return; }
    $end = &first_loc_from('.*\*\/', $beg, *rest);
    @com = @rest[$beg .. $end];
    $com[0] =~ s/.*\/\*\s*h2man:include\s+(.*\n)/\1/g;
    $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
    if($first == 0) {
      print OUTFILE ".SS Miscellaneous Items\n.LP\n";
      $first = 1;
    }
    &do_text(*com);
    $cur = $end + 1;
  }
}

######################################################################

# converts a macro into a pretty printing form.

sub convert_macro {
  local($macro) = $_[0];
  $_ = $macro;
  s/^(#define) (\w+\([^\)]*\)).*/\1\\ \2 .../g;
  s/\(\s*/\( /g;
  s/\s*\)\s*/ \)\\ /g;
  s/\s*,\s*/ , \\ /g;
  $_;
}
    
######################################################################

# Does one whole file.

sub do_file {
  if($filter = 0) {
    open(INFILE, $inname) || die "h2man: can't open \"$inname\".\n";
    open(OUTFILE, ">$outname") || die "h2man: can't open \"$outname\".\n";  
  }
  else {
    open(INFILE, "-");
    open(OUTFILE, ">-");
  }
  
  @lines = <INFILE>;

  # Get the header comment
  $beg = &first_loc('^\s*\/\*', *lines);
  $end = &first_loc('^\s*\*\/', *lines);
  @header = @lines[$beg .. $end];
  if($header[0] =~ /^\s*\/\* [A-Z][A-Z]+\s*/) {
    $header[0] =~ s/^\s*\// /g;
  }
  else {
    splice(@header, 0, &first_loc('^ \* ([A-Z][A-Z]+\s)+', *header));
    pop(@header);
  }
  foreach(@header) {
    s/ \* (.*)/\1/g;
    s/^ \*$//g;
  }

  # Get everything but the header
  @rest = @lines[$end + 1 .. $#lines];
  &clean_rest();

  # Write out the starting info
  $date = `date '+%d %h %Y'`;
  chop($date);
  print OUTFILE ".TH $title $section \"$date\" \"$source\" \"$manual\"\n";

  if(! &do_section('NAME')) {
    print OUTFILE ".SH NAME\n$inname.\n";
  }

  if(! &do_section('SYNOPSIS')) {
    print OUTFILE ".SH SYNOPSIS\n";
  }
  @protos = grep(/^\w+(\s|\*)+\w+(\w|\s|\*)*\(.*\)\s*;/, @rest);
  @protos = grep(!/^ *typedef/, @protos);
  print OUTFILE ".LP\n.B #include <$inname>\n.LP\n";
  foreach(@protos) {
    if(/^\s*private/i) {
      next;
    }
    $tmp = &convert_proto($_);
    print OUTFILE ".BI $tmp.br\n";
  }
  @macros = grep(/^#define \w+\([^\)]*\)/, @rest);
  foreach(@macros) {
    if(/private/i) {
      next;
    }
    $tmp = &convert_macro($_);
    print OUTFILE ".BI $tmp.br\n";
  }

  # Description = typedefs, protos, macros, and globals.
  if(! &do_section('DESCRIPTION')) {
    print OUTFILE ".SH DESCRIPTION\n";
  }

  &do_typedefs();
  &do_protos();
  &do_macros();
  &do_extras();

  @sects = grep(/^\w+.*/, @header);
  foreach(@sects) {
    &do_section($_);
  }

  close(INFILE);
  close(OUTFILE);
}

######################################################################

sub main_loop {
  local($iiii);			# perl bug.  if named $i it wont work.
  $inname = $outname = $title = $section = $source = $manual = "";
  
  $filter = 1;
  for($iiii = 0; $iiii < $#ARGV + 1; $iiii++) {
    $_ = $ARGV[$iiii];
    if(/^-title$/) {
      $title = $ARGV[$iiii + 1];
      $iiii++;
    }
    elsif(/^-section$/) {
      $section = $ARGV[$iiii + 1];
      $iiii++;
    }
    elsif(/^-source$/) {
      $source = $ARGV[$iiii + 1];
      $iiii++;
    }
    elsif(/^-manual$/) {
      $manual = $ARGV[$iiii + 1];
      $iiii++;
    }
    elsif(/^-name/) {
      $inname = $ARGV[$iiii + 1];
      $iiii++;
    }
    elsif(/.*\.h$/) {
      $filter = 1;
      if($section eq "") {
	$section = "3";
      }
      $outname = $inname = $_;
      $outname =~ s/(.*)\.h/\1.$section/g;
      if($title eq "") {
	$title = $inname;
	$title =~ s/(.*).h/\1/g;
	$title =~ tr/a-z/A-Z/;
      }
      &do_file();
      $inname = $outname = $title = $section = $source = $manual = "";
    }
    else {
      die "h2man: bad option \"$_\".\n";
    }
  }
  if($filter == 1) {
    &do_file();
  }	
}

######################################################################

&main_loop();


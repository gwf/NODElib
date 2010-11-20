
# Converts header files into man pages.  See this man page for
# details.  This is my first perl script ever, so please feel
# free to improve on it. -- Gary Flake (gary.flake@usa.net).


$tablecolor = "\"#DDDDDD\"";

$begindent = "\
<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=0>\
<TR VALIGN=TOP><TD>\
<IMG ALT=\"\" SRC=\"pixel.gif\" BORDER=0 HEIGHT=1 WIDTH=20 ALIGN=BOTTOM>\
</TD><TD>\
";

$endindent = "</TD></TR></TABLE>\n";

#$begsec = "<p><table width=100% bgcolor=#dddddd border=0"
#	  . "cellspacing=0 cellpadding=5><tr><td><b><font size=+1>";
#
#$endsec = "</font></b></tr></td></table>\n";

$begsec = "<p><table width=100% bgcolor=#dddddd border=0"
	  . "cellspacing=0 cellpadding=5><tr><td><h3>";

$endsec = "</h3></tr></td></table>\n";

$state = 0;

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
    s/\/\*\\n\*\///g;
    s/^   //g;
    if(/\\begin{verbatim} *\n*/) {
      #$tmp1 = "<TABLE CELLSPACING=0 CELLPADDING=5 BGCOLOR=\"#E6E6E6\">";
      $tmp1 = "<TABLE CELLSPACING=0 CELLPADDING=5>";
      $tmp2 = "<TR NOSAVE><TD NOSAVE><PRE><TT>";
      s/\\begin{verbatim}/$tmp1$tmp2/g;
      $verb = 1;
    }
    if(/\\end{verbatim}/) {
      s/\\end{verbatim}/<\/TT><\/PRE><\/TD><\/TR><\/TABLE>\n/g;
      $verb = 0;
    }
    s/\\begin{itemize}/<UL>/g;
    s/\\end{itemize}/<\/UL>/g;


    if(/\\bf *$/) {
      s/ *\\bf *[\n]//g;
      if($state == 1) {
        print OUTFILE "</B>";
      }
      elsif($state == 2) {
	print OUTFILE "</I>";
      }
      elsif($state == 3) {
	print OUTFILE "</TT>";
      }
      print OUTFILE "<B>";
      $state = 1;
    }
    elsif(/\\em *$/) {
      s/ *\\em *[\n]//g;
      if($state == 1) {
        print OUTFILE "</B>";
      }
      elsif($state == 2) {
	print OUTFILE "</I>";
      }
      elsif($state == 3) {
	print OUTFILE "</TT>";
      }
      print OUTFILE "<B>";
      $state = 2;
    }
    elsif(/\\rm *$/) {
      s/ *\\rm *[\n]//g;
      if($state == 1) {
        print OUTFILE "</B>";
      }
      elsif($state == 2) {
	print OUTFILE "</I>";
      }
      elsif($state == 3) {
	print OUTFILE "</TT>";
      }
      $state = 0;
    }

    s/^(..*)$/$pref\1$postf/g;
    s/^ *\n/<P>\n/g;

    # Fix item -- add itemize.

    s/\\indent *(.*)/\n<UL>\n\1\n<\/UL>\n/g;
    s/\\item/<LI>/g;
    s/\\it/\\em/g;
    s/\\newline/\n<BR>\n/g;
    s/\\section{([^}]*)}/<h4>\1<\/h4>/g;

    # convert \em{xxx} to \n.I xxx\n
    s/\\em\{([^\}]*)}/<I>\1<\/I>/g;

    # convert \bf{xxx} to \n.B xxx\n
    s/\\bf\{([^\}]*)}/<B>\1<\/B>/g;

    # convert \url{anchor}{href}
    s/\\url\{([^\}]*)\}\{([^\}]*)\}/<a href="\2">\1<\/a>/g;

    # convert \qurl{href}
    s/\\qurl\{([^\}]*)\}/<a href="\1">\1<\/a>/g;

    s/\\n$//g;
    s/^\\n/<P>/g;
    s/\\n *\\n/\\n/g;
    s/(.)\\n *\\n/\1\\n/g;

    s/\\(.)/\1/g;		# Replace any remaining "\X" with "X".

    # Fix broken period.
    #s/\n\.  *(.*)\n/\n.RI . \n\1\n/g;
    #s/\n\.  *\n/\n.RI . \n/g;
    #s/\n\.$/\n.RI . \n/g;

    #s/\n\n\n*/\n/g;

    print OUTFILE $_;
  }
}

######################################################################

# Formats a whole section and removes it from the header comment.

sub do_section {
  local($sname) = $_[0];
  local($sname2);

  $i = &first_loc("^$sname", *header);
  if(!defined($i)) { return 0; }
  @tmp = @header[$i+1 .. $#header];
  $j = &first_loc("^[A-Za-z]+", *tmp);
  if(!defined($j)) { $j = $#tmp + 1; }
  @tmp = splice(@header, $i, ($j + 1));
  $sname2 = $tmp[0];
  $sname2 =~ s/\n$//g;
  print OUTFILE "$begsec" . "$sname2" . "$endsec";
  @tmp = @tmp[1 .. $#tmp];
  foreach(@tmp) { s/^/ /g; }
  $item = 0;
  print OUTFILE "$begindent";
  &do_text(*tmp);
  return 1;
}

######################################################################

# Converts a prototype into a form such that that is pretty printed.
# Yes, this is disgusting, but it apparently works, and that's good
# enough for me right now.

sub convert_proto {
  local($proto) = $_[0];
  local($munge) = 0;
  local(@plines);
  local($i);
  local($width) = 76;
  $_ = $proto;

  s/\/\*\\n\*\///g;
  s/[ \t]+/ /g;
  if(length($_) > $width) {
    $munge = $_;
    $munge =~ s/^([^\(]*)\(.*/\1/g;
#    $munge =~ s/^([^\(]*)(\s|\*)+\w+\(.*/\1/g;
    $munge = length($munge);
  }

  if($munge != 0) {
    $munge = ' ' x $munge;
    s/^([^\(]+\([^\(]+)\s*,\s*/\1,\n$munge/g;
    s/, ([^\)]+\);)/,\n$munge\1/g;
    s/\)\s*,\s*/),\n$munge/g;

    if(/.*\(\*\w+\)\(.*/) {
      @plines = split("\n", $_);
      $i = 0;
      foreach(@plines) {
	$psub = $_;
        if(length($psub) > $width+6 && /.*\(\*\w+\)\(.*/) {
            $munge2 = $psub;
#            $munge2 =~ s/([^\(]+\(\*).*/\1/g;
            $munge2 =~ s/([^\(]+\(\*\w+\)\().*/\1/g;
            $munge2 = length($munge2);
            $munge2 = ' ' x $munge2;
            $_ = $plines[$i];
            while(/, ([^\),]+)/) {
                s/, ([^\),]+)/,\n$munge2\1/g;
            }
            $plines[$i] = $_;
        }
        $i++;
      }
      $_ = join("\n", @plines);
      $_ = "$_\n";

    }
  }

  s/\n/<BR>\n/g;
  s/ /&nbsp;/g;
  s/([a-zA-Z_][a-zA-Z0-9_]*),/<\/B><I>\1<\/I><B>,/g;
  s/([a-zA-Z_][a-zA-Z0-9_]*)\)/<\/B><I>\1<\/I><B>\)/g;
  s/\(<\/B><I>void<\/I><B>\)/(void)/g;

  "<font color=\#333399><B>$_</B></font>";
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
    elsif(/^\s*$/) {
	if($#tmp >= 0) {
	    chop($tmp[$#tmp]);
	    $tmp[$#tmp] = "$tmp[$#tmp]/*\\n*/\n";
	}
    }
  }
  @rest = @tmp;
}

######################################################################

# Processes all typedefs

sub do_typedefs {
  local($name, $cur, $beg, $end, $i, @com, @td, $first);
  undef $beg;
  
  $cur = 0;
  $first = 0;
  while(1) {
    undef $beg;
    $beg = &first_loc_from('^typedef.*', $cur, *rest);
    if(!defined($beg)) {
      #print OUTFILE "$endindent";
      return;
    }
    if($rest[$beg] =~ /.*;.*/) {
      $end = $beg;
    }
    else {
      $end = &first_loc_from('^}\s*\w+\s*;\s*', $beg + 1, *rest);
    }

    $rest[$end] =~ s/\/\*\\n\*\///g;
    if($rest[$beg-1] =~ /.*\*\/\s*$/) {
      @td = @rest[$beg .. $end];
      $name = $rest[$end];
      if($beg == $end) {
        $name =~ s/.*\W(\w*)\s*;\s*/\1/g;
      }
      else {
        $name =~ s/^}\s*(\w+)\s*;\s*/\1/g;
      }
      $i = $beg - 1;
      while(!($rest[$i] =~ /^\/\*.*/) && $i >= 0) {
	$i--;
      }
      @com = @rest[$i .. $beg - 1];
      $com[$#com] =~ s/\/\*\\n\*\///g;
      $com[0] =~ s/.*\/\*(.*\n)/  \1/g;
      $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
      if($first == 0) {
        print OUTFILE "<h4>Type Declarations</h4>";
	print OUTFILE "The following types are defined in the header ";
	print OUTFILE "file <I>$inname</I>.<P>\n";
	$first = 1;
      }
      print OUTFILE "<TABLE WIDTH=100% CELLSPACING=0 CELLPADDING=0 "
	  . " BGCOLOR=$tablecolor>\n";
      print OUTFILE "<TR NOSAVE><TD NOSAVE><TT>";
      print OUTFILE "<b><A NAME=\"$name\">$name</A></b>";
      print OUTFILE "</TT></TD></TR>\n";
      @com = (@com, "\n");
      print OUTFILE "<TR><TD>\n";
      print OUTFILE "<TABLE WIDTH=100% BGCOLOR=#FFFFFF CELLSPACING=0 "
	  . "CELLPADDING=5><TR><TD>\n";
      print OUTFILE "<p><font color=\#993333><PRE><TT>";
      foreach (@td) {
        chop($_);
        s/\/\*\\n\*\//<br>/g;
	s/\b(auto|char|const|double|enum|extern|float|int|long|register|short|signed|static|struct|typedef|union|unsigned|void|volatile)\b/<b>$1<\/b>/g;
	s/^\/\*\\n\*\/$/$1<br>/g;
	s/\/\*/<font color=\#333399><i>\/\*/g;
	s/\*\//\*\/<\/i><\/font>/g;
        print OUTFILE "$_\n";
      }
      print OUTFILE "<\/TT><\/PRE><\/font><P>\n";
      @com = (@com, "\n");
      &do_text(*com);
      print OUTFILE "</TD></TR></TABLE>\n";
      print OUTFILE "</TD></TR></TABLE><p>\n";
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
      if(!defined($beg)) {
	#print OUTFILE "$endindent";
        return;
      }
      if(@rest[$beg] =~ /^\s*typedef/) {
	$cur = $beg + 1;
      }
      else {
        $okay = 1;
      }
    }
    $end = $beg;
    $rest[$beg-1] =~ s/\/\*\\n\*\///g;
    if($rest[$beg-1] =~ /.*\*\/\s*$/) {
      $proto = $rest[$beg];
      $i = $beg - 1;
      while(!($rest[$i] =~ /^\/\*.*/) && $i >= 0) {
	$i--;
      }
      @com = @rest[$i .. $beg - 1];
      $com[$#com] =~ s/\/\*\\n\*\///g;
      $com[0] =~ s/.*\/\*(.*\n)/  \1/g;
      $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
      if($first == 0) {
        print OUTFILE "<h4>Function Definitions</h4>";
	print OUTFILE "The following function prototypes are given ";
	print OUTFILE "in the header file <I>$inname</I>.<P>\n";
	$first = 1;
      }
      $proto = &convert_proto($proto);
      $proto =~ s/([a-zA-Z_][a-zA-Z0-9_]*)\(/<A NAME=\"\1\">\1<\/A>\(/;
      print OUTFILE "<TABLE WIDTH=100% CELLSPACING=0 CELLPADDING=0 "
	  . " BGCOLOR=$tablecolor>\n";
      print OUTFILE "<TR NOSAVE><TD NOSAVE><TT>";
      print OUTFILE "$proto";
      print OUTFILE "</TT></TD></TR>\n";
      @com = (@com, "\n");
      print OUTFILE "<TR><TD>\n";
      print OUTFILE "<TABLE WIDTH=100% BGCOLOR=#FFFFFF CELLSPACING=0 "
	  . "CELLPADDING=5><TR><TD>\n";
      &do_text(*com);
      print OUTFILE "</TD></TR></TABLE>\n";
      print OUTFILE "</TD></TR></TABLE><p>\n";
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
    if(!defined($beg)) {
      #print OUTFILE "$endindent";
      return;
    }
    $end = $beg;
    $rest[$beg-1] =~ s/\/\*\\n\*\///g;
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
        print OUTFILE "<h4>Macro Definitions</h4>";
	print OUTFILE "The following macros are defined ";
	print OUTFILE "in the header file <I>$inname</I>.<P>\n";
	$first = 1;
      }
      $macro = &convert_macro($macro);
      $macro =~ s/([a-zA-Z_][a-zA-Z0-9_]*)\(/<A NAME=\"\1\">\1<\/A>\(/;

      print OUTFILE "<TABLE WIDTH=100% CELLSPACING=0 CELLPADDING=0 "
	  . " BGCOLOR=$tablecolor>\n";
      print OUTFILE "<TR NOSAVE><TD NOSAVE><TT>";
      print OUTFILE "$macro";
      print OUTFILE "</TT></TD></TR>\n";
      @com = (@com, "\n");
      print OUTFILE "<TR><TD>\n";
      print OUTFILE "<TABLE WIDTH=100% BGCOLOR=#FFFFFF CELLSPACING=0 "
	  . "CELLPADDING=5><TR><TD>\n";
      &do_text(*com);
      print OUTFILE "</TD></TR></TABLE>\n";
      print OUTFILE "</TD></TR></TABLE><p>\n";
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
    if(!defined($beg)) {
      #print OUTFILE "$endindent";
      return;
    }
    $end = &first_loc_from('.*\*\/', $beg, *rest);
    @com = @rest[$beg .. $end];
    $com[0] =~ s/.*\/\*\s*h2man:include\s+(.*\n)/\1/g;
    $com[$#com] =~ s/\/\*\\n\*\///g;
    $com[$#com] =~ s/(.*)\*\/.*\n/\1\n/g;
    if($first == 0) {
      print OUTFILE "<h4>Miscellaneous Items</h4>";
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

  s/^(#define) (\w+\([^\)]*\)).*/\1 \2 .../g;
  s/([a-zA-Z_][a-zA-Z0-9_]*),/<\/B><\/TT><I>\1<\/I><TT><B>,/g;
  s/([a-zA-Z_][a-zA-Z0-9_]*)\)/<\/B><\/TT><I>\1<\/I><TT><B>\)/g;

  "<B>$_</B>";
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
  open(DEVNULL, "> /dev/null");
  
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
  $date = `date '+%d %h 19%y'`;
  chop($date);
  print OUTFILE <<EOF;
<HTML>
<HEAD>
<TITLE>NODElib: $inname</TITLE>
</HEAD>
<BODY TEXT="#000000" BGCOLOR="#FFFFFF" LINK=#993399
 VLINK="#666666">
<table width=100% bgcolor="#dddddd" border=0 cellspacing=0 cellpadding=2>
<tr><td>
<table width=100% bgcolor="#ffffff" border=0 cellspacing=0 cellpadding=5>
<tr align=center><td>
  <font size=+5>
  <b>NODElib</b> Documentation
  </font>
</tr></td>
<tr align=center><td><font size=-1> By Gary William Flake</font></td></tr>
</td></tr></table>
</td></tr></table>

<p>

<!-- INDEX -->
EOF

  if(! &do_section('NAME')) {
    print OUTFILE "$begsec" . "NAME" . "$endsec";
    print OUTFILE "$begindent";
    print OUTFILE "\n$inname";
  }
  print OUTFILE "$endindent";
  if(! &do_section('SYNOPSIS'))  {
    print OUTFILE "$begsec" . "SYNOPSIS" . "$endsec";
    print OUTFILE "$begindent";
  }
  @protos = grep(/^\w+(\s|\*)+\w+(\w|\s|\*)*\(.*\)\s*;/, @rest);
  @protos = grep(!/^ *typedef/, @protos);
  print OUTFILE "<p><TABLE BORDER=0 CELLSPACING=0 CELLPADDING=2 "
      . "align=center BGCOLOR=$tablecolor NOSAVE ><tr><td>\n";
  print OUTFILE "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=10 "
      . "BGCOLOR=#ffffff NOSAVE >\n";
  print OUTFILE "<TR NOSAVE>\n<TD NOSAVE><B>#include &lt;$inname&gt;</B><P><TT>\n";
  foreach(@protos) {
    if(/^\s*private/i) {
      next;
    }
    $tmp = &convert_proto($_);
    print OUTFILE "$tmp<BR>\n";
  }
  @macros = grep(/^#define \w+\([^\)]*\)/, @rest);
  foreach(@macros) {
    if(/private/i) {
      next;
    }
    $tmp = &convert_macro($_);
    print OUTFILE "$tmp<BR>\n";
  }
  print OUTFILE "</TT></TD></TR></TABLE></TD></TR></TABLE>\n";
  print OUTFILE "$endindent";

  # Description = typedefs, protos, macros, and globals.
  if(! &do_section('DESCRIPTION'))  {
      print OUTFILE "$begsec" . "DESCRIPTION" . "$endsec";
    print OUTFILE "$begindent";
  }
  &do_typedefs();
  &do_protos();
  &do_macros();
  &do_extras();
  print OUTFILE "$endindent";

  @sects = grep(/^\w+.*/, @header);
  foreach(@sects) {
    &do_section($_);
    print OUTFILE "$endindent";
  }

  print OUTFILE "\n</BODY></HTML>\n";

  close(INFILE);
  close(OUTFILE);
}

######################################################################

sub main_loop {
  local($iiii);			# perl bug?  if named $i it doesn't work.
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


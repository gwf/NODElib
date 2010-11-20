
######################################################################

sub main_loop {

  $catwords = "";

  foreach(@ARGV) {
      $fname = $_;
      open(INFILE, $fname) || die "crossref: can't open \"$_\" for read.\n";
      @words = grep(/.*\<A NAME=\"[a-zA-Z_][a-zA-Z0-9_]*\"\>.*/, <INFILE>);
      close(INFILE);
      foreach(@words) {
	  s/.*\<A NAME=\"([a-zA-Z_][a-zA-Z0-9_]*)\"\>.*/\1/o;
	  $word = $_;
	  chop($word);
	  $aref = "$fname\#$word";
	  $arefs{$word} = $aref;
	  if($catwords ne "") {
	      $catwords .= "|" . $word;
	  }
	  else {
	      $catwords = $word;
	  }
      }
  }
  foreach(@ARGV) {
      $fname = $_;
      open(INFILE, $fname) || die "crossref: can't open \"$_\" for read.\n";
      @lines = <INFILE>;
      close(INFILE);      
      open(OUTFILE, ">$fname") || die "crossref: can't open \"$_\" for write.\n";
      print OUTFILE "<!-- HTMLCROSSREF --\>";
      foreach(@lines) {
	  $line = $_;
	  $line =~ s/(^|[^\>\"\w])($catwords)([^\<\"\w])/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
	  $line =~ s/(^|\>)($catwords)(\<\/[^aA])/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
	  $line =~ s/(^|\>)($catwords)(\(\))/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
	  while($line =~ /([^\>\"\w])($catwords)([^\<\"\w])/) {
	    $line =~ s/(^|[^\>\"\w])($catwords)([^\<\"\w])/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
	    $line =~ s/(^|\>)($catwords)(\<\/[^aA])/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
	    $line =~ s/(^|\>)($catwords)(\(\))/"$1<A HREF=\"$arefs{$2}\">$2<\/A>$3"/egos;
  	  }
 	  print OUTFILE "$line";
      }
      close(OUTFILE);
  }
}

######################################################################

&main_loop();


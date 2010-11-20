
######################################################################

$pat = "\\<H1\\>";

sub main_loop {
  foreach(@ARGV) {
      $fname = $_;
      open(INFILE, $fname) || die "htmlindex: can't open \"$_\" for read.\n";
      @lines = <INFILE>;
      @sects = grep(/$pat([^\<]*)/, @lines);
      @sects = grep(s/.*$pat([^\<]*).*/$1/go, @sects);
      close(INFILE);      
      open(OUTFILE, ">$fname") || die "htmlindex: can't open \"$_\" for write.\n";
      foreach(@lines) {
	  $line = $_;
	  if(/\<\!-- INDEX --\>/) {
	      $first = 1;
	      foreach(@sects) {
		  $name = $_;
		  chop($name);
		  $aref = $name;
		  $aref =~ s/ /_/go;
		  if($first == 1) {
		      print OUTFILE "<A HREF=\"#$aref\">$name</A>\n";
		      $first = 0;
		  }
		  else {
		      print OUTFILE "<A HREF=\"#$aref\">$name</A>\n";
		  }
	      }
	      print OUTFILE "\n";
	  }
	  elsif(/$pat/) {
	      $name = $line;
	      $name =~ s/.*$pat([^\<]*).*/$1/go;
	      chop($name);
	      $aref = $name;
	      $aref =~ s/ /_/go;
	      $line =~ s/(.*$pat)([^\<]*)/$1<A NAME=\"$aref\">$name<\/A>/g;
	      print OUTFILE "$line";		  
	  }
	  else {
	      print OUTFILE "$line";
	  }
      }
      close(OUTFILE);
  }
}

######################################################################

&main_loop();



# Pretty print a manifest which consists of every normal file
# descended from the current directory.  (G.W. Flake)

$= = 1000;

format top =
  File Name                        Lines     Words   Chars      SYS V Check Sum
  -------------------------------  --------  ------  ---------  ---------------
.

format STDOUT =
  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  @<<<<<<<  @<<<<<  @<<<<<<<<  @<<<<<<<<
  $name,                           $lines,   $words, $chars,         $chsum
.

$fstar = "find . \! -name '*.[oa3]' -a \! -name '*.html' " .
         "-a \! -name '#*' -a \! -name '*~' -a  \! -perm 755 " .
         "-a \! -type d -a \! -type l ";

$fsbigtar = "find . \! -name '*.[oa]' -a \! -name '#*' " .
            "-a \! -name '*~' -a \! -perm 755 " .
            "-a \! -type d -a \! -type l ";


if($ARGV[0] eq "tar") {
  $findstr = "$fstar";
} 
else {
  $findstr = "$fsbigtar";
}    

open(INPUT, "$findstr -exec wc '{}' ';' |");

foreach (<INPUT>) {
  /\s*(\S*)\s*(\S*)\s*(\S*)\s*\.\/(\S*)/;
  $lines = $1;
  $words = $2;
  $chars = $3;
  $name  = $4;

  undef $/;
  open(SUMF, $name);
  $chsum = unpack("%16C*", <SUMF>);
  close(SUMF);
  $/ = "\n";

  $lsum += $lines;
  $wsum += $words;
  $csum += $chars;
  
  @output = (@output, "$name $lines $words $chars $chsum\n");
}

sort(@output);

foreach (@output) {
  chop;
  /(\S*)\s*(\S*)\s*(\S*)\s*(\S*)\s*(\S*)/;
  $name  = $1;
  $lines = $2;
  $words = $3;
  $chars = $4;
  $chsum = $5;
  write;
}

eval "
format STDOUT =
  ------------------------------------------------------------
  total                            @<<<<<<<  @<<<<<  @<<<<<<<<
                                   $lsum,    $wsum,  $csum
.
";
write;
exit 0;

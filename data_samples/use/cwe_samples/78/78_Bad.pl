
                  use CGI qw(:standard);$name = param('name');$nslookup = "/path/to/nslookup";print header;if (open($fh, "$nslookup $name|")) {while (<$fh>) {print escapeHTML($_);print "<br>\n";}close($fh);}
               
               
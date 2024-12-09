
                  sub GetUntrustedInput {return($ARGV[0]);}
                     sub encode {my($str) = @_;$str =~ s/\&/\&amp;/gs;$str =~ s/\"/\&quot;/gs;$str =~ s/\'/\&apos;/gs;$str =~ s/\</\&lt;/gs;$str =~ s/\>/\&gt;/gs;return($str);}
                     sub doit {my $uname = encode(GetUntrustedInput("username"));print "<b>Welcome, $uname!</b><p>\n";system("cd /home/$uname; /bin/ls -l");
                     }
               
               
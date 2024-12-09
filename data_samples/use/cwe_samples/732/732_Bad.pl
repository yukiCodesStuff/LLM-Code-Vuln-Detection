
                  $fileName = "secretFile.out";
                     if (-e $fileName) {chmod 0777, $fileName;}
                     my $outFH;if (! open($outFH, ">>$fileName")) {ExitError("Couldn't append to $fileName: $!");}my $dateString = FormatCurrentTime();my $status = IsHostAlive("cwe.mitre.org");print $outFH "$dateString cwe status: $status!\n";close($outFH);
               
               
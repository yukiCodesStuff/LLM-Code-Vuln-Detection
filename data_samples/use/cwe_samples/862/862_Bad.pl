
                  sub DisplayPrivateMessage {my($id) = @_;my $Message = LookupMessageObject($id);print "From: " . encodeHTML($Message->{from}) . "<br>\n";print "Subject: " . encodeHTML($Message->{subject}) . "\n";print "<hr>\n";print "Body: " . encodeHTML($Message->{body}) . "\n";}
                     my $q = new CGI;
                     # For purposes of this example, assume that CWE-309 and
                     
                     
                     
                     # CWE-523 do not apply.
                     if (! AuthenticateUser($q->param('username'), $q->param('password'))) {ExitError("invalid username or password");}
                     my $id = $q->param('id');DisplayPrivateMessage($id);
               
               
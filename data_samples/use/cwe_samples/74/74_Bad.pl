
                  
					my $arg = GetArgument("filename");
					do_listing($arg);
					

					sub do_listing {
					
						my($fname) = @_;
						if (! validate_name($fname)) {
							
							print "Error: name is not well-formed!\n";
							return;
							
						}
						# build command
						my $cmd = "/bin/ls -l $fname";
						system($cmd);
					
					}
					
					sub validate_name {
					
						my($name) = @_;
						if ($name =~ /^[\w\-]+$/) {
						
							return(1);
						
						}
						else {
						
							return(0);
						
						}
					
					}
				  
               

               
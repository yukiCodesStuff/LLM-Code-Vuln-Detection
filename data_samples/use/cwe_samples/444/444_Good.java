
                  protected void processRequest(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
				  
                    
                      
                      // Set up response writer object
                      ...try {
					  
						
                          // check for multiple content length headers
                          Enumeration contentLengthHeaders = request.getHeaders("Content-Length");
						  int count = 0;
						  while (contentLengthHeaders.hasMoreElements()) {
						  count++;
						  }
						  if (count > 1) {
						  
							// output error response
							}
							else {
							
							  
								// process request
							  
							  }
							  
                        
                    } catch (Exception ex) {...}
					}
				  
               
            
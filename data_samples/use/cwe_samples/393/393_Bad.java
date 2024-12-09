
                  try {
                        
                           
                           // something that might throw IOException
                           ...
                     } catch (IOException ioe) {response.sendError(SC_NOT_FOUND);}
               
            
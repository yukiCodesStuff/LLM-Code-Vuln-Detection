
                  String val = request.getParameter("val");try {
                        
                           int value = Integer.parseInt(val);
                     }catch (NumberFormatException) {log.info("Failed to parse val = " + val);}...
               
               
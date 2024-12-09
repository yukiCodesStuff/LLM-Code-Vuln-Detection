
                  public class TrustedClass {
                        ...
                           @Overridepublic boolean equals(Object obj) {
                              boolean isEquals = false;
                                 
                                 // first check to see if the object is of the same class
                                 if (obj.getClass().getName().equals(this.getClass().getName())) {
                                    
                                       
                                       // then compare object fields
                                       ...if (...) {isEquals = true;}
                                 }
                                 return isEquals;
                           }
                           ...
                     }
               
               
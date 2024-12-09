
                  public class OuterClass {
                     
                        
                           
                           // private member variables of OuterClass
                           private String memberOne;private static String memberTwo;
                           
                           // constructor of OuterClass
                           public OuterClass(String varOne, String varTwo) {
                           this.memberOne = varOne;this.memberTwo = varTwo;}
                           
                           // InnerClass is a static inner class of OuterClass
                           private static class InnerClass {
                              
                                 private String innerMemberOne;
                                 public InnerClass(String innerVarOne) {this.innerMemberOne = innerVarOne;}public String concat(String separator) {
                                    
                                       // InnerClass only has access to static member variables of OuterClass
                                       return memberTwo + separator + this.innerMemberOne;
                                 }
                           }
                     }
               
               
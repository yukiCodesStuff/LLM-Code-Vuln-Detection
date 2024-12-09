
                  public class OuterClass {
                        // private member variables of OuterClass
                        private String memberOne;private String memberTwo;
                        
                        // constructor of OuterClass
                        public OuterClass(String varOne, String varTwo) {this.memberOne = varOne;this.memberTwo = varTwo;}
                        
                        // InnerClass is a member inner class of OuterClass
                        private class InnerClass {private String innerMemberOne;
                           public InnerClass(String innerVarOne) {this.innerMemberOne = innerVarOne;}
                           public String concat(String separator) {
                              // InnerClass has access to private member variables of OuterClass
                              System.out.println("Value of memberOne is: " + memberOne);return OuterClass.this.memberTwo + separator + this.innerMemberOne;}}}
               
               
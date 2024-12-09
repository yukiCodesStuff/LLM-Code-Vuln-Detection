
                  public class Kibitzer {
                        public Object clone() throws CloneNotSupportedException {
                              
                                 Object returnMe = new Kibitzer();...
                           }
                     }
                     public class FancyKibitzer extends Kibitzer{
                        public Object clone() throws CloneNotSupportedException {
                              
                                 Object returnMe = super.clone();...
                           }
                     }
               
            
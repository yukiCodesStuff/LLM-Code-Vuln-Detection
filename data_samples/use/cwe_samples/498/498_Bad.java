
                  public class CloneClient {
                        public CloneClient() //throwsjava.lang.CloneNotSupportedException {
                              
                                 Teacher t1 = new Teacher("guddu","22,nagar road");//...// Do some stuff to remove the teacher.Teacher t2 = (Teacher)t1.clone();System.out.println(t2.name);
                           }public static void main(String args[]) {
                              
                                 new CloneClient();
                           }
                     }class Teacher implements Cloneable {
                        
                           public Object clone() {
                              
                                 try {return super.clone();}catch (java.lang.CloneNotSupportedException e) {
                                    
                                       throw new RuntimeException(e.toString());
                                 }
                           }public String name;public String clas;public Teacher(String name,String clas) {
                              
                                 this.name = name;this.clas = clas;
                           }
                     }
               
               

                  public classRace {
                        static int foo = 0;public static void main() {
                              
                                 new Threader().start();foo = 1;
                           }public static class Threader extends Thread {
                              
                                 public void run() {System.out.println(foo);}
                           }
                     }
               
            
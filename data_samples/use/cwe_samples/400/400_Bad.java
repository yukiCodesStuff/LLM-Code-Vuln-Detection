
                  class Worker implements Executor {
                        ...public void execute(Runnable r) {
                              
                                 try {...}catch (InterruptedException ie) {
                                    
                                       
                                       // postpone response
                                       Thread.currentThread().interrupt();
                                 }
                           }
                           public Worker(Channel ch, int nworkers) {...}
                           protected void activate() {
                              
                                 Runnable loop = new Runnable() {
                                    
                                       public void run() {
                                          
                                             try {for (;;) {Runnable r = ...;r.run();}}catch (InterruptedException ie) {...}
                                       }
                                 };new Thread(loop).start();
                           }
                     }
               
               
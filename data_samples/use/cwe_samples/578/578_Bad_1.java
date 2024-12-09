
                  @Statelesspublic class LoaderSessionBean implements LoaderSessionRemote {
                     
                        public LoaderSessionBean() {try {ClassLoader loader = new CustomClassLoader();Class c = loader.loadClass("someClass");Object obj = c.newInstance();/* perform some task that uses the new class instance member variables or functions */...} catch (Exception ex) {...}}
                           public class CustomClassLoader extends ClassLoader {
                           }
                     }
               
            
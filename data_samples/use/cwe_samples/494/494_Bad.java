
                  URL[] classURLs= new URL[]{new URL("file:subdir/")};URLClassLoader loader = new URLClassLoader(classURLs);Class loadedClass = Class.forName("loadMe", true, loader);
               
               
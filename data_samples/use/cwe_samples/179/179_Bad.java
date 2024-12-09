
                  String path = getInputPath();if (path.startsWith("/safe_dir/")){File f = new File(path);return f.getCanonicalPath();}
               
               
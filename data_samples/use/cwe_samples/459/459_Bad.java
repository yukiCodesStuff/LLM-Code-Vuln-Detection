
                  try {InputStream is = new FileInputStream(path);byte b[] = new byte[is.available()];is.read(b);is.close();} catch (Throwable t) {log.error("Something bad happened: " + t.getMessage());}
               
            
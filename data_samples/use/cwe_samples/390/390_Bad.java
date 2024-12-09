
                  public String readFile(String filename) {
                        String retString = null;try {
                              // initialize File and FileReader objectsFile file = new File(filename);FileReader fr = new FileReader(file);
                                 // initialize character bufferlong fLen = file.length();char[] cBuf = new char[(int) fLen];
                                 // read data from fileint iRead = fr.read(cBuf, 0, (int) fLen);
                                 // close filefr.close();
                                 retString = new String(cBuf);
                              
                           } catch (Exception ex) {/* do nothing, but catch so it'll compile... */}return retString;
                     }
               
               
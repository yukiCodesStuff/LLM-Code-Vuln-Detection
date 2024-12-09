
                  public String readFile(String filename) throws FileNotFoundException, IOException, Exception {
                        String retString = null;try {
                              // initialize File and FileReader objectsFile file = new File(filename);FileReader fr = new FileReader(file);
                                 // initialize character bufferlong fLen = file.length();char [] cBuf = new char[(int) fLen];
                                 // read data from fileint iRead = fr.read(cBuf, 0, (int) fLen);
                                 // close filefr.close();
                                 retString = new String(cBuf);
                              
                           } catch (FileNotFoundException ex) {System.err.println ("Error: FileNotFoundException opening the input file: " + filename );System.err.println ("" + ex.getMessage() );throw new FileNotFoundException(ex.getMessage());} catch (IOException ex) {System.err.println("Error: IOException reading the input file.\n" + ex.getMessage() );throw new IOException(ex);} catch (Exception ex) {System.err.println("Error: Exception reading the input file.\n" + ex.getMessage() );throw new Exception(ex);}return retString;
                     }
               
            

                  private File readFile = null;
                     public void setInputFile(String inputFile) {
                        
                           
                           // create readFile File object from string containing name of file
                           
                        
                     }
                     public void readFromFile() {
                        try {
                              if (readFile == null) {System.err.println("Input file has not been set, call setInputFile method before calling openInputFile");throw NullPointerException;}
                                 reader = new FileReader(readFile);
                                 
                                 // read input file
                                 
                              
                           } catch (FileNotFoundException ex) {...}catch (NullPointerException ex) {...}
                     }
               
            
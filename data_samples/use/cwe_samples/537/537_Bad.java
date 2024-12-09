
                  public class InputFileRead {
                     
                        private File readFile = null;private FileReader reader = null;private String inputFilePath = null;private final String DEFAULT_FILE_PATH = "c:\\somedirectory\\";
                           public InputFileRead() {inputFilePath = DEFAULT_FILE_PATH;}
                           public void setInputFile(String inputFile) {
                              
                                 
                                 /* Assume appropriate validation / encoding is used and privileges / permissions are preserved */
                                 
                              
                           }
                           public void readInputFile() {
                              try {reader = new FileReader(readFile);...} catch (RuntimeException rex) {System.err.println("Error: Cannot open input file in the directory " + inputFilePath);System.err.println("Input file has not been set, call setInputFile method before calling readInputFile");
                                 
                                 } catch (FileNotFoundException ex) {...}
                              
                           }
                     }
               
               
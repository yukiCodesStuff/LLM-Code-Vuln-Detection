
                  try {File temp = File.createTempFile("pattern", ".suffix");temp.deleteOnExit();BufferedWriter out = new BufferedWriter(new FileWriter(temp));out.write("aString");out.close();}catch (IOException e) {}
               
               
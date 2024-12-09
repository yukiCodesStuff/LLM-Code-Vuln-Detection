
                private void processFile(string fName){BufferReader fil = new BufferReader(new FileReader(fName));String line;while ((line = fil.ReadLine()) != null){processLine(line);}fil.Close();}
               
            
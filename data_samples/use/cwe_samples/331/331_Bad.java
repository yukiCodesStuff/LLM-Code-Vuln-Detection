
                  String GenerateReceiptURL(String baseUrl) {Random ranGen = new Random();ranGen.setSeed((new Date()).getTime());return(baseUrl + ranGen.nextInt(400000000) + ".html");}
               
               
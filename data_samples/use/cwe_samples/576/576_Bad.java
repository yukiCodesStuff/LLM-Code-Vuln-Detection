
                  @Statelesspublic class InterestRateBean implements InterestRateRemote {
                     
                        private Document interestRateXMLDocument = null;private File interestRateFile = null;
                           public InterestRateBean() {
                              try {
                                    
                                       
                                       /* get XML document from the local filesystem */
                                       interestRateFile = new File(Constants.INTEREST_RATE_FILE);
                                       if (interestRateFile.exists()){DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();DocumentBuilder db = dbf.newDocumentBuilder();interestRateXMLDocument = db.parse(interestRateFile);}
                                 } catch (IOException ex) {...}
                           }
                           public BigDecimal getInterestRate(Integer points) {return getInterestRateFromXML(points);}
                           
                           /* member function to retrieve interest rate from XML document on the local file system */
                           
                           private BigDecimal getInterestRateFromXML(Integer points) {...}
                     }
               
               
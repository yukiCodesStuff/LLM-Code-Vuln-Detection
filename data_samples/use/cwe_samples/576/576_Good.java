
                  @Statelesspublic class InterestRateBean implements InterestRateRemote {
                        
                           public InterestRateBean() {}
                           public BigDecimal getInterestRate(Integer points) {return getInterestRateFromXMLParser(points);}
                           
                           /* member function to retrieve interest rate from XML document using an XML parser API */
                           
                           private BigDecimal getInterestRateFromXMLParser(Integer points) {...}
                     }
               
            
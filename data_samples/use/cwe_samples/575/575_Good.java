
                  @Statelesspublic class ConverterSessionBean implements ConverterSessionRemoteInterface {
                        
                           
                           /* conversion rate on US dollars to Yen */
                           private BigDecimal yenRate = new BigDecimal("115.3100");
                           public ConverterSessionBean() {}
                           
                           /* remote method to convert US dollars to Yen */
                           
                           public BigDecimal dollarToYen(BigDecimal dollars) {BigDecimal result = dollars.multiply(yenRate);return result.setScale(2, BigDecimal.ROUND_DOWN);}
                     }
               
               
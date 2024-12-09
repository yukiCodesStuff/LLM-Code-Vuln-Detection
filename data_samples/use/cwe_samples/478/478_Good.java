
                  public static final String INTEREST_RATE_AT_ZERO_POINTS = "5.00";public static final String INTEREST_RATE_AT_ONE_POINTS = "4.75";public static final String INTEREST_RATE_AT_TWO_POINTS = "4.50";...public BigDecimal getInterestRate(int points) {
                        BigDecimal result = new BigDecimal(INTEREST_RATE_AT_ZERO_POINTS);
                           switch (points) {
                              case 0:result = new BigDecimal(INTEREST_RATE_AT_ZERO_POINTS);break;
                                 case 1:result = new BigDecimal(INTEREST_RATE_AT_ONE_POINTS);break;
                                 case 2:result = new BigDecimal(INTEREST_RATE_AT_TWO_POINTS);break;
                                 default:System.err.println("Invalid value for points, must be 0, 1 or 2");System.err.println("Returning null value for interest rate");result = null;
                              
                           }
                           return result;
                     }
               
            
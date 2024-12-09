
                  @Statelesspublic class ConverterSessionBean extends Component implements KeyListener, ConverterSessionRemote {
                        
                           
                           /* member variables for receiving keyboard input using AWT API */
                           
                           ...private StringBuffer enteredText = new StringBuffer();
                           
                           /* conversion rate on US dollars to Yen */
                           
                           private BigDecimal yenRate = new BigDecimal("115.3100");
                           public ConverterSessionBean() {
                              super();
                                 /* method calls for setting up AWT Component for receiving keyboard input */
                                 
                                 ...addKeyListener(this);
                           }
                           public BigDecimal dollarToYen(BigDecimal dollars) {BigDecimal result = dollars.multiply(yenRate);return result.setScale(2, BigDecimal.ROUND_DOWN);}
                           
                           /* member functions for implementing AWT KeyListener interface */
                           
                           public void keyTyped(KeyEvent event) {...}
                           public void keyPressed(KeyEvent e) {}
                           public void keyReleased(KeyEvent e) {}
                           
                           /* member functions for receiving keyboard input and displaying output */
                           
                           public void paint(Graphics g) {...}
                           ...
                     }
               
               
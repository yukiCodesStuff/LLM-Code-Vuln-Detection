
                  public Cipher getRSACipher() {Cipher rsa = null;try {rsa = javax.crypto.Cipher.getInstance("RSA/NONE/NoPadding");}catch (java.security.NoSuchAlgorithmException e) {log("this should never happen", e);}catch (javax.crypto.NoSuchPaddingException e) {log("this should never happen", e);}return rsa;}
               
               
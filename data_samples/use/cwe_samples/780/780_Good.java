
                  public Cipher getRSACipher() {Cipher rsa = null;try {rsa = javax.crypto.Cipher.getInstance("RSA/ECB/OAEPWithMD5AndMGF1Padding");}catch (java.security.NoSuchAlgorithmException e) {log("this should never happen", e);}catch (javax.crypto.NoSuchPaddingException e) {log("this should never happen", e);}return rsa;}
               
            
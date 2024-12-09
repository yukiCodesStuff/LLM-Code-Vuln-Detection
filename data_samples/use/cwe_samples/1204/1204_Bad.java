
			      public class SymmetricCipherTest {
                              public static void main() {
                              
                              byte[] text ="Secret".getBytes();byte[] iv ={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};KeyGenerator kg = KeyGenerator.getInstance("DES");kg.init(56);SecretKey key = kg.generateKey();Cipher cipher = Cipher.getInstance("DES/CBC/PKCS5Padding");IvParameterSpec ips = new IvParameterSpec(iv);cipher.init(Cipher.ENCRYPT_MODE, key, ips);return cipher.doFinal(inpBytes);
                              }
			      }
			    
			    
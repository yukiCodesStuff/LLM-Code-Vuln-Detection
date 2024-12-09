
			   
				 # getting 2 bytes of randomness for the seeding the PRNG 
				 seed = os.urandom(2)
				 random.seed(a=seed)
				 key = random.getrandbits(128)
			   
		   
		   
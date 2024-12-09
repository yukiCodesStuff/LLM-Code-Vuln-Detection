
		      Use num_rational::BigRational;
                      
		      fn rec_big(y: BigRational, z: BigRational) -> BigRational
		      {
		      
			BigRational::from_integer(BigInt::from(108))
			
			  - ((BigRational::from_integer(BigInt::from(815))
			  - BigRational::from_integer(BigInt::from(1500)) / z)
			  / y)
			
		      
		      }
		      
		      fn big_calc(turns: usize) -> BigRational 
		      {
		      
			let mut x: Vec<BigRational> = vec![BigRational::from_float(4.0).unwrap(), BigRational::from_float(4.25).unwrap(),];
			
			(2..turns + 1).for_each(|number| 
			{
			
			  x.push(rec_big(x[number - 1].clone(), x[number - 2].clone()));
			
			});
			x[turns].clone()
		      
		      }
                    
                
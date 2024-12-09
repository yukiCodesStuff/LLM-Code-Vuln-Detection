
		      fn rec_float(y: f64, z: f64) -> f64 
		      {
		      
			108.0 - ((815.0 - 1500.0 / z) / y);
		      
		      }
		      
		      fn float_calc(turns: usize) -> f64 
		      {
		      
			let mut x: Vec<f64> = vec![4.0, 4.25];
			(2..turns + 1).for_each(|number| 
			{
			
			  x.push(rec_float(x[number - 1], x[number - 2]));
			
			});
			
			x[turns]
		      
		      }
                    
                    
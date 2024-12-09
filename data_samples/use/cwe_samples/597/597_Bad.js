
                  <p id="ieq3s1" type="text">(i === s1) is FALSE</p>
                  <p id="s4eq3i" type="text">(s4 === i) is FALSE</p>
                  <p id="s4eq3s1" type="text">(s4 === s1) is FALSE</p>
                  
                  var i = 65;
                  var s1 = '65';
                  var s4 = new String('65');
                  
                  if (i === s1)
                  {
                  document.getElementById("ieq3s1").innerHTML = "(i === s1) is TRUE";
                  }
                  
                  if (s4 === i)
                  {
                  document.getElementById("s4eq3i").innerHTML = "(s4 === i) is TRUE";
                  }
                  
                  if (s4 === s1)
                  {
                  document.getElementById("s4eq3s1").innerHTML = "(s4 === s1) is TRUE";
                  }
               
               
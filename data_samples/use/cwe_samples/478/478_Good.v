
                case (user_input)
                3'h0:
                3'h1:
                3'h2:
                3'h3: state = 2'h3;
                3'h4: state = 2'h2;
                3'h5: state = 2'h1;
                default: state = 2'h0;
              
                endcase
              
            
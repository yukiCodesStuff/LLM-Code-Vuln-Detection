
                  int aN, Bn;switch (ctl) {
                        case -1:aN = 0;bN = 0;break;
                           case 0:aN = i;bN = -i;break;
                           case 1:aN = i + NEXT_SZ;bN = i - NEXT_SZ;break;
                           default:aN = -1;aN = -1;break;
                        
                     }repaint(aN, bN);
               
               

                  ...#define MAX_DIM 100...
                     /* board dimensions */
                     
                     int m,n, error;board_square_t *board;printf("Please specify the board height: \n");error = scanf("%d", &m);if ( EOF == error ){die("No integer passed: Die evil hacker!\n");}printf("Please specify the board width: \n");error = scanf("%d", &n);if ( EOF == error ){die("No integer passed: Die evil hacker!\n");}if ( m > MAX_DIM || n > MAX_DIM ) {die("Value too large: Die evil hacker!\n");}board = (board_square_t*) malloc( m * n * sizeof(board_square_t));...
               
               
					putenv("TEST_PHP_EXECUTABLE=$php");
					$environment['TEST_PHP_EXECUTABLE'] = $php;
					break;
				case 'q':
					putenv('NO_INTERACTION=1');
					break;
				//case 'r'

    -p <php>    Specify PHP executable to run.

    -q          Quiet, no user interaction (same as environment NO_INTERACTION).

    -s <file>   Write output to <file>.

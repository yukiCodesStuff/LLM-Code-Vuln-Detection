} catch (Exception $e) {
	/* catch exceptions so that we can show the relative error */
	echo "Exception! at line ", $e->getLine(), "\n";
	var_dump($e->getMessage());
}
if(isset($filename)) {
	@unlink($filename);
}
?>
--EXPECT--
Preparing test file and array for CopyFrom tests
Testing pgsqlCopyFromArray() with default parameters
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  string(13) "test insert 0"
  [1]=>
  string(13) "test insert 0"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  string(13) "test insert 1"
  [1]=>
  string(13) "test insert 1"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  string(13) "test insert 2"
  [1]=>
  string(13) "test insert 2"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromArray() with different field separator and not null indicator
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  string(13) "test insert 0"
  [1]=>
  string(13) "test insert 0"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  string(13) "test insert 1"
  [1]=>
  string(13) "test insert 1"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  string(13) "test insert 2"
  [1]=>
  string(13) "test insert 2"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromArray() with only selected fields
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromArray() with error
bool(false)
Testing pgsqlCopyFromFile() with default parameters
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  string(13) "test insert 0"
  [1]=>
  string(13) "test insert 0"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  string(13) "test insert 1"
  [1]=>
  string(13) "test insert 1"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  string(13) "test insert 2"
  [1]=>
  string(13) "test insert 2"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromFile() with different field separator and not null indicator
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  string(13) "test insert 0"
  [1]=>
  string(13) "test insert 0"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  string(13) "test insert 1"
  [1]=>
  string(13) "test insert 1"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  string(13) "test insert 2"
  [1]=>
  string(13) "test insert 2"
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromFile() with only selected fields
bool(true)
array(6) {
  ["a"]=>
  int(0)
  [0]=>
  int(0)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(1)
  [0]=>
  int(1)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromFile() with error
bool(false)
array(6) {
  ["a"]=>
  int(2)
  [0]=>
  int(2)
  ["b"]=>
  NULL
  [1]=>
  NULL
  ["c"]=>
  NULL
  [2]=>
  NULL
}
Testing pgsqlCopyFromFile() with error
bool(false)
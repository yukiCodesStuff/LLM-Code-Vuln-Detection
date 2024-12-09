	echo "Exception! at line ", $e->getLine(), "\n";
	var_dump($e->getMessage());
}
if(isset($filename)) {
	@unlink($filename);
}
?>
--EXPECT--
Preparing test file and array for CopyFrom tests
  NULL
}
Testing pgsqlCopyFromFile() with error
bool(false)
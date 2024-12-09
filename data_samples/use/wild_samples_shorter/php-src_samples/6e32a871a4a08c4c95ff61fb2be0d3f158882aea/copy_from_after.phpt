	echo "Exception! at line ", $e->getLine(), "\n";
	var_dump($e->getMessage());
}

// Clean up 
foreach (array($filename, $filenameWithDifferentNullValues, $filenameWithDifferentNullValuesAndSelectedFields) as $f) {
	@unlink($f);
}
?>
--EXPECT--
Preparing test file and array for CopyFrom tests
  NULL
}
Testing pgsqlCopyFromFile() with error
bool(false)
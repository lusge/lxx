--TEST--
lxx_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('lxx')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = lxx_test1();

var_dump($ret);
?>
--EXPECT--
The extension lxx is loaded and working!
NULL

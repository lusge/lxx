--TEST--
Check if lxx is loaded
--SKIPIF--
<?php
if (!extension_loaded('lxx')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "lxx" is available';
?>
--EXPECT--
The extension "lxx" is available

<?php
$output = null;
$retval = null;
exec('./C++/str8ts', $output, $retval);
if ($retval === 0) {
	echo $output[0];
} else {
	echo "Error";
}
?>

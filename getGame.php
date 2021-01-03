<?php
$output = null;
$retval = null;
$difficulty = 3;
if (isset($_GET['difficulty']) and is_numeric($_GET['difficulty']) and $_GET['difficulty'] >= 0 and $_GET['difficulty'] <= 5) {
	$difficulty = $_GET['difficulty'];
}
exec('./C++/str8ts '.$difficulty, $output, $retval);
if ($retval === 0) {
	echo $output[0];
} else {
	echo "Error";
}
?>

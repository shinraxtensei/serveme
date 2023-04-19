<?php
$body = "hello from PHP POST"; 
// $body = $body . " " . $_POST['name'];
$len = strlen($body);
header ("content-length: $len\r\n\r\n");
//string ing php
echo $body;
?>

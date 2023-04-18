<?php
$body = "hello from PHP"; 
$len = strlen($body);
header ("content-length: $len");

//string ing php
echo $body;
?>

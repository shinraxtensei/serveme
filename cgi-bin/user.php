<?php
$body = "hello from PHP GET"; 
$len = strlen($body);
header ("content-length: $len");

//string ing php
echo $body;
?>

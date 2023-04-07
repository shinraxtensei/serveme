#!/usr/bin/php-cgi
<?php

// Set the content type of the response
header("Content-type: text/plain");

// Get the values of the "name" and "age" parameters from the request
$name = $_REQUEST["name"];
$age = $_REQUEST["age"];

// Print out the values of the parameters
echo "Name: $name\n";
echo "Age: $age\n";

?>
#!/usr/bin/php-cgi
<?php

// Set the content type of the response
header("HTTP/1.1 200 OK");
header("Server: Garson/0.1.5 (1337)");
header("Content-type: text/html");

// Get the values of the "name" and "age" parameters from the request
$name = isset($_REQUEST["name"]) ? $_REQUEST["name"] : '';
$age = isset($_REQUEST["age"]) ? $_REQUEST["age"] : '';
// Print out the values of the parameters
echo "Name: $name\n";
echo "Age: $age\n";

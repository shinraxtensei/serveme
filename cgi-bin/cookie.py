#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb , html
# cgitb.enable()

# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
# Get data from fields
name = form.getvalue("name")
age  = form.getvalue("age")
if (form.getvalue(color))
	color = form.getvalue("color")
else
	color = "white"

print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
if (form.getvalue(color))
	print ("Set-cookie: background_color=%s", color)
print ("Content-type:text/html\n\r\n\r")


print ("<html>")
print ("<head>")
print ("style")
print ("body {")
print ("background-color: %s", color)
print ("}")
print ("</style>")
print ("</head>")
print ("<body>")
print ("<h1>Welcome to my page</h1>")
print ("<form method="GET" action="localhost:6969/cookie.php">")
print ("<label for="color">Background Color:</label>")
print ("<input type="text" name="color" id="color">")
print ("<input type="submit" value="Set Color">")
print ("</form>")
print ("</body>")
print ("</html>")
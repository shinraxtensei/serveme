#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb , html
# cgitb.enable()

# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
# Get data from fields

if form.getvalue('color'):
	color = form.getvalue('color')
elif form.getvalue('HTTP_COOKIE'):
	color = form.getvalue('HTTP_COOKIE')
else:
	color = "white"

print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
if form.getvalue('color'):
	print ("Set-cookie: background_color=%s" % (color))
print ("Content-type:text/html\n\r\n\r")

print ("<html>")
print ("<head>")
print ("<body style=\"background-color: %s;\">" % (color))
print ("</head>")
print ("<body>")
print ("<h1>Welcome to my page</h1>")
print ("<form method=\"GET\" action=\"cookie.py\">")
print ("<label for=\"color\">Background Color:</label>")
print ("<input type=\"text\" name=\"color\" id=\"color\">")
print ("<input type=\"submit\" value=\"Set Color\">")
print ("</form>")
print ("</body>")
print ("</html>")
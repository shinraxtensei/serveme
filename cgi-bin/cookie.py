#!/usr/bin/python3

# Import modules for CGI handling 
import cgi, cgitb , html , os
# cgitb.enable()
# parse the HTTP cookies sent by the client
cookie_string = os.environ.get('HTTP_COOKIE')
if cookie_string:
	index = cookie_string.find("=")
	color = cookie_string[index+1:]
else:
	color = "white"
# access the value of a cookie named "my_cookie"
# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
# Get data from fields
# if form.getvalue('HTTP_COOKIE'):
# 	color = cookie_string
if form.getvalue('color') and form.getvalue('color') != color:
	color = form.getvalue('color')

print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Content-type:text/html\n\r\n\r")

print ("<html>")
print ("<head>")
print ("<body style=\"background-color: %s;\">" % (color))
print ("</head>")
print ("<body>")
print ("<h1>Welcome to my page</h1>")
print ("<p>%s</p>" % (cookie_string))
print ("<form method=\"GET\" action=\"cookie.py\">")
print ("<label for=\"color\">Background Color:</label>")
print ("<input type=\"text\" name=\"color\" id=\"color\">")
print ("<input type=\"submit\" value=\"Set Color\">")
print ("</form>")
print ("</body>")
print ("</html>")
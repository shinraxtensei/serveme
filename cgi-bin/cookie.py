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
color = form.getvalue("color")

print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Set-cookie: background_color=%s", color)
print ("Content-type:text/html\n\r\n\r")
print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello %s %s</h2>" % (name, age))
print ("</body>")
print ("</html>")
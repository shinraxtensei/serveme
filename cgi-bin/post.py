#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage()
cgitb.enable()

# Get data from fields
if 'first_name' in form:
    name = form.getvalue('name')
else:
    name = 'Unknown'
if 'last_name' in form:
    age  = form.getvalue('age')
else:
    age = 'Unknown'

print("Content-type:text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("</head>")
print("<body>")
print("<h2>Hello %s %s</h2>" % (first_name, last_name))
print("</body>")
print("</html>")
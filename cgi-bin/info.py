#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 
cgitb.enable()

# Create instance of FieldStorage 
form = cgi.FieldStorage()

# Get data from fields
# Get data from fields
if 'first_name' in form:
    first_name = form.getvalue('first_name')
else:
    first_name = 'Unknown'
if 'last_name' in form:
    last_name  = form.getvalue('last_name')
else:
    last_name = 'Unknown'

print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Content-type:text/html\n\r\n\r")
print ("<html>")
print ("<head>")
print ("<title>Hello - Second CGI Program</title>")
print ("</head>")
print ("<body>")
print ("<h2>Hello %s %s</h2>" % (first_name, last_name))
print ("</body>")
print ("</html>")
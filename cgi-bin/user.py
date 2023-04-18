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

body_string = "<html><head><title>Hello - Second CGI Program</title></head><body><h2>Hello %s %s</h2></body></html>" % (name, age)
print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Content-type:text/html")
print ("Content-Length: %d" % len(body_string))
print ("connection: close\n\r\n\r")
print (body_string)
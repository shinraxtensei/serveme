#!/usr/bin/python3
# Import modules for CGI handling
import cgi, cgitb
# Create instance of FieldStorage
form = cgi.FieldStorage()
# Get data from fields
if form.getvalue('textcontent'):
   text_content = form.getvalue('textcontent')
else:
   text_content = "Not entered"

body_string = "<html><head><title>Hello - Second CGI Program</title></head><body><h2> %s </h2></body></html>" % (text_content)
print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Content-type:text/html")
print ("Content-Length: %d" % len(body_string))
print ("connection: close\n\r\n\r")
print (body_string)
# while True:
#    print ("mok")
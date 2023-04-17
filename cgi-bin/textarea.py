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
print ("HTTP/1.1 200 OK")
print ("server: Garson/0.1.5 (1337)")
print ("Content-type:text/html")
print ("connection: close\n\r\n\r")
print ("<html>")
print ("<head>")
print ("<title>Text Area - Fifth CGI Program</title>")
print ("</head>")
print ("<body>")
# print(len(text_content))
print ("%s" % text_content)
print ("</body>")
print ("</html>\r\n\r\n")

while True:
      pass

#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
name = form.getvalue('name')
nick  = form.getvalue('nick')

print "Content-type:text/html\r\n"
print "Server: Servme v0.1 software\r\n\r\n"
print "<html>"
print "<head>"
print "<title>Hello - Second CGI Program</title>"
print "</head>"
print "<body>"
print "<h2>Hello %s %s</h2>" % (name, nick)
print "</body>"
print "</html>"
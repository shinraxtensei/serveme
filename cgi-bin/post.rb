#!/usr/bin/env ruby
require 'cgi'
cgi = CGI.new

name = cgi['name'] # => "Maria"
age = cgi['age']

body = "Hello #{name} #{age}"

len = body.size
puts "HTTP/1.1 200 OK\r\n"
puts "Content-Type: text/plain\r\n"
puts "Content-Length: #{len}\r\n\r\n"
# puts "Content-Length: 13\r\n\r\n"

puts body
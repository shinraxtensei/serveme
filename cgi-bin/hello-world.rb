#!/usr/bin/env ruby

require 'cgi'

# Set content type to plain text
body = "Hello, world! From Ruby!"
len = body.size
puts "HTTP/1.1 200 OK\r\n"
puts "Content-Type: text/plain\r\n"
puts "Content-Length: #{len}\r\n\r\n"
# puts "Content-Length: 13\r\n\r\n"

puts body
# puts "Hello, world! From Ruby!"
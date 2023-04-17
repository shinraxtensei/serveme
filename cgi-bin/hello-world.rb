#!/usr/bin/env ruby

require 'cgi'

# Set content type to plain text
puts "HTTP/1.1 200 OK\r\n"
puts "Content-Type: text/plain\r\n"
puts "Content-Length: 13\r\n\r\n"

# Print a message
puts "Hello, world!"
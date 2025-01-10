#!/usr/bin/python3

import cgi
import cgitb
cgitb.enable()  # Enable debugging for CGI scripts

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue("name", "(no name provided)")
message = form.getvalue("message", "(no message provided)")

# Generate response
response = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>POST Request Response</title>
</head>
<body>
    <h1>POST Request Received</h1>
    <p><strong>Name:</strong> {name}</p>
    <p><strong>Message:</strong> {message}</p>
    <a href="/">Go Back</a>
</body>
</html>
"""
# headers
print("Content-Type: text/html\r\n", end="")
print(f"Content-Length:", len(response), "\r\n", end="")
print(f"Connection: close", "\r\n", end="")
print("\r\n", end="")
print(response, end="")

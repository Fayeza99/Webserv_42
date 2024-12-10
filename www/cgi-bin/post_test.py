#!/usr/bin/python3

import cgi
import cgitb
cgitb.enable()  # Enable debugging for CGI scripts

print("Content-Type: text/html\n")  # HTTP header

# Parse form data
form = cgi.FieldStorage()
name = form.getvalue("name", "(no name provided)")
message = form.getvalue("message", "(no message provided)")

# Generate response
print(f"""
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
    <a href="/form.html">Go Back</a>
</body>
</html>
""")

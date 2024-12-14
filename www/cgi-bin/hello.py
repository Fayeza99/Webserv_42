#!/usr/bin/python3

import os
import cgitb

cgitb.enable()  # Enable debugging for CGI scripts
env = os.environ

# Generate response
response = """
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>POST Request Response</title>
</head>
<body>
<h1>Hello!</h1>
<p>this page is kinda pointless.</p>
<p>but here are the environment variables for you:</p>"""

for e in env:
	response += f"<p>{e} = {env[e]}</p>\n"
response += """</body></html>"""

# headers
print("Content-Type: text/html\r\n", end="")
print(f"Content-Length:", len(response), "\r\n", end="")
print("\r\n", end="")
print(response, end="")

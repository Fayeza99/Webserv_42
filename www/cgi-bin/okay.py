#!/Users/asemsey/.brew/bin/python3

import cgitb
cgitb.enable()  # Enable debugging for CGI scripts

# Generate response
response = f"""
<!DOCTYPE html>
<html lang="en">
<body>
<h1>the python script works!</h1>
<p>good job anna</p>
</body>
</html>
"""
# headers
print("Content-Type: text/html\r\n", end="")
print(f"Content-Length:", len(response), "\r\n", end="")
print("\r\n", end="")
print(response, end="")

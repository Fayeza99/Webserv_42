#!/usr/bin/python3
import os

env = os.environ

print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n")
print("<!DOCTYPE html>")
print("<html>")
print("<body>")
print("<h1>Hello!</h1>")
print("<p>this page is kinda pointless.</p>\n<p>but here are the environment variables for you:", env , "</p>")
print("</body>")
print("</html>")

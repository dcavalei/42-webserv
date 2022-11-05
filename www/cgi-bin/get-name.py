#!/usr/bin/python3
import cgi

form = cgi.FieldStorage()
if form.getvalue('name'):
    name = form.getvalue('name')
else:
    name = "Nameless King"

print("Content-type: text/plain", end='\n')
print(f"Content-length: {len(name)}\r\n", end='\n')
print(f"{name}")

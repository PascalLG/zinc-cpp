#! /usr/bin/python

import os
import cgi

print "Content-Type: text/html"
print
print """<!doctype html>
<html lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
	<link rel="stylesheet" type="text/css" href="style.css"/>
	<title>Tests</title>
</head>
<body>
	<div id="header">Simple form</div>
	<div id="content">
		<h1>Environment variables</h1>
		<table>
		<tr><th>Key</th><th>Value</th></tr>
"""

env = os.environ
for key in env:
	print '<tr><td>' + key + '</td><td>' + env[key] + '</td></tr>'

print """		</table>
		<h1>Form/query string fields</h1>
		<table>
		<tr><th>Key</th><th>Value</th></tr>
"""

form = cgi.FieldStorage()
for key in form:
	print '<tr><td>' + key + '</td><td>' + form[key].value + '</td></tr>'


print """		</table>		<p><a href="index.html">Back to home page</a></p>
	</div>
</body>
</html>"""

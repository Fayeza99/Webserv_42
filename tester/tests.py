import requests as req

URL = "http://localhost/"
PARAMS = {}

r = req.get(url = URL, params = PARAMS)
print(r)

# data = r.json()

import json

books = '[{"title":"Harry Potter"}, {"title":"The Subtle Art Of Not Giving A F*ck"}]'

with open("../db/books.json", "w") as file:
	file.write(books)
	# books = json.loads(books)
	# file.write(str(books))
print("wrote data to file")
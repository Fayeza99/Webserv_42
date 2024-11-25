# import json

# db_path = "../db/books.json"

# body = ""
# while (True):
# 	try:
# 		stdin = input()
# 	except EOFError:
# 		break
# 	body += stdin + "\n"
# params = body.split("&")

# with open(db_path) as file:
# 	db_data = json.load(file)

# entry = {}
# for p in params:
# 	pair = p.split('=')

# 	entry[pair[0]] = pair[1].strip("\n")
# db_data.append(entry)

# json_string = json.dumps(db_data, ensure_ascii=False)
# with open(db_path, "w") as file:
# 	file.write(json_string)

# print(db_data, "\r\n")
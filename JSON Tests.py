import json
import sys


data={"test1" : "1", "test2" : "2", "test3" : "3"}
data2 = [{"lok_dir": 0, "lok_id": 1, "session_id": "ead63992950643aba4fb067ff8c7461c", "lok_name": "S-BAHN", "lok_speed": 0, "image_url": "/static/S-Bahn.jpg", "client_id": 1, "user_name": "Fred"}]

print (data2)
print ("")
print (json.dumps(data2))

json_str = json.dumps(data)


resp = json.loads(json_str)


print (resp)


print (resp['test1'])

for item in data2:
   print ("Next Line", item)
   print (item["lok_name"])



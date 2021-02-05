import json
import collections
from TrainControllLok import Lok
from pathlib import Path




filename = Path('config/loklist.json')
if not filename.exists():
    print("Oops, file doesn't exist!")

data_file = open(filename)
loklist_json = json.load(data_file)

for item in loklist_json:
    # Create Instances for each lok
    Lok(id=item["id"],name=item["name"], image_url=item["image_url"], addr=item["addr"], protocol=item["protocol"])
Lok.printLokList()

Lok.bindLokID(1, 1, "Jochen")
Lok.bindLokID(2, 2, "Valentina")
Lok.printLokList()


old_json = Lok.getDataJSONforClient(1)

print( old_json )

a_dict_json = json.loads(old_json)
keys = a_dict_json.keys()
values = a_dict_json.values()
def create_object(self):
    return collections.namedtuple('object_name', keys)(* values)

ls_lok = json.loads(old_json, object_hook=create_object)
print(ls_lok.name)

Lok.set(5, ls_lok)
Lok.printLokList()
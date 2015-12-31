import sys
import struct
import json
import os

STORY_DATA_STRING = "STORY_DATA_"

def write_story(story, datafile):
    nextlocation = 0
    count = 1
    
    binarydata = struct.pack('h', int(story["id"]))
    binarydata += struct.pack('h', int(story["version"]))
    binarydata += struct.pack('h', len(story["name"]))
    binarydata += story["name"]
    binarydata += struct.pack('h', len(story["description"]))
    binarydata += story["description"]
    
    datafile.write(struct.pack('h', count))
    nextlocation = (1 + 2 * count) * 2
    datafile.write(struct.pack('h', nextlocation))
    datafile.write(struct.pack('h', len(binarydata)))
    datafile.write(binarydata)


appinfo = {}
data_objects = []

with open("appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)

with open("src_data/stories.txt") as stories:
    for line in stories.readlines():
        story_filename = "src_data/" + line.strip()
        story_datafile = "Auto" + os.path.splitext(line.strip())[0]+'.dat'
        with open(story_filename) as story_file:
            story = json.load(story_file)
            with open("resources/data/" + story_datafile, 'wb') as datafile:
                write_story(story, datafile)
                newobject = {"file": "data/" + story_datafile, "name": STORY_DATA_STRING + os.path.splitext(story_datafile)[0].upper(), "type": "raw"}
                data_objects.append(newobject)


medialist = appinfo["resources"]["media"]

for newobject in data_objects:
    found = False
    for object in medialist:
        if object["file"] == newobject["file"]:
            found = True
    if not found:
        medialist.append(newobject)

for object in list(medialist):
    found = False
    if object["name"][:len(STORY_DATA_STRING)] != STORY_DATA_STRING:
        continue
    for newobject in data_objects:
        if object["file"] == newobject["file"]:
            found = True
    if not found:
        medialist.remove(object)
        os.remove("resources/" + object["file"])

with open("appinfo.json", 'w') as appinfo_file:
   json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
   appinfo_file.write("\n")

with open("src/AutoStoryList.h", 'w') as storylist_file:
    if len(data_objects) == 0:
        storylist_file.write("int autoStoryList[] = {0};")
    else:
        storylist_file.write("int autoStoryList[] = \n")
        storylist_file.write("{\n")
        for object in data_objects:
            storylist_file.write("\tRESOURCE_ID_" + object["name"] + ",\n")
        storylist_file.write("};\n")

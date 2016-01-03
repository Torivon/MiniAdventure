import sys
import struct
import json

appinfo = {}

with open("appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)


with open("appinfo.json.new", 'w') as appinfo_file:
    json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
    appinfo_file.write("\n")

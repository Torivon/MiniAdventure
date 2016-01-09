import json
import os
import uuid
import sys

if len(sys.argv) < 4:
    print("Too few arguments")
    print("python dev_tools/init_new_project.py <AuthorName> <shortname> <longname>")
    quit()

appinfo = {}

# Load appinfo so we can configure the stories and modify it to match
with open("src_data/base-appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)

appinfo["companyName"] = sys.argv[1]
appinfo["shortName"] = sys.argv[2]
appinfo["longName"] = sys.argv[3]
appinfo["uuid"] = str(uuid.uuid4())

# Write out the new appinfo file
with open("src_data/base-appinfo.json", 'w') as appinfo_file:
    json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
    appinfo_file.write("\n")

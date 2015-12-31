import sys
import struct
import json
import os

STORY_DATA_STRING = "STORY_DATA_"

g_size_constants = {}
g_size_constants["MAX_STORY_NAME_LENGTH"] = 256
g_size_constants["MAX_STORY_DESC_LENGTH"] = 256
g_size_constants["MAX_ADJACENT_LOCATIONS"] = 10
g_size_constants["MAX_BACKGROUND_IMAGES"] = 10

def pack_integer(i):
    '''
    Write out an integer into a packed binary file
    '''
    return struct.pack('h', i)

def pack_string(s):
    '''
    Write out a string into a packed binary file
    '''
    binarydata = struct.pack('h', len(s))
    binarydata += s
    return binarydata

def pack_location(location):
    '''
    Write out all information needed for a location into a packed binary file
    '''
    binarydata = pack_string(location["name"])
    binarydata += pack_integer(len(location["adjacent_locations_index"]))
    for adjacent in location["adjacent_locations_index"]:
        binarydata += pack_integer(adjacent)
    binarydata += pack_integer(len(location["background_images_index"]))
    for image in location["background_images_index"]:
        binarydata += pack_integer(image)

    return binarydata

def pack_story(story):
    '''
    Write out the main information for a story into a packed binary file
    '''
    binarydata = pack_integer(int(story["id"]))
    binarydata += pack_integer(int(story["version"]))
    binarydata += pack_string(story["name"])
    binarydata += pack_string(story["description"])
    binarydata += pack_integer(int(story["start_location_index"]))
    return binarydata

def get_total_objects(story):
    '''
    Given a story, how many distinct objects will it have?
    This includes locations, monsters, classes, skills, and possibly more
    '''
    return 1 + len(story["locations"])

def write_story(story, datafile):
    '''
    Take a story and write all of its parts into datafile
    '''
    next_write_location = 0

    # we always write count first, though it is not clear it is necessary
    count = get_total_objects(story)
    datafile.write(struct.pack('h', count))
    # For each object in the file, we store two 16 bit (2 byte) integers, start index and size.
    # With an additional number for count, this gives us the location to start writing actual object data.
    next_write_location = (1 + 2 * count) * 2
    
    # Here, we generate the binary data for the main story object, and write out its size
    binarydata = pack_story(story)
    datafile.write(struct.pack('h', next_write_location))
    datafile.write(struct.pack('h', len(binarydata)))
    next_write_location += len(binarydata)
    
    # This loop walks all locations. For each one, we add the packed data to binarydata
    # and write out the location and size directly to the file.
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location_binary = pack_location(location)
        datafile.write(struct.pack('h', next_write_location))
        datafile.write(struct.pack('h', len(location_binary)))
        next_write_location += len(location_binary)
        binarydata += location_binary
    
    # Now that all the index and size data has been written, write out the accumulated data
    datafile.write(binarydata)

def process_story(story, imagelist):
    '''
    Here we prepare the story for being written to a packed binary file.
    We have to turn each reference to an object into what will become the 
    index of that object. In addition, we generate a map for image resource references.
    '''
    location_map = {}
    data_index = 1 # 0 is reserved for the main story struct
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location_map[location["id"]] = data_index
        data_index += 1
        location["background_images_index"] = []
        for background_image in location["background_images"]:
            if imagelist.count(background_image) == 0:
                imagelist.append(background_image)
            location["background_images_index"].append(imagelist.index(background_image))


    story["start_location_index"] = location_map[story["start_location"]]
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location["adjacent_locations_index"] = []
        for adjacent in location["adjacent_locations"]:
            location["adjacent_locations_index"].append(location_map[adjacent])


appinfo = {}
data_objects = []
imagelist = []

# Load appinfo so we can configure the stories and modify it to match
with open("appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)

# Process the stories to include. This includes generating the data files,
# adding them to the appinfo, and storing a list of images used.
with open("src_data/stories.txt") as stories:
    for line in stories.readlines():
        story_filename = "src_data/" + line.strip()
        story_datafile = "Auto" + os.path.splitext(line.strip())[0]+'.dat'
        with open(story_filename) as story_file:
            story = json.load(story_file)
            process_story(story, imagelist)
            with open("resources/data/" + story_datafile, 'wb') as datafile:
                write_story(story, datafile)
                newobject = {"file": "data/" + story_datafile, "name": STORY_DATA_STRING + os.path.splitext(story_datafile)[0].upper(), "type": "raw"}
                data_objects.append(newobject)


medialist = appinfo["resources"]["media"]

# Make a mapping of image indexes to resource ids
for object in medialist:
    if imagelist.count(object["file"]) > 0:
        index = imagelist.index(object["file"])
        imagelist[index] = object["name"]

# Walk the list of story objects and make sure they are in appinfo
for newobject in data_objects:
    found = False
    for object in medialist:
        if object["file"] == newobject["file"]:
            found = True
    if not found:
        medialist.append(newobject)

# the appinfo medialist and remove any files for stories that should no longer be included
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

# Write out the new appinfo file
with open("appinfo.json", 'w') as appinfo_file:
   json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
   appinfo_file.write("\n")

# Write out a mapping of image indexes to resource ids in a header file
with open("src/AutoImageMap.h", 'w') as imagemap_file:
    if len(imagelist) == 0:
        imagemap_file.write("int imageResourceMap[] = {0};")
    else:
        imagemap_file.write("int autoImageMap[] = \n")
        imagemap_file.write("{\n")
        for image in imagelist:
            imagemap_file.write("\tRESOURCE_ID_" + image + ",\n")
        imagemap_file.write("};\n")

# Write out the story list in a header file
with open("src/AutoStoryList.h", 'w') as storylist_file:
    if len(data_objects) == 0:
        storylist_file.write("int autoStoryList[] = {0};")
    else:
        storylist_file.write("int autoStoryList[] = \n")
        storylist_file.write("{\n")
        for object in data_objects:
            storylist_file.write("\tRESOURCE_ID_" + object["name"] + ",\n")
        storylist_file.write("};\n")

# Write out size constants in a header file so they match at both process time and load time
with open("src/AutoSizeConstants.h", 'w') as constants_file:
        for k, v in g_size_constants.items():
            constants_file.write("#define " + k + " " + str(v) + "\n")

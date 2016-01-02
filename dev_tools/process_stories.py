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
g_size_constants["MAX_MONSTERS"] = 10

g_skill_types = {}
g_skill_types["attack"] = 0
g_skill_types["heal"] = 1
g_skill_types["counter"] = 2
g_skill_types["buff"] = 3
g_skill_types["debuff"] = 4

g_damage_types = {}
g_damage_types["physical"] = 1 << 0
g_damage_types["magic"] = 1 << 1
g_damage_types["fire"] = 1 << 2
g_damage_types["ice"] = 1 << 3
g_damage_types["lightning"] = 1 << 4
g_damage_types["slashing"] = 1 << 5
g_damage_types["piercing"] = 1 << 6
g_damage_types["bludgeoning"] = 1 << 7

g_combatant_ranks = {}
g_combatant_ranks["rankf"] = 0
g_combatant_ranks["ranke"] = 1
g_combatant_ranks["rankd"] = 2
g_combatant_ranks["rankc"] = 3
g_combatant_ranks["rankb"] = 4
g_combatant_ranks["ranka"] = 5
g_combatant_ranks["ranks"] = 6

g_combatant_stats = ["strength", "magic", "defense", "magic_defense", "speed", "health"]

def pack_integer(i):
    '''
    Write out an integer into a packed binary file
    '''
    return struct.pack('<h', i)

def pack_string(s):
    '''
    Write out a string into a packed binary file
    '''
    binarydata = struct.pack('h', len(s))
    binarydata += str(s) #struct.pack('s', str(s))
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
    if location.has_key("length"):
        binarydata += pack_integer(int(location["length"]))
    else:
        binarydata += pack_integer(0)
    if location.has_key("base_level"):
        binarydata += pack_integer(int(location["base_level"]))
    else:
        binarydata += pack_integer(0)
    if location.has_key("encounter_chance"):
        binarydata += pack_integer(int(location["encounter_chance"]))
    else:
        binarydata += pack_integer(0)
    if location.has_key("monsters_index"):
        binarydata += pack_integer(len(location["monsters_index"]))
        for monster in location["monsters_index"]:
            binarydata += pack_integer(monster)
    else:
        binarydata += pack_integer(0)
    return binarydata

def pack_skill(skill):
    '''
    Write out all information needed for a skill into a packed binary file
    '''
    binarydata = pack_string(skill["name"])
    binarydata += pack_string(skill["description"])
    binarydata += pack_integer(skill["type_value"])
    binarydata += pack_integer(skill["speed"])
    binarydata += pack_integer(skill["damage_type_value"])
    binarydata += pack_integer(skill["potency"])
    return binarydata

def pack_monster(monster):
    '''
    Write out all information needed for a monster into a packed binary file
    '''
    binarydata = pack_string(monster["name"])
    binarydata += pack_integer(monster["image_index"])
    for stat in monster["combatantclass_values"]:
        binarydata += pack_integer(stat)
    binarydata += pack_integer(len(monster["skill_list"]))
    for index in range(len(monster["skill_list"])):
        skill = monster["skill_list"][index]
        binarydata += pack_integer(skill["index"])
        binarydata += pack_integer(skill["min_level"])
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
    count = 1 #main object
    count += len(story["locations"])
    if story.has_key("skills"):
        count += len(story["skills"])
    if story.has_key("monsters"):
        count += len(story["monsters"])
    return count

def write_story(story, datafile):
    '''
    Take a story and write all of its parts into datafile
    '''
    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.

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
    
    if story.has_key("skills"):
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["skills"])):
            skill = story["skills"][index]
            skill_binary = pack_skill(skill)
            datafile.write(struct.pack('h', next_write_location))
            datafile.write(struct.pack('h', len(skill_binary)))
            next_write_location += len(skill_binary)
            binarydata += skill_binary

    if story.has_key("monsters"):
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["monsters"])):
            monster = story["monsters"][index]
            monster_binary = pack_monster(monster)
            datafile.write(struct.pack('h', next_write_location))
            datafile.write(struct.pack('h', len(monster_binary)))
            next_write_location += len(monster_binary)
            binarydata += monster_binary

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

def process_skills(story, skill_map, data_index):
    if not story.has_key("skills"):
        return data_index
   
    for index in range(len(story["skills"])):
        skill = story["skills"][index]
        skill_map[skill["id"]] = data_index
        data_index += 1

        skill["type_value"] = g_skill_types[skill["type"]]
        skill["damage_type_value"] = 0
        for damage_type in skill["damage_types"]:
            skill["damage_type_value"] = skill["damage_type_value"] | g_damage_types[damage_type]

    return data_index

def process_monsters(story, monster_map, skill_map, imagelist, data_index):
    if not story.has_key("monsters"):
        return data_index
    
    for index in range(len(story["monsters"])):
        monster = story["monsters"][index]
        monster_map[monster["id"]] = data_index
        data_index += 1
        if imagelist.count(monster["image"]) == 0:
            imagelist.append(monster["image"])
        monster["image_index"] = imagelist.index(monster["image"])
        monster["combatantclass_values"] = []
        for stat_name in g_combatant_stats:
            monster["combatantclass_values"].append(g_combatant_ranks[monster["combatantclass"][stat_name]])

        for skill_index in range(len(monster["skill_list"])):
            skill = monster["skill_list"][skill_index]
            skill["index"] = skill_map[skill["id"]]

    return data_index

def process_locations(story, monster_map, imagelist, data_index):
    if not story.has_key("locations"):
        return data_index

    location_map = {}
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location_map[location["id"]] = data_index
        data_index += 1
        location["background_images_index"] = []
        for background_image in location["background_images"]:
            if imagelist.count(background_image) == 0:
                imagelist.append(background_image)
            location["background_images_index"].append(imagelist.index(background_image))
        if location.has_key("monsters"):
            location["monsters_index"] = []
            for monster in location["monsters"]:
                location["monsters_index"].append(monster_map[monster])

    story["start_location_index"] = location_map[story["start_location"]]
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location["adjacent_locations_index"] = []
        for adjacent in location["adjacent_locations"]:
            location["adjacent_locations_index"].append(location_map[adjacent])

    return data_index


def process_dungeons(story):
    '''
    This takes a dungeon definition and unrolls it into the proper set of
    locations. These are then added to the location list to be handled in the 
    next step
    '''

    if not story.has_key("dungeons"):
        return

    for dungeonindex in range(len(story["dungeons"])):
        dungeon = story["dungeons"][dungeonindex]
        idlist = []
        namelist = []
        floors = int(dungeon["floors"])
        for floor in range(floors):
            if floor == 0:
                idsuffix = "_start"
            elif floor == floors - 1:
                idsuffix = "_end"
            else:
                idsuffix = "_" + str(floor + 1)
            idlist.append(dungeon["id"] + idsuffix)
            namelist.append(dungeon["name"] + " Floor " + str(floor + 1))
            if floor < floors - 1:
                idlist.append(dungeon["id"] + idsuffix + "_end")
                namelist.append(dungeon["name"] + " Floor " + str(floor + 1) + " End")
        
        for index in range(len(idlist)):
            location = {}
            location["id"] = idlist[index]
            location["name"] = namelist[index]
            location["adjacent_locations"] = []
            if index == 0:
                location["adjacent_locations"].append(dungeon["adjacent_locations"][0])
            else:
                location["adjacent_locations"].append(idlist[index - 1])

            if index == len(idlist) - 1:
                location["adjacent_locations"].append(dungeon["adjacent_locations"][1])
            else:
                location["adjacent_locations"].append(idlist[index + 1])

            if index % 2 == 0:
                location["background_images"] = list(dungeon["background_images"])
                location["length"] = dungeon["length"]
            else:
                location["background_images"] = list(dungeon["fixed_background_image"])
                location["length"] = 0

            story["locations"].append(location)

def process_story(story, imagelist):
    '''
    Here we prepare the story for being written to a packed binary file.
    We have to turn each reference to an object into what will become the 
    index of that object. In addition, we generate a map for image resource references.
    '''
    
    # This just unrolls the dungeon definitions into the appropriate
    # number of locations. Actual writing to the file happens when
    # we process the rest of the locations.
    process_dungeons(story)

    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.
    data_index = 1 # 0 is reserved for the main story struct

    skill_map = {}
    data_index = process_skills(story, skill_map, data_index)
    
    monster_map = {}
    data_index = process_monsters(story, monster_map, skill_map, imagelist, data_index)
    
    data_index = process_locations(story, monster_map, imagelist, data_index)

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

with open("src/AutoSkillConstants.h", 'w') as skill_file:
    skill_file.write("#pragma once\n\n")
    for k, v in g_skill_types.items():
        skill_file.write("#define SKILL_TYPE_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

    for k, v in g_damage_types.items():
        skill_file.write("#define DAMAGE_TYPE_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

with open("src/AutoCombatantConstants.h", 'w') as skill_file:
    skill_file.write("#pragma once\n\n")
    for k, v in g_combatant_ranks.items():
        skill_file.write("#define COMBATANT_RANK_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

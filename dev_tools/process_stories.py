import sys
import struct
import json
import os
import hashlib
import shutil

STORY_DATA_STRING = "STORY_DATA_"

g_size_constants = {}
g_size_constants["MAX_STORY_NAME_LENGTH"] = 32
g_size_constants["MAX_STORY_DESC_LENGTH"] = 64
g_size_constants["MAX_ADJACENT_LOCATIONS"] = 10
g_size_constants["MAX_BACKGROUND_IMAGES"] = 10
g_size_constants["MAX_MONSTERS"] = 10
g_size_constants["MAX_SKILLS_IN_LIST"] = 15
g_size_constants["MAX_CLASSES"] = 5

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
g_damage_types["holy"] = 1 << 8

g_combatant_ranks = {}
g_combatant_ranks["rankf"] = 0
g_combatant_ranks["ranke"] = 1
g_combatant_ranks["rankd"] = 2
g_combatant_ranks["rankc"] = 3
g_combatant_ranks["rankb"] = 4
g_combatant_ranks["ranka"] = 5
g_combatant_ranks["ranks"] = 6

g_location_properties = {}
g_location_properties["rest_area"] = 1 << 0
g_location_properties["game_win"] = 1 << 1
g_location_properties["level_up"] = 1 << 2

g_combatant_stats = ["strength", "magic", "defense", "magic_defense", "speed", "health"]

def pack_integer(i):
    '''
    Write out an integer into a packed binary file
    '''
    return struct.pack('<h', i)

def pack_integer_with_default(dict, key, default):
    if key in dict:
        binarydata = pack_integer(dict[key])
    else:
        binarydata = pack_integer(default)
    return binarydata

def pack_string(s, max_length):
    '''
    Write out a string into a packed binary file
    '''
    binarydata = struct.pack(str(max_length) + 's', s.encode('ascii'))
    return binarydata

def pack_location(location):
    '''
    Write out all information needed for a location into a packed binary file
    '''
    binarydata = pack_string(location["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_integer(len(location["adjacent_locations_index"]))
    for index in range(g_size_constants["MAX_ADJACENT_LOCATIONS"]):
        if index < len(location["adjacent_locations_index"]):
            binarydata += pack_integer(location["adjacent_locations_index"][index])
        else:
            binarydata += pack_integer(0)
    binarydata += pack_integer(len(location["background_images_index"]))
    for index in range(g_size_constants["MAX_BACKGROUND_IMAGES"]):
        if index < len(location["background_images_index"]):
            binarydata += pack_integer(location["background_images_index"][index])
        else:
            binarydata += pack_integer(0)
    binarydata += pack_integer_with_default(location, "location_properties_value", 0)
    binarydata += pack_integer_with_default(location, "length", 0)
    binarydata += pack_integer_with_default(location, "base_level", 0)
    binarydata += pack_integer_with_default(location, "encounter_chance", 0)
    if "monsters_index" in location:
        binarydata += pack_integer(len(location["monsters_index"]))
        for index in range(g_size_constants["MAX_MONSTERS"]):
            if index < len(location["monsters_index"]):
                binarydata += pack_integer(location["monsters_index"][index])
            else:
                binarydata += pack_integer(0)
    else:
        binarydata += pack_integer(0)
        for index in range(g_size_constants["MAX_MONSTERS"]):
            binarydata += pack_integer(0)
    return binarydata

def pack_skill(skill):
    '''
    Write out all information needed for a skill into a packed binary file
    '''
    binarydata = pack_string(skill["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string(skill["description"], g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integer(skill["type_value"])
    binarydata += pack_integer(skill["speed"])
    binarydata += pack_integer(skill["damage_types_value"])
    binarydata += pack_integer(skill["potency"])
    binarydata += pack_integer(skill["cooldown"])
    return binarydata

def pack_battler(battler):
    '''
    Write out all information needed for a battler into a packed binary file
    '''
    binarydata = pack_string(battler["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    if "description" in battler:
        binarydata += pack_string(battler["description"], g_size_constants["MAX_STORY_DESC_LENGTH"])
    else:
        binarydata += pack_string("", g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integer(battler["image_index"])
    for stat in battler["combatantclass_values"]:
        binarydata += pack_integer(stat)
    binarydata += pack_integer(len(battler["skill_list"]))
    for index in range(g_size_constants["MAX_SKILLS_IN_LIST"]):
        if index < len(battler["skill_list"]):
            skill = battler["skill_list"][index]
            binarydata += pack_integer(skill["index"])
            binarydata += pack_integer(skill["min_level"])
        else:
            binarydata += pack_integer(0)
            binarydata += pack_integer(0)

    binarydata += pack_integer_with_default(battler, "vulnerable_value", 0)
    binarydata += pack_integer_with_default(battler, "resistant_value", 0)
    binarydata += pack_integer_with_default(battler, "immune_value", 0)
    binarydata += pack_integer_with_default(battler, "absorb_value", 0)

    return binarydata

def pack_story(story, hash):
    '''
    Write out the main information for a story into a packed binary file
    '''
    binarydata = pack_integer(story["id"])
    binarydata += pack_integer(story["version"])
    binarydata += pack_integer(hash)
    binarydata += pack_string(story["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string(story["description"], g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integer(story["start_location_index"])
    binarydata += pack_integer_with_default(story, "xp_monsters_per_level", 0)
    binarydata += pack_integer_with_default(story, "xp_difference_scale", 0)
    binarydata += pack_integer(len(story["classes_index"]))
    for index in range(g_size_constants["MAX_CLASSES"]):
        if index < len(story["classes_index"]):
            binarydata += pack_integer(story["classes_index"][index])
        else:
            binarydata += pack_integer(0)
    return binarydata

def get_total_objects(story):
    '''
    Given a story, how many distinct objects will it have?
    This includes locations, monsters, classes, skills, and possibly more
    '''
    count = 1 #main object
    count += len(story["locations"])
    if "skills" in story:
        count += len(story["skills"])
    if "battlers" in story:
        count += len(story["battlers"])
    return count

def write_story(story, datafile, hash):
    '''
    Take a story and write all of its parts into datafile
    '''
    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.

    next_write_location = 0

    # we always write count first, though it is not clear it is necessary
    count = get_total_objects(story)
    datafile.write(pack_integer(count))
    # For each object in the file, we store two 16 bit (2 byte) integers, start index and size.
    # With an additional number for count, this gives us the location to start writing actual object data.
    next_write_location = (1 + 2 * count) * 2
    
    # Here, we generate the binary data for the main story object, and write out its size
    binarydata = pack_story(story, hash)
    datafile.write(pack_integer(next_write_location))
    datafile.write(pack_integer(len(binarydata)))
    next_write_location += len(binarydata)
    
    if "skills" in story:
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["skills"])):
            skill = story["skills"][index]
            skill_binary = pack_skill(skill)
            datafile.write(pack_integer(next_write_location))
            datafile.write(pack_integer(len(skill_binary)))
            next_write_location += len(skill_binary)
            binarydata += skill_binary

    if "battlers" in story:
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["battlers"])):
            battler = story["battlers"][index]
            battler_binary = pack_battler(battler)
            datafile.write(pack_integer(next_write_location))
            datafile.write(pack_integer(len(battler_binary)))
            next_write_location += len(battler_binary)
            binarydata += battler_binary

    # This loop walks all locations. For each one, we add the packed data to binarydata
    # and write out the location and size directly to the file.
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location_binary = pack_location(location)
        datafile.write(pack_integer(next_write_location))
        datafile.write(pack_integer(len(location_binary)))
        next_write_location += len(location_binary)
        binarydata += location_binary
    
    # Now that all the index and size data has been written, write out the accumulated data
    datafile.write(binarydata)

def process_bit_field(dict, field, global_dict):
    if not field in dict:
        return

    new_field_name = field + "_value"
    dict[new_field_name] = 0
    for damage_type in dict[field]:
        dict[new_field_name] = dict[new_field_name] | global_dict[damage_type]

def process_skills(story, skill_map, data_index):
    if not "skills" in story:
        return data_index
   
    for index in range(len(story["skills"])):
        skill = story["skills"][index]
        
        if len(skill["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Name is too long: " + skill["name"])
        if len(skill["description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
            quit("Description is too long: " + skill["description"])

        skill_map[skill["id"]] = data_index
        data_index += 1

        skill["type_value"] = g_skill_types[skill["type"]]
        process_bit_field(skill, "damage_types", g_damage_types)

    return data_index

def process_battlers(story, battler_map, skill_map, imagelist, data_index):
    if not "battlers" in story:
        return data_index
    
    for index in range(len(story["battlers"])):
        battler = story["battlers"][index]
        
        if len(battler["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Name is too long: " + battler["name"])
        if "description" in battler and len(battler["description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
            quit("Description is too long: " + battler["description"])
        if len(battler["skill_list"]) > g_size_constants["MAX_SKILLS_IN_LIST"]:
            quit("Too many skills for " + battler["name"])

        battler_map[battler["id"]] = data_index
        data_index += 1
        if imagelist.count(battler["image"]) == 0:
            imagelist.append(battler["image"])
        battler["image_index"] = imagelist.index(battler["image"])
        battler["combatantclass_values"] = []
        for stat_name in g_combatant_stats:
            battler["combatantclass_values"].append(g_combatant_ranks[battler["combatantclass"][stat_name]])

        for skill_index in range(len(battler["skill_list"])):
            skill = battler["skill_list"][skill_index]
            skill["index"] = skill_map[skill["id"]]

        process_bit_field(battler, "vulnerable", g_damage_types)
        process_bit_field(battler, "resistant", g_damage_types)
        process_bit_field(battler, "immune", g_damage_types)
        process_bit_field(battler, "absorb", g_damage_types)

    return data_index

def process_locations(story, battler_map, imagelist, data_index):
    if not "locations" in story:
        return data_index

    location_map = {}
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        
        if len(location["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Name is too long: " + location["name"])
        if len(location["adjacent_locations"]) > g_size_constants["MAX_ADJACENT_LOCATIONS"]:
            quit("Too many adjacent locations for " + location["name"])
        if len(location["background_images"]) > g_size_constants["MAX_BACKGROUND_IMAGES"]:
            quit("Too many background images for " + location["name"])
        if "monsters" in location and len(location["monsters"]) > g_size_constants["MAX_MONSTERS"]:
            quit("Too many monsters for " + location["name"])

        process_bit_field(location, "location_properties", g_location_properties)

        location_map[location["id"]] = data_index
        data_index += 1
        location["background_images_index"] = []
        for background_image in location["background_images"]:
            if imagelist.count(background_image) == 0:
                imagelist.append(background_image)
            location["background_images_index"].append(imagelist.index(background_image))
        if "monsters" in location:
            location["monsters_index"] = []
            for monster in location["monsters"]:
                location["monsters_index"].append(battler_map[monster])

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

    if not "dungeons" in story:
        return

    for dungeonindex in range(len(story["dungeons"])):
        dungeon = story["dungeons"][dungeonindex]
        idlist = []
        namelist = []
        floors = dungeon["floors"]
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
                floor = index / 2
                location["background_images"] = list(dungeon["background_images"])
                location["length"] = dungeon["length"]
                if "encounter_chance" in dungeon:
                    location["encounter_chance"] = dungeon["encounter_chance"]
                if "base_level" in dungeon:
                    location["base_level"] = dungeon["base_level"] + int(floor / dungeon["level_rate"])
                if "monster_scaling" in dungeon and "monsters" in dungeon:
                    location["monsters"] = []
                    for monster_index in range(len(dungeon["monsters"])):
                        if dungeon["monster_scaling"] == 0:
                            location["monsters"].append(dungeon["monsters"][monster_index])
                        elif monster_index <= floor / dungeon["monster_scaling"]:
                            location["monsters"].append(dungeon["monsters"][monster_index])
            else:
                location["background_images"] = list(dungeon["fixed_background_image"])
                location["length"] = 0

            story["locations"].append(location)

def process_included_files(story, file_list_key, object_key):
    if file_list_key in story:
        for filename in story[file_list_key]:
            with open("src_data/stories/" + filename) as object_file:
                try:
                    object_list = json.load(object_file)
                except ValueError as e:
                    quit("Failed to parse file " + filename + ". Probably an extraneous ','.")
                if not object_key in story:
                    story[object_key] = []
                for newobject in object_list[object_key]:
                    for oldobject in story[object_key]:
                        if newobject["id"] == oldobject["id"]:
                            quit("Duplicate id, " + oldobject["id"] + " in list of " + object_key)
                story[object_key].extend(object_list[object_key])

def process_story(story, imagelist):
    '''
    Here we prepare the story for being written to a packed binary file.
    We have to turn each reference to an object into what will become the 
    index of that object. In addition, we generate a map for image resource references.
    '''
    
    # In order to allow stories to share skills and battlers, allow the user to
    # have a list of skill and battler files to include. process_includedfiles appends
    # the contents into the appropriate list.
    process_included_files(story, "skill_files", "skills")
    process_included_files(story, "battler_files", "battlers")
    
    # This just unrolls the dungeon definitions into the appropriate
    # number of locations. Actual writing to the file happens when
    # we process the rest of the locations.
    process_dungeons(story)

    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.
    data_index = 1 # 0 is reserved for the main story struct

    skill_map = {}
    data_index = process_skills(story, skill_map, data_index)
    
    battler_map = {}
    data_index = process_battlers(story, battler_map, skill_map, imagelist, data_index)
    
    data_index = process_locations(story, battler_map, imagelist, data_index)

    if "classes" in story:
        story["classes_index"] = []
        for battler in story["classes"]:
            story["classes_index"].append(battler_map[battler])
    else:
        quit("Must have at least one class.")

appinfo = {}
data_objects = []
imagelist = []

# Load appinfo so we can configure the stories and modify it to match
with open("src_data/base-appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)

# Process the stories to include. This includes generating the data files,
# adding them to the appinfo, and storing a list of images used.
with open("src_data/stories.txt") as stories:
    for line in stories.readlines():
        print("Processing story in " + line.strip())
        story_filename = "src_data/stories/" + line.strip()
        story_datafile = "Auto" + os.path.splitext(line.strip())[0]+'.dat'
        with open(story_filename) as story_file:
            m = hashlib.md5()
            for line in story_file.readlines():
                m.update(line.encode("ascii"))
            hash = struct.unpack("<h", m.digest()[-2:])
        with open(story_filename) as story_file:
            try:
                story = json.load(story_file)
            except ValueError as e:
                quit("Failed to parse story " + story_filename + ". Probably an extraneous ','.")
            process_story(story, imagelist)
            with open("resources/data/" + story_datafile, 'wb') as datafile:
                write_story(story, datafile, hash[0])
                newobject = {"file": "data/" + story_datafile, "name": STORY_DATA_STRING + os.path.splitext(story_datafile)[0].upper(), "type": "raw"}
                data_objects.append(newobject)

# Adds the list of art needed by the engine.
with open("src_data/imagelist.txt") as imagelist_file:
    for line in imagelist_file.readlines():
        imagename = line.strip()
        if imagelist.count(imagename) == 0:
            imagelist.append(imagename)

# Prep the appinfo for resources
if not "resources" in appinfo:
    appinfo["resources"] = {};

if not "media" in appinfo["resources"]:
    appinfo["resources"]["media"] = []

medialist = appinfo["resources"]["media"]

imagemap = []
prefixlist = []

# Add all required images to the medialist. Also creates an imagemap so we can map indexes to resource ids at run time. Lastly, creates
# the list of file prefixes so we can copy/delete the correct files.
for index in range(len(imagelist)):
    newimage = imagelist[index];
    prefix = os.path.splitext(newimage)[0]
    if prefix.count('/') > 0:
        prefix = prefix[prefix.index('/') + 1:]
    newobject = {"file": newimage, "name": "IMAGE_" + prefix.upper(), "type": "bitmap"}
    medialist.append(newobject)
    imagemap.append(newobject["name"])
    prefixlist.append(prefix)

# Walk the list of story objects and make sure they are in appinfo
for newobject in data_objects:
    medialist.append(newobject)

source_image_files = os.listdir(os.getcwd() + "/src_data/images")
dest_image_files = os.listdir(os.getcwd() + "/resources/images")

# Delete all files in the resources/images that we do not need.
for imagefile in dest_image_files:
    prefix = os.path.splitext(imagefile)[0]
    if prefix.count('~') > 0:
        prefix = prefix[:prefix.index('~')]

    if prefixlist.count(prefix) == 0:
        os.remove(os.getcwd() + "/resources/images/" + imagefile)

# Add all files to resources/images that we need.
for imagefile in source_image_files:
    prefix = os.path.splitext(imagefile)[0]
    if prefix.count('~') > 0:
        prefix = prefix[:prefix.index('~')]
    if prefixlist.count(prefix) > 0:
        shutil.copyfile(os.getcwd() + "/src_data/images/" + imagefile, os.getcwd() + "/resources/images/" + imagefile)


# Remove any old story files that are no longer included.
dest_story_files = os.listdir(os.getcwd() + "/resources/data")
for story_file in dest_story_files:
    for object in list(medialist):
        found = False
        if object["name"][:len(STORY_DATA_STRING)] != STORY_DATA_STRING:
            continue
        if object["file"] == "data/" + story_file:
            found = True
            break;
    if not found:
        os.remove("resources/data/" + story_file)

# Write out the new appinfo file
with open("appinfo.json", 'w') as appinfo_file:
   json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
   appinfo_file.write("\n")

# Write out a mapping of image indexes to resource ids in a header file
with open("src/AutoImageMap.h", 'w') as imagemap_file:
    if len(imagemap) == 0:
        imagemap_file.write("int imageResourceMap[] = {0};")
    else:
        imagemap_file.write("int autoImageMap[] = \n")
        imagemap_file.write("{\n")
        for image in imagemap:
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

# Write out skill constants in a header file so they match at both process time and load time
with open("src/AutoSkillConstants.h", 'w') as skill_file:
    skill_file.write("#pragma once\n\n")
    for k, v in g_skill_types.items():
        skill_file.write("#define SKILL_TYPE_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

    for k, v in g_damage_types.items():
        skill_file.write("#define DAMAGE_TYPE_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

# Write out combatant constants in a header file so they match at both process time and load time
with open("src/AutoCombatantConstants.h", 'w') as skill_file:
    skill_file.write("#pragma once\n\n")
    for k, v in g_combatant_ranks.items():
        skill_file.write("#define COMBATANT_RANK_" + k.upper() + " " + str(v) + "\n")

    skill_file.write("\n")

# Write out location property constants in a header file so they match at both process time and load time
with open("src/AutoLocationConstants.h", 'w') as location_file:
    location_file.write("#pragma once\n\n")
    for k, v in g_location_properties.items():
        location_file.write("#define LOCATION_PROPERTY_" + k.upper() + " " + str(v) + "\n")
    
    location_file.write("\n")

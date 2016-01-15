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
g_size_constants["MAX_DIALOG_LENGTH"] = 256
g_size_constants["MAX_GAME_STATE_VARIABLES"] = 16
g_size_constants["MAX_EVENTS"] = 10

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

def add_image(imagelist, imagename):
    if imagelist.count(imagename) == 0:
        imagelist.append(imagename)
    return imagelist.index(imagename)

def pack_bool(b):
    '''
    Write out a boolean into a packed binary file
    '''

    return struct.pack('<H', b)

def pack_bool_with_default(dict, key, default):
    if key in dict:
        binarydata = pack_bool(dict[key])
    else:
        binarydata = pack_bool(default)
    return binarydata

def pack_integer(i):
    '''
    Write out an integer into a packed binary file
    '''
    return struct.pack('<H', i)

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

def pack_string_with_default(dict, key, default, size):
    if key in dict:
        binarydata = pack_string(dict[key], size)
    else:
        binarydata = pack_string(default, size)
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

    binarydata += pack_integer_with_default(location, "initial_event_index", 0)
    if "events_index" in location:
        binarydata += pack_integer(len(location["events_index"]))
        for index in range(g_size_constants["MAX_EVENTS"]):
            if index < len(location["events_index"]):
                binarydata += pack_integer(location["events_index"][index])
            else:
                binarydata += pack_integer(0)
    else:
        binarydata += pack_integer(0)
        for index in range(g_size_constants["MAX_EVENTS"]):
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

def pack_dialog(dialog):
    binarydata = pack_string(dialog["text"], g_size_constants["MAX_DIALOG_LENGTH"])
    binarydata += pack_bool_with_default(dialog, "allow_cancel", False);
    binarydata += pack_bool(True);
    return binarydata

def pack_gamestate(dict, listkey):
    binarydata = ""
    if listkey in dict:
        for i in range(g_size_constants["MAX_GAME_STATE_VARIABLES"]):
            if i < len(dict[listkey]):
                binarydata += pack_integer(dict[listkey][i])
            else:
                binarydata += pack_integer(0)
    else:
        for i in range(g_size_constants["MAX_GAME_STATE_VARIABLES"]):
            binarydata += pack_integer(0)

    return binarydata

def pack_event(event):
    binarydata = pack_string_with_default(event, "name", "", g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_integer_with_default(event, "dialog_index", 0)
    binarydata += pack_bool_with_default(event, "use_prerequisites", False)
    binarydata += pack_gamestate(event, "positive_prerequisites_values")
    binarydata += pack_gamestate(event, "negative_prerequisites_values")
    binarydata += pack_gamestate(event, "state_changes_values")
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

    binarydata += pack_integer_with_default(battler, "event_index", 0)
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
    binarydata += pack_integer_with_default(story, "opening_dialog_index", 0)
    binarydata += pack_integer_with_default(story, "win_dialog_index", 0)
    return binarydata

def get_total_objects(story):
    '''
    Given a story, how many distinct objects will it have?
    This includes locations, monsters, classes, skills, and possibly more
    '''
    count = 1 #main object
    if "dialog" in story:
        count += len(story["dialog"])
    if "events" in story:
        count += len(story["events"])
    if "skills" in story:
        count += len(story["skills"])
    if "battlers" in story:
        count += len(story["battlers"])
    if "locations" in story:
        count += len(story["locations"])
    return count

def write_data_block(datafile, write_state, new_data):
    datafile.write(pack_integer(write_state["next_write_location"]))
    datafile.write(pack_integer(len(new_data)))
    write_state["binarydata"] += new_data
    write_state["next_write_location"] += len(new_data)

def write_story(story, datafile, hash):
    '''
    Take a story and write all of its parts into datafile
    '''
    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.

    write_state = {}
    
    write_state["next_write_location"] = 0

    # we always write count first, though it is not clear it is necessary
    count = get_total_objects(story)
    datafile.write(pack_integer(count))
    # For each object in the file, we store two 16 bit (2 byte) integers, start index and size.
    # With an additional number for count, this gives us the location to start writing actual object data.
    write_state["next_write_location"] = (1 + 2 * count) * 2
    
    # Here, we generate the binary data for the main story object, and write out its size
    write_state["binarydata"] = pack_story(story, hash)
    datafile.write(pack_integer(write_state["next_write_location"]))
    datafile.write(pack_integer(len(write_state["binarydata"])))
    write_state["next_write_location"] += len(write_state["binarydata"])
    
    if "dialog" in story:
        for index in range(len(story["dialog"])):
            dialog = story["dialog"][index]
            dialog_binary = pack_dialog(dialog)
            write_data_block(datafile, write_state, dialog_binary)

    if "events" in story:
        for index in range(len(story["events"])):
            event = story["events"][index]
            event_binary = pack_event(event)
            write_data_block(datafile, write_state, event_binary)

    if "skills" in story:
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["skills"])):
            skill = story["skills"][index]
            skill_binary = pack_skill(skill)
            write_data_block(datafile, write_state, skill_binary)

    if "battlers" in story:
        # This loop walks all skills. For each one, we add the packed data to binarydata
        # and write out the skill and size directly to the file.
        for index in range(len(story["battlers"])):
            battler = story["battlers"][index]
            battler_binary = pack_battler(battler)
            write_data_block(datafile, write_state, battler_binary)

    # This loop walks all locations. For each one, we add the packed data to binarydata
    # and write out the location and size directly to the file.
    for index in range(len(story["locations"])):
        location = story["locations"][index]
        location_binary = pack_location(location)
        write_data_block(datafile, write_state, location_binary)
    
    # Now that all the index and size data has been written, write out the accumulated data
    datafile.write(write_state["binarydata"])

def process_bit_field(dict, field, global_dict):
    if not field in dict:
        return

    new_field_name = field + "_value"
    dict[new_field_name] = 0
    for damage_type in dict[field]:
        dict[new_field_name] = dict[new_field_name] | global_dict[damage_type]

def process_dialog(story, dialog_map, data_index):
    if not "dialog" in story:
        return data_index

    for index in range(len(story["dialog"])):
        dialog = story["dialog"][index]
        
        if len(dialog["text"]) >= g_size_constants["MAX_DIALOG_LENGTH"]:
            quit("Text is too long: " + dialog["text"])
        
        dialog_map[dialog["id"]] = data_index
        data_index += 1

    return data_index

def add_gamestate_to_list(gamestate_list, newstate):
    if gamestate_list.count(newstate) > 0:
        return

    gamestate_list.append(newstate)

def apply_variable(gamestate_values, bit):
    variable_index = 15
    while bit >= 16:
        bit -= 16
        variable_index -= 1

    gamestate_values[variable_index] = gamestate_values[variable_index] | (1 << bit)

def process_gamestate_list(dict, gamestate_list, local_list_key, newkey):
    if not local_list_key in dict:
        return
    
    local_list = dict[local_list_key]
    for variable in local_list:
        add_gamestate_to_list(gamestate_list, variable)

    dict[newkey] = [0 for index in range(g_size_constants["MAX_GAME_STATE_VARIABLES"])]
    for variable in local_list:
        i = gamestate_list.index(variable)
        apply_variable(dict[newkey], i)

def process_events(story, event_map, dialog_map, gamestate_list, data_index):
    if not "events" in story:
        return data_index

    for index in range(len(story["events"])):
        event = story["events"][index]
        if "name" in event:
            if len(event["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
                quit("Event name is too long: " + event["name"])

        event_map[event["id"]] = data_index
        data_index += 1

        if "dialog" in event:
            event["dialog_index"] = dialog_map[event["dialog"]]

        process_gamestate_list(event, gamestate_list, "positive_prerequisites", "positive_prerequisites_values")
        process_gamestate_list(event, gamestate_list, "negative_prerequisites", "negative_prerequisites_values")
        process_gamestate_list(event, gamestate_list, "state_changes", "state_changes_values")
        if "positive_prerequisites_values" in event or "negative_prerequisites_values" in event:
            event["use_prerequisites"] = True

    return data_index

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

def process_battlers(story, battler_map, skill_map, imagelist, event_map, data_index):
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
        battler["image_index"] = add_image(imagelist, battler["image"])
        battler["combatantclass_values"] = []
        for stat_name in g_combatant_stats:
            battler["combatantclass_values"].append(g_combatant_ranks[battler["combatantclass"][stat_name]])

        for skill_index in range(len(battler["skill_list"])):
            skill = battler["skill_list"][skill_index]
            skill["index"] = skill_map[skill["id"]]

        if "event" in battler:
            battler["event_index"] = event_map[battler["event"]]
        process_bit_field(battler, "vulnerable", g_damage_types)
        process_bit_field(battler, "resistant", g_damage_types)
        process_bit_field(battler, "immune", g_damage_types)
        process_bit_field(battler, "absorb", g_damage_types)

    return data_index

def process_locations(story, battler_map, imagelist, event_map, gamestate_list, data_index):
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
            location["background_images_index"].append(add_image(imagelist, background_image))
        if "monsters" in location:
            location["monsters_index"] = []
            for monster in location["monsters"]:
                location["monsters_index"].append(battler_map[monster])
        if "initial_event" in location:
            location["initial_event_index"] = event_map[location["initial_event"]]
        if "events" in location:
            location["events_index"] = []
            for event in location["events"]:
                location["events_index"].append(event_map[event])

        process_gamestate_list(location, gamestate_list, "positive_prerequisites", "positive_prerequisites_values")
        process_gamestate_list(location, gamestate_list, "negative_prerequisites", "negative_prerequisites_values")
        if "positive_prerequisites_values" in location or "negative_prerequisites_values" in location:
            location["use_prerequisites"] = True

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

    dialog_map = {}
    data_index = process_dialog(story, dialog_map, data_index)
    
    gamestate_list = []
    event_map = {}
    data_index = process_events(story, event_map, dialog_map, gamestate_list, data_index)

    skill_map = {}
    data_index = process_skills(story, skill_map, data_index)
    
    battler_map = {}
    data_index = process_battlers(story, battler_map, skill_map, imagelist, event_map, data_index)
    
    data_index = process_locations(story, battler_map, imagelist, event_map, gamestate_list, data_index)

    if "classes" in story:
        story["classes_index"] = []
        for battler in story["classes"]:
            story["classes_index"].append(battler_map[battler])
    else:
        quit("Must have at least one class.")

    if "opening_dialog" in story:
        story["opening_dialog_index"] = dialog_map[story["opening_dialog"]]
    if "win_dialog" in story:
        story["win_dialog_index"] = dialog_map[story["win_dialog"]]

def pack_engineinfo(engineinfo):
    binarydata = pack_integer(engineinfo["image_index"]["title_image"])
    binarydata += pack_integer(engineinfo["image_index"]["right_arrow_image"])
    binarydata += pack_integer(engineinfo["image_index"]["left_arrow_image"])
    binarydata += pack_integer(engineinfo["image_index"]["rest_image"])
    binarydata += pack_integer(engineinfo["image_index"]["default_battlefloor"])
    binarydata += pack_integer(engineinfo["image_index"]["engine_repository"])
    binarydata += pack_integer(engineinfo["image_index"]["default_adventure_image"])
    binarydata += pack_integer(engineinfo["tutorial_dialog_index"])
    binarydata += pack_integer(engineinfo["gameover_dialog_index"])
    binarydata += pack_integer(engineinfo["battlewin_dialog_index"])
    binarydata += pack_integer(engineinfo["levelup_dialog_index"])
    binarydata += pack_integer(engineinfo["engine_credits_dialog_index"])
    binarydata += pack_integer(engineinfo["reset_dialog_index"])
    binarydata += pack_integer(engineinfo["exit_dialog_index"])
    return binarydata

def write_engineinfo(engineinfo, datafile):
    write_state = {}
    
    write_state["next_write_location"] = 0
    
    # we always write count first, though it is not clear it is necessary
    count = get_total_objects(engineinfo)
    datafile.write(pack_integer(count))
    # For each object in the file, we store two 16 bit (2 byte) integers, start index and size.
    # With an additional number for count, this gives us the location to start writing actual object data.
    write_state["next_write_location"] = (1 + 2 * count) * 2
    
    # Here, we generate the binary data for the main story object, and write out its size
    write_state["binarydata"] = pack_engineinfo(engineinfo)
    datafile.write(pack_integer(write_state["next_write_location"]))
    datafile.write(pack_integer(len(write_state["binarydata"])))
    write_state["next_write_location"] += len(write_state["binarydata"])
    
    if "dialog" in engineinfo:
        for index in range(len(engineinfo["dialog"])):
            dialog = engineinfo["dialog"][index]
            dialog_binary = pack_dialog(dialog)
            write_data_block(datafile, write_state, dialog_binary)

    # Now that all the index and size data has been written, write out the accumulated data
    datafile.write(write_state["binarydata"])

def process_engineinfo(engineinfo, appinfo, data_objects, imagelist):
    # Process the stories to include. This includes generating the data files,
    # adding them to the appinfo, and storing a list of images used.
    for story_name in engineinfo["stories"]:
        print("Processing story in " + story_name)
        story_filename = "src_data/stories/" + story_name
        story_datafile = "Auto" + os.path.splitext(story_name)[0]+'.dat'
        with open(story_filename) as story_file:
            m = hashlib.md5()
            for line in story_file.readlines():
                m.update(line.encode("ascii"))
            hash = struct.unpack("<H", m.digest()[-2:])
        with open(story_filename) as story_file:
            story = json.load(story_file)
            process_story(story, imagelist)
            with open("resources/data/" + story_datafile, 'wb') as datafile:
                write_story(story, datafile, hash[0])
            newobject = {"file": "data/" + story_datafile, "name": STORY_DATA_STRING + os.path.splitext(story_datafile)[0].upper(), "type": "raw"}
            data_objects.append(newobject)

    # Adds the list of art needed by the engine.
    engineinfo["image_index"] = {}
    for k, v in engineinfo["images"].items():
        engineinfo["image_index"][k] = add_image(imagelist, v)

    data_index = 1 # 0 is reserved for the main story struct
    dialog_map = {}
    data_index = process_dialog(engineinfo, dialog_map, data_index)

    engineinfo["tutorial_dialog_index"] = dialog_map[engineinfo["tutorial_dialog"]]
    engineinfo["gameover_dialog_index"] = dialog_map[engineinfo["gameover_dialog"]]
    engineinfo["battlewin_dialog_index"] = dialog_map[engineinfo["battlewin_dialog"]]
    engineinfo["levelup_dialog_index"] = dialog_map[engineinfo["levelup_dialog"]]
    engineinfo["engine_credits_dialog_index"] = dialog_map[engineinfo["engine_credits_dialog"]]
    engineinfo["reset_dialog_index"] = dialog_map[engineinfo["reset_dialog"]]
    engineinfo["exit_dialog_index"] = dialog_map[engineinfo["exit_dialog"]]

    with open("resources/data/" + "engineinfo.dat", 'wb') as datafile:
        write_engineinfo(engineinfo, datafile)
    newobject = {"file": "data/" + "engineinfo.dat", "name": "ENGINEINFO", "type": "raw"}
    data_objects.append(newobject)

    return add_image(imagelist, engineinfo["icon"])

def write_headers(imagemap, data_objects):
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
                if object["name"][:len(STORY_DATA_STRING)] == STORY_DATA_STRING:
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

def create_appinfo(appinfo, imagelist, imagemap, prefixlist, icon_index):
    # Prep the appinfo for resources
    if not "resources" in appinfo:
        appinfo["resources"] = {};

    if not "media" in appinfo["resources"]:
        appinfo["resources"]["media"] = []

    medialist = appinfo["resources"]["media"]

    # Add all required images to the medialist. Also creates an imagemap so we can map indexes to resource ids at run time. Lastly, creates
    # the list of file prefixes so we can copy/delete the correct files.
    for index in range(len(imagelist)):
        newimage = imagelist[index];
        prefix = os.path.splitext(newimage)[0]
        if prefix.count('/') > 0:
            prefix = prefix[prefix.index('/') + 1:]
        newobject = {"file": newimage, "name": "IMAGE_" + prefix.upper(), "type": "bitmap"}
        if(index == icon_index):
            newobject["menuIcon"] = True
        medialist.append(newobject)
        imagemap.append(newobject["name"])
        prefixlist.append(prefix)

    # Walk the list of story objects and make sure they are in appinfo
    for newobject in data_objects:
        medialist.append(newobject)

    # Write out the new appinfo file
    with open("appinfo.json", 'w') as appinfo_file:
        json.dump(appinfo, appinfo_file, indent = 4, separators=(',', ': '), sort_keys=True)
        appinfo_file.write("\n")

def update_files(appinfo, prefixlist):
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
    dest_data_files = os.listdir(os.getcwd() + "/resources/data")
    for data_file in dest_data_files:
        for object in list(appinfo["resources"]["media"]):
            found = False
            if object["name"] == "ENGINEINFO":
                found = True
                break
            if object["name"][:len(STORY_DATA_STRING)] != STORY_DATA_STRING:
                continue
            if object["file"] == "data/" + data_file:
                found = True
                break
        if not found:
            os.remove("resources/data/" + data_file)

imagelist = []
engineinfo = {}
appinfo = {}
data_objects = []
imagemap = []
prefixlist = []

# Load base-appinfo so we can configure the stories and create the real appinfo with all needed files
with open("src_data/base-appinfo.json") as appinfo_file:
    appinfo = json.load(appinfo_file)

with open("src_data/engineinfo.json") as engineinfo_file:
    engineinfo = json.load(engineinfo_file)

icon_index = process_engineinfo(engineinfo, appinfo, data_objects, imagelist)

create_appinfo(appinfo, imagelist, imagemap, prefixlist, icon_index)

update_files(appinfo, prefixlist)

write_headers(imagemap, data_objects)

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
g_size_constants["MAX_SKILLS_PER_AI_STAGE"] = 10
g_size_constants["MAX_AI_STAGES"] = 4
g_size_constants["MAX_AI_INTERRUPTS"] = 10
g_size_constants["MAX_BATTLE_EVENT_PREREQS"] = 5
g_size_constants["MAX_BATTLE_EVENTS"] = 5
g_size_constants["MAX_STATUS_ICONS"] = 5

g_ai_stage_types = ["sequential", "random"]

g_skill_target = {}
g_skill_target["enemy"] = 0
g_skill_target["self"] = 1
g_skill_target["counter"] = 2

g_skill_properties = {}
g_skill_properties["stun"] = 0
g_skill_properties["poison"] = 1
g_skill_properties["slow"] = 2
g_skill_properties["haste"] = 3
g_skill_properties["silence"] = 4
g_skill_properties["passify"] = 5

g_skill_properties_bits = {k: 1 << v for (k, v) in g_skill_properties.items()}

g_size_constants["MAX_STATUS_EFFECTS"] = len(g_skill_properties)

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

g_battle_event_prereqs = {}
g_battle_event_prereqs["monster_health_below_percent"] = 0
g_battle_event_prereqs["player_health_below_percent"] = 1
g_battle_event_prereqs["time_above"] = 2

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

def pack_integerlist_with_default(dict, list_key, max_size, default, include_count = True):
    if list_key in dict:
        if include_count:
            binarydata = pack_integer(len(dict[list_key]))
        else:
            binarydata = b""
        for index in range(max_size):
            if index < len(dict[list_key]):
                binarydata += pack_integer(dict[list_key][index])
            else:
                binarydata += pack_integer(default)
    else:
        if include_count:
            binarydata = pack_integer(0)
        else:
            binarydata = b""
        for index in range(max_size):
            binarydata += pack_integer(default)
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
    binarydata += pack_string_with_default(location, "menu_name", location["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string_with_default(location, "menu_description", location["name"], g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integerlist_with_default(location, "adjacent_locations_index", g_size_constants["MAX_ADJACENT_LOCATIONS"], 0)
    binarydata += pack_integerlist_with_default(location, "background_images_index", g_size_constants["MAX_BACKGROUND_IMAGES"], 0)
    binarydata += pack_bool_with_default(location, "override_battle_floor", False)
    binarydata += pack_integer_with_default(location, "battle_floor_index", 0)
    binarydata += pack_integer_with_default(location, "location_properties_value", 0)
    binarydata += pack_integer_with_default(location, "length", 0)
    binarydata += pack_integer_with_default(location, "base_level", 0)
    binarydata += pack_integer_with_default(location, "encounter_chance", 0)
    binarydata += pack_integerlist_with_default(location, "monsters_index", g_size_constants["MAX_MONSTERS"], 0)
    binarydata += pack_integer_with_default(location, "initial_event_index", 0)
    binarydata += pack_integerlist_with_default(location, "events_index", g_size_constants["MAX_EVENTS"], 0)

    return binarydata

def pack_skill(skill):
    '''
    Write out all information needed for a skill into a packed binary file
    '''
    binarydata = pack_string(skill["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string(skill["description"], g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integer(skill["target_value"])
    binarydata += pack_integer(skill["speed"])
    binarydata += pack_integer(skill["damage_types_value"])
    binarydata += pack_integer(skill["potency"])
    binarydata += pack_integer(skill["cooldown"])
    binarydata += pack_integer_with_default(skill, "skill_properties_value", 0)
    binarydata += pack_integer_with_default(skill, "property_duration", 0)
    binarydata += pack_integer_with_default(skill, "counter_damage_types_value", 0)
    return binarydata

def pack_dialog(dialog):
    binarydata = pack_string_with_default(dialog, "name", "", g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string(dialog["text"], g_size_constants["MAX_DIALOG_LENGTH"])
    binarydata += pack_bool_with_default(dialog, "allow_cancel", False);
    binarydata += pack_bool(True);
    return binarydata

def pack_gamestate(dict, listkey):
    binarydata = b""
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
    binarydata += pack_string_with_default(event, "menu_description", "", g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_integer_with_default(event, "dialog_index", 0)
    binarydata += pack_bool_with_default(event, "use_prerequisites", False)
    binarydata += pack_gamestate(event, "positive_prerequisites_values")
    binarydata += pack_gamestate(event, "negative_prerequisites_values")
    binarydata += pack_gamestate(event, "positive_state_changes_values")
    binarydata += pack_gamestate(event, "negative_state_changes_values")
    return binarydata

def pack_battle_event(battle_event):
    binarydata = pack_string_with_default(battle_event, "name", "", g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string_with_default(battle_event, "menu_description", "", g_size_constants["MAX_STORY_DESC_LENGTH"])
    binarydata += pack_bool_with_default(battle_event, "automatic", True)
    binarydata += pack_integer_with_default(battle_event, "dialog_index", 0)
    binarydata += pack_integer_with_default(battle_event, "sub_event_index", 0)
    binarydata += pack_integer_with_default(battle_event, "skill_index", 0)
    binarydata += pack_integerlist_with_default(battle_event, "prerequisite_types", g_size_constants["MAX_BATTLE_EVENT_PREREQS"], 0)
    binarydata += pack_integerlist_with_default(battle_event, "prerequisite_values", g_size_constants["MAX_BATTLE_EVENT_PREREQS"], 0, False)
    binarydata += pack_bool_with_default(battle_event, "battler_switch", False)
    binarydata += pack_integer_with_default(battle_event, "new_battler_index", 0)
    binarydata += pack_bool_with_default(battle_event, "full_heal_on_switch", False)
    return binarydata

def pack_battler(battler):
    '''
    Write out all information needed for a battler into a packed binary file
    '''
    binarydata = pack_string(battler["name"], g_size_constants["MAX_STORY_NAME_LENGTH"])
    binarydata += pack_string_with_default(battler, "description", "", g_size_constants["MAX_STORY_DESC_LENGTH"])
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

    binarydata += pack_integerlist_with_default(battler, "battle_events_index", g_size_constants["MAX_BATTLE_EVENTS"], 0)

    binarydata += pack_integer_with_default(battler, "event_index", 0)
    binarydata += pack_integer_with_default(battler, "vulnerable_value", 0)
    binarydata += pack_integer_with_default(battler, "resistant_value", 0)
    binarydata += pack_integer_with_default(battler, "immune_value", 0)
    binarydata += pack_integer_with_default(battler, "absorb_value", 0)
    binarydata += pack_integer_with_default(battler, "status_immunities_value", 0)

    binarydata += pack_integer_with_default(battler, "ai_type_value", g_ai_stage_types.index("sequential"))

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
    binarydata += pack_integerlist_with_default(story, "classes_index", g_size_constants["MAX_CLASSES"], 0)
    binarydata += pack_integer_with_default(story, "opening_dialog_index", 0)
    binarydata += pack_integer_with_default(story, "win_dialog_index", 0)
    binarydata += pack_integer_with_default(story, "credits_dialog_index", 0)
    return binarydata

def get_total_objects(story):
    '''
    Given a story, how many distinct objects will it have?
    This includes locations, monsters, classes, skills, and possibly more
    '''
    count = 1 #main object
    for object_type in object_type_list:
        if object_type in story:
            count += len(story[object_type])
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
    
    pack_object_types(story, datafile, write_state)
    
    # Now that all the index and size data has been written, write out the accumulated data
    datafile.write(write_state["binarydata"])

def process_bit_field(dict, field, global_dict):
    if not field in dict:
        return

    new_field_name = field + "_value"
    dict[new_field_name] = 0
    for bit_type in dict[field]:
        dict[new_field_name] = dict[new_field_name] | global_dict[bit_type]

def process_dialog(dialog):
    if "name" in dialog:
        if len(dialog["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Name is too long: " + dialog["name"])
    
    if len(dialog["text"]) >= g_size_constants["MAX_DIALOG_LENGTH"]:
        quit("Text is too long: " + dialog["text"])

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

def process_event(event):
    if "name" in event:
        if len(event["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Event name is too long: " + event["name"])
        
    if "menu_description" in event:
        if len(event["menu_description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
            quit("Event description is too long: " + event["menu_description"])
    
    if "dialog" in event:
        event["dialog_index"] = object_type_data["dialog"]["map"][event["dialog"]]

    process_gamestate_list(event, gamestate_list, "positive_prerequisites", "positive_prerequisites_values")
    process_gamestate_list(event, gamestate_list, "negative_prerequisites", "negative_prerequisites_values")
    process_gamestate_list(event, gamestate_list, "positive_state_changes", "positive_state_changes_values")
    process_gamestate_list(event, gamestate_list, "negative_state_changes", "negative_state_changes_values")
    if "positive_prerequisites_values" in event or "negative_prerequisites_values" in event:
        event["use_prerequisites"] = True

def process_battle_event(battle_event):
    if "name" in battle_event:
        if len(battle_event["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Event name is too long: " + battle_event["name"])
        
    if "menu_description" in battle_event:
        if len(battle_event["menu_description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
            quit("Event description is too long: " + battle_event["menu_description"])
    
    if "dialog" in battle_event:
        battle_event["dialog_index"] = object_type_data["dialog"]["map"][battle_event["dialog"]]

    if "new_battler" in battle_event:
        battle_event["new_battler_index"] = object_type_data["battlers"]["map"][battle_event["new_battler"]]

    if "sub_event" in battle_event:
        battle_event["sub_event_index"] = object_type_data["events"]["map"][battle_event["sub_event"]]

    if "skill" in battle_event:
        battle_event["skill_index"] = object_type_data["skills"]["map"][battle_event["skill"]]

    # Prerequisites should be a dictionary. We then turn it into a pair of arrays at packing time.
    if "prerequisites" in battle_event:
        if len(battle_event["prerequisites"]) > g_size_constants["MAX_BATTLE_EVENT_PREREQS"]:
            quit("Too many battle prerequisites")
        battle_event["prerequisite_count"] = len(battle_event["prerequisites"])
        battle_event["prerequisite_types"] = []
        battle_event["prerequisite_values"] = []
        for k, v in battle_event["prerequisites"].items():
            if not g_battle_event_prereqs.has_key(k):
                 quit("Invalid battle event prerequisite " + k)
            battle_event["prerequisite_types"].append(g_battle_event_prereqs[k])
            battle_event["prerequisite_values"].append(v)

def process_skill(skill):
    if len(skill["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
        quit("Name is too long: " + skill["name"])
    if len(skill["description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
        quit("Description is too long: " + skill["description"])

    skill["target_value"] = g_skill_target[skill["target"]]
    process_bit_field(skill, "damage_types", g_damage_types)
    process_bit_field(skill, "counter_damage_types", g_damage_types)
    process_bit_field(skill, "skill_properties", g_skill_properties_bits)

def process_battler(battler):
    if len(battler["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
        quit("Name is too long: " + battler["name"])
    if "description" in battler and len(battler["description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
        quit("Description is too long: " + battler["description"])
    if len(battler["skill_list"]) > g_size_constants["MAX_SKILLS_IN_LIST"]:
        quit("Too many skills for " + battler["name"])

    battler["image_index"] = add_image(imagelist, battler["image"])
    battler["combatantclass_values"] = []
    for stat_name in g_combatant_stats:
        battler["combatantclass_values"].append(g_combatant_ranks[battler["combatantclass"][stat_name]])

    for skill_index in range(len(battler["skill_list"])):
        skill = battler["skill_list"][skill_index]
        skill["index"] = object_type_data["skills"]["map"][skill["id"]]

    if "event" in battler:
        battler["event_index"] = object_type_data["events"]["map"][battler["event"]]

    if "battle_events" in battler:
        if len(battler["battle_events"]) > g_size_constants["MAX_BATTLE_EVENTS"]:
            quit("Too many battle events on " + battler["name"])

        battler["battle_events_index"] = []
        for battle_event in battler["battle_events"]:
            battler["battle_events_index"].append(object_type_data["battle_events"]["map"][battle_event])


    if "ai_type" in battler:
        if g_ai_stage_types.count(battler["ai_type"]) == 0:
            quit("Invalid ai type " + battler["ai_type"])
        battler["ai_type_value"] = g_ai_stage_types.index(battler["ai_type"])

    process_bit_field(battler, "vulnerable", g_damage_types)
    process_bit_field(battler, "resistant", g_damage_types)
    process_bit_field(battler, "immune", g_damage_types)
    process_bit_field(battler, "absorb", g_damage_types)
    process_bit_field(battler, "status_immunities", g_skill_properties_bits)

def process_location(location):
    if len(location["name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
        quit("Name is too long: " + location["name"])
    if "menu_name" in location:
        if len(location["menu_name"]) >= g_size_constants["MAX_STORY_NAME_LENGTH"]:
            quit("Menu name is too long: " + location["menu_name"])
    if "menu_description" in location:
        if len(location["menu_description"]) >= g_size_constants["MAX_STORY_DESC_LENGTH"]:
            quit("Menu description is too long: " + location["menu_description"])
    if len(location["adjacent_locations"]) > g_size_constants["MAX_ADJACENT_LOCATIONS"]:
        quit("Too many adjacent locations for " + location["name"])
    if len(location["background_images"]) > g_size_constants["MAX_BACKGROUND_IMAGES"]:
        quit("Too many background images for " + location["name"])
    if "monsters" in location and len(location["monsters"]) > g_size_constants["MAX_MONSTERS"]:
        quit("Too many monsters for " + location["name"])

    process_bit_field(location, "location_properties", g_location_properties)

    location["background_images_index"] = []
    for background_image in location["background_images"]:
        location["background_images_index"].append(add_image(imagelist, background_image))
    if "battle_floor" in location:
        location["battle_floor_index"] = add_image(imagelist, location["battle_floor"])
        location["override_battle_floor"] = True
    if "monsters" in location:
        location["monsters_index"] = []
        for monster in location["monsters"]:
            location["monsters_index"].append(object_type_data["battlers"]["map"][monster])
    if "initial_event" in location:
        location["initial_event_index"] = object_type_data["events"]["map"][location["initial_event"]]
    if "events" in location:
        location["events_index"] = []
        for event in location["events"]:
            location["events_index"].append(object_type_data["events"]["map"][event])

    process_gamestate_list(location, gamestate_list, "positive_prerequisites", "positive_prerequisites_values")
    process_gamestate_list(location, gamestate_list, "negative_prerequisites", "negative_prerequisites_values")
    if "positive_prerequisites_values" in location or "negative_prerequisites_values" in location:
        location["use_prerequisites"] = True

    location["adjacent_locations_index"] = []
    for adjacent in location["adjacent_locations"]:
        location["adjacent_locations_index"].append(object_type_data["locations"]["map"][adjacent])

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
        floor_word = "Floor"
        if "floor_word" in dungeon:
            floor_word = dungeon["floor_word"]
        traversal_word = "Traverse"
        if "traversal_word" in dungeon:
            traversal_word = dungeon["traversal_word"]
        idlist = []
        namelist = []
        menu_namelist = []
        menu_descriptionlist = []
        floors = dungeon["floors"]
        for floor in range(floors):
            if floor == 0:
                idsuffix = "_start"
            elif floor == floors - 1:
                idsuffix = "_end"
            else:
                idsuffix = "_" + str(floor + 1)
            idlist.append(dungeon["id"] + idsuffix)
            namelist.append(dungeon["name"] + " " + floor_word + " " + str(floor + 1))
            menu_namelist.append(floor_word + " " + str(floor + 1))
            menu_descriptionlist.append(traversal_word + " " + floor_word + " " + str(floor + 1))
            if floor < floors - 1:
                idlist.append(dungeon["id"] + idsuffix + "_end")
                namelist.append(dungeon["name"] + " " + floor_word + " " + str(floor + 1) + " End")
                menu_namelist.append(floor_word + " " + str(floor + 1) + " End")
                menu_descriptionlist.append("The end of " + floor_word + " " + str(floor + 1))
        
        for index in range(len(idlist)):
            location = {}
            location["id"] = idlist[index]
            location["name"] = namelist[index]
            location["menu_name"] = menu_namelist[index]
            location["menu_description"] = menu_descriptionlist[index]
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

def process_external_files(story, file_list_key, m):
    if file_list_key in story:
        for filename in story[file_list_key]:
            print("\tProcessing " + filename)
            with open("src_data/stories/" + filename) as object_file:
                for line in object_file.readlines():
                    m.update(line.encode("ascii"))
            with open("src_data/stories/" + filename) as object_file:
                object_list = json.load(object_file)
                for k, v in object_list.items():
                    if not k in story:
                        story[k] = []
                    for newobject in object_list[k]:
                        for oldobject in story[k]:
                            if newobject["id"] == oldobject["id"]:
                                quit("Duplicate id, " + oldobject["id"] + " in list of " + object_key)
                    story[k].extend(object_list[k])

gamestate_list = []
object_type_list = ["dialog", "events", "battle_events", "skills", "battlers", "locations"]

object_type_data = {}
object_type_data["dialog"] = {"process": process_dialog, "pack": pack_dialog, "map": {}}
object_type_data["events"] = {"process": process_event, "pack": pack_event, "map": {}}
object_type_data["battle_events"] = {"process": process_battle_event, "pack": pack_battle_event, "map": {}}
object_type_data["skills"] = {"process": process_skill, "pack": pack_skill, "map": {}}
object_type_data["battlers"] = {"process": process_battler, "pack": pack_battler, "map": {}}
object_type_data["locations"] = {"process": process_location, "pack": pack_location, "map": {}}

def assign_object_indexes(story):
    data_index = 1
    for object_type in object_type_list:
        object_type_data[object_type]["map"] = {}
        if object_type in story:
            for object in story[object_type]:
                if object_type_data[object_type]["map"].has_key(object["id"]):
                    quit("Duplicate id " + object["id"] + " for " + object_type + ".")
                object_type_data[object_type]["map"][object["id"]] = data_index
                data_index += 1

def process_object_types(story):
    for object_type in object_type_list:
        if object_type in story:
            for object in story[object_type]:
                object_type_data[object_type]["process"](object)

def pack_object_types(story, datafile, write_state):
    for object_type in object_type_list:
        if object_type in story:
            for object in story[object_type]:
                binary_data = object_type_data[object_type]["pack"](object)
                write_data_block(datafile, write_state, binary_data)

def process_story(story, m):
    '''
    Here we prepare the story for being written to a packed binary file.
    We have to turn each reference to an object into what will become the 
    index of that object. In addition, we generate a map for image resource references.
    '''
    
    # In order to allow stories to share skills and battlers, allow the user to
    # have a list of external files to include. process_external_files appends
    # the contents into the appropriate lists.
    process_external_files(story, "external_files", m)
    
    # This just unrolls the dungeon definitions into the appropriate
    # number of locations. Actual writing to the file happens when
    # we process the rest of the locations.
    process_dungeons(story)

    #processing data objects assigns them an index in the file. They must be
    # written to the file in the same order they were processed.
    assign_object_indexes(story)
    process_object_types(story)

    story["start_location_index"] = object_type_data["locations"]["map"][story["start_location"]]

    if "classes" in story:
        story["classes_index"] = []
        for battler in story["classes"]:
            story["classes_index"].append(object_type_data["battlers"]["map"][battler])
    else:
        quit("Must have at least one class.")

    if "opening_dialog" in story:
        story["opening_dialog_index"] = object_type_data["dialog"]["map"][story["opening_dialog"]]
    if "win_dialog" in story:
        story["win_dialog_index"] = object_type_data["dialog"]["map"][story["win_dialog"]]
    if "credits_dialog" in story:
        story["credits_dialog_index"] = object_type_data["dialog"]["map"][story["credits_dialog"]]

def pack_engineinfo(engineinfo):
    binarydata = pack_integer(engineinfo["image_index"]["title_image"])
    binarydata += pack_integer(engineinfo["image_index"]["right_arrow_image"])
    binarydata += pack_integer(engineinfo["image_index"]["left_arrow_image"])
    binarydata += pack_integer(engineinfo["image_index"]["rest_image"])
    print(engineinfo["image_index"]["default_battlefloor"])
    binarydata += pack_integer(engineinfo["image_index"]["default_battlefloor"])
    print(engineinfo["image_index"]["engine_repository"])
    binarydata += pack_integer(engineinfo["image_index"]["engine_repository"])
    binarydata += pack_integer(engineinfo["image_index"]["default_adventure_image"])
    binarydata += pack_integer(engineinfo["tutorial_dialog_index"])
    binarydata += pack_integer(engineinfo["gameover_dialog_index"])
    binarydata += pack_integer(engineinfo["battlewin_dialog_index"])
    binarydata += pack_integer(engineinfo["levelup_dialog_index"])
    binarydata += pack_integer(engineinfo["engine_credits_dialog_index"])
    binarydata += pack_integer(engineinfo["reset_dialog_index"])
    binarydata += pack_integer(engineinfo["exit_dialog_index"])
    binarydata += pack_integerlist_with_default(engineinfo, "status_icon_index", len(g_skill_properties), 0)
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

def process_engineinfo(engineinfo, appinfo, data_objects): #TODO: This needs to be updated to match the way storydoes things
    # Process the stories to include. This includes generating the data files,
    # adding them to the appinfo, and storing a list of images used.
    
    story_map = {}
    
    for story_name in engineinfo["stories"]:
        print("Processing story in " + story_name)
        story_filename = "src_data/stories/" + story_name
        story_datafile = "Auto" + os.path.splitext(story_name)[0]+'.dat'
        m = hashlib.md5()
        with open(story_filename) as story_file:
            for line in story_file.readlines():
                m.update(line.encode("ascii"))
        with open(story_filename) as story_file:
            story = json.load(story_file)
            if story["id"] in story_map:
                quit("Two stories with the same id: " + story_map[story["id"]] + ", " + story["name"])
            else:
                story_map[story["id"]] = story["name"]
            process_story(story, m)
            with open("resources/data/" + story_datafile, 'wb') as datafile:
                hash = struct.unpack("<H", m.digest()[-2:])
                write_story(story, datafile, hash[0])
            newobject = {"file": "data/" + story_datafile, "name": STORY_DATA_STRING + os.path.splitext(story_datafile)[0].upper(), "type": "raw"}
            data_objects.append(newobject)

    # Adds the list of art needed by the engine.
    engineinfo["image_index"] = {}
    for k, v in engineinfo["images"].items():
        engineinfo["image_index"][k] = add_image(imagelist, v)

    assign_object_indexes(engineinfo)
    process_object_types(engineinfo)

    engineinfo["tutorial_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["tutorial_dialog"]]
    engineinfo["gameover_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["gameover_dialog"]]
    engineinfo["battlewin_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["battlewin_dialog"]]
    engineinfo["levelup_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["levelup_dialog"]]
    engineinfo["engine_credits_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["engine_credits_dialog"]]
    engineinfo["reset_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["reset_dialog"]]
    engineinfo["exit_dialog_index"] = object_type_data["dialog"]["map"][engineinfo["exit_dialog"]]

    engineinfo["status_icon_index"] = [0 for i in range(len(g_skill_properties))]
    for k, v in engineinfo["status_icons"].items():
        engineinfo["status_icon_index"][g_skill_properties[k]] = add_image(imagelist, v)

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
        for k, v in g_skill_target.items():
            skill_file.write("#define SKILL_TARGET_" + k.upper() + " " + str(v) + "\n")
        skill_file.write("\n")

        for k, v in g_damage_types.items():
            skill_file.write("#define DAMAGE_TYPE_" + k.upper() + " " + str(v) + "\n")
        skill_file.write("\n")

        for k, v in g_skill_properties.items():
            skill_file.write("#define STATUS_EFFECT_" + k.upper() + " " + str(v) + "\n")
        skill_file.write("#define STATUS_EFFECT_COUNT " + str(len(g_skill_properties)) + "\n")
        skill_file.write("\n")

        for k, v in g_skill_properties_bits.items():
            skill_file.write("#define SKILL_PROPERTIES_" + k.upper() + " " + str(v) + "\n")
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

    with open("src/AutoAIConstants.h", 'w') as ai_file:
        ai_file.write("#pragma once\n\n")
        for index in range(len(g_ai_stage_types)):
            type = g_ai_stage_types[index]
            ai_file.write("#define AI_STAGE_TYPE_" + type.upper() + " " + str(index) + "\n")

        ai_file.write("\n")

    with open("src/AutoBattleEventConstants.h", 'w') as battle_event_file:
        battle_event_file.write("#pragma once\n\n")
        for k, v in g_battle_event_prereqs.items():
            battle_event_file.write("#define BATTLE_EVENT_TYPE_" + k.upper() + " " + str(v) + "\n")

        battle_event_file.write("\n")

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

icon_index = process_engineinfo(engineinfo, appinfo, data_objects)

create_appinfo(appinfo, imagelist, imagemap, prefixlist, icon_index)

update_files(appinfo, prefixlist)

write_headers(imagemap, data_objects)

#MiniAdventure

This is an extension of MiniDungeon (https://github.com/Torivon/MiniDungeon). It started from the same core engine to support a broader game. 

The game is broken up into stories. From the title screen you can choose which story to explore. At the moment, each story has its own locations, monsters, character classes, and skills. 


##Story files

A story file is a json file which describese all of the attributes of the story. Adding a new story simply involves creating the appropriate json file and including it in the list in stories.txt. The wscript file has been modified to automatically process the stories and include them in the build.

###Main Story Object

* "id": <integer> # This needs to be unique across all stories included. It is used to index persisted storage.
* "version": <integer> # Should be updated whenever the story fundamentally changes. This is used to invalidate existing persisted storage.
* "name": <string>
* "description": <string>
* "start_location": <string> # the id of a location in the file
* "xp_monsters_per_level": <integer> # The engine uses a simplified experience scale. xp_monsters_per_level represents the number of monsters of equal level that it takes to level up. Fighting higher level monsters will give more experience, while lower level monsters will give less. An xp_monsters_per_level of 0 means no leveling up.
* "xp_difference_scale": <integer> # an integer percent for the bonus or penalty in experience gains per level difference. With an xp_difference_scale of 20, if you defeat a monster 3 levels higher, you will gain a 60% experience bonus. Leaving this out or setting it to 0 gives no bonus or penalty.
* "classes": <list> <string> # the ids of all allowed classes for the story. These must be valid battler ids.
* "locations": <list> <location> # This is a list of location objects to which the player can travel
* "dungeons": <list> <dungeon> # A list of dungeons. At processing, each dungeon is turned into a list of locations.
* "skills": <list> <skill> # A list of the skills available to monsters int the story
* "battlers": <list> <battler> # A list of battlers available to the locations, and available as classes.

###Location

* "id": <string> # unique identifier for the location. During processing, these are turned into indexes directly into the processed file.
* "name": <string>
* "adjacent_locations": <list> <string> # This is a list of location ids to which one can move from the given location. A location with a length bigger than 0, will only use the first two entries.
* "background_images": <list> <string> # This is a list of filenames for images that can be shown while traversing the location. For the moment, these files must already be included in the appinfo.json file.
* "length": <integer> # The number of minutes it takes to journey across the location. If this is zero, the player is simply given a menu of new locations to which to travel.
* "monsters": <list> <string> # These strings are ids for monsters included in the story. For a location with length, the chance of an encounter in any given minute are given by dungeon["ecounter_chance"]. For a location with length == 0, a monster on the list will be encountered upon entering the location.
* "encounter_chance": <integer> # Determines the likelihood in any given minute of encountering a monster.
* "base_level": <integer> # Determines the level of monsters encountered in the location

###Dungeon

A dungeon will be unrolled into a series of locations. These locations alternate between paths with length > 0, and fixed locations with length == 0. The properties of the dungeon become the properties of the locations as appropriate.

* "id": <string> # unique identifier for the dungeon. When unrolled, the endpoints of the dungeon are locations with ids "id" + "_start" and "id" + "_end". These should be used by other locations to connect to the dungeon.
* "name": <string>
* "adjacent_locations": <list> <string> # These are location ids. Like a location with length > 0, a dungeon will only make use of the first two as the end points.
* "background_images": <list> <string> # As with locations, these are filenames for images. This is the list used for paths.
* "fixed_background_image": <list> <string> # The background image used for fixed locations between dungeon floors.
* "length": <integer> # The length of each dungeon floor
* "base_level": <integer> # The starting level of monsters in the dungeon
* "level_rate": <integer> # The number of floors after which the level of monsters will increase by one
* "encounter_chance": <integer> # The chance of an encounter on dungeon floors.
* "monsters": <list> <string> # The ids of the monsters to be seen in the dungeon. These will only show up on the floors with length > 0, and not on the fixed locations.
* "monster_scaling": <integer> # The number of floors between adding a new monster to the mix. If this is 0, all monsters are available on all floors.

###Skill

* "id": <string> # unique identifier for the skill
* "name": <string>
* "description": <string>
* "type": <string> # Must be from the list g_skill_types at the top of process_stories.py. Will be turned into integers during processing. This determines how the skill behaves.
    g_skill_types["attack"] = 0
    g_skill_types["heal"] = 1
    g_skill_types["counter"] = 2
    g_skill_types["buff"] = 3
    g_skill_types["debuff"] = 4
* "speed": <integer> # How quickly the skill executes after being chosen.
* "damage_types": <list> <string> # These strings must be from g_damage_types at the top of process_stories.py. This will be turned into a bitfield during processing.
    g_damage_types["physical"] = 1 << 0
    g_damage_types["magic"] = 1 << 1
    g_damage_types["fire"] = 1 << 2
    g_damage_types["ice"] = 1 << 3
    g_damage_types["lightning"] = 1 << 4
    g_damage_types["slashing"] = 1 << 5
    g_damage_types["piercing"] = 1 << 6
    g_damage_types["bludgeoning"] = 1 << 7
* "potency": <integer> # How strong the skill is.
* "cooldown": <integer> # How many combat rounds must pass before the skill is ready to be used again

###Battler

* "id": <string> # unique identifier for the battler. This should be included in the monsters list of locations and dungeons. They can also be used as the classes for the player character.
* "name" <string>
* "description" <string>
* "image" <string> # The image used for the monster during combat. Must already by included in appinfo.json.
* "combatantclass": <dict> # This dict has keys from g_combatant_stats and values from g_combatant_ranks. All will be turned into integer values in processing
    g_combatant_stats = ["strength", "magic", "defense", "magic_defense", "speed", "health"]
    g_combatant_ranks["rankf"] = 0
    g_combatant_ranks["ranke"] = 1
    g_combatant_ranks["rankd"] = 2
    g_combatant_ranks["rankc"] = 3
    g_combatant_ranks["rankb"] = 4
    g_combatant_ranks["ranka"] = 5
    g_combatant_ranks["ranks"] = 6
* "skill_list": <list> <skillentry> # Each skill entry must have the id of a skill that is defined in the file, as well as the minimum level for the monster to have access to it. This way more powerful monsters and players get access to a wider variety of skills.

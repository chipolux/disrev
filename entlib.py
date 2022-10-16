"""
Utility module for processing .entities resources.
"""
from enum import Enum
from io import BytesIO

# example entity definition
# <definition-type> {
#     <entity-type> <entity-identifier> {
#         <key> = <value>
#     }
# }

# valid whitespace \n \r \t \s
# valid key/identifier contains letters, numbers, and underscores
# values can be true|false, strings "|', NULL, int, float, and numbers with units
# values can also be arbitrarily nested groups, keys seem to always be unique
# values that are not groups always end with a ;
# lists are represented by a group that contains a "num" key and a matching
#   number of "item[n]" keys for the items
# matrices are represented by groups that contain only "mat[n]" keys, they seem
#   to support only partial values, not sure if n represents row or column, used
#   for rotation fields and likely represent quaternions

WHITESPACE = b" \t\r\n="
FRAME_CHANGE = b"{}"
STRING_CHANGE = b"\"'"
VALUE_END = ord(";")
ESCAPE = ord("\\")


def load_entities(path):
    if isinstance(path, (bytes, bytearray)):
        return _parse_entities(BytesIO(path))
    with open(path, "rb") as f:
        return _parse_entities(f)


def _parse_entities(f):
    header = f.read(10)
    assert header == b"Version 6\n", "Invalid data, unrecognized header!"
    buff = f.read(1024)
    escape_next_char = False
    in_string = False
    in_value = False
    stack = []
    frame = 0
    tokens = []
    token = []
    entities = {}
    count = 0
    while buff:
        for c in buff:
            # early exit if we know we are in an escape sequence
            if escape_next_char:
                token.append(c)
                escape_next_char = False
                continue
            # early exit if we are starting an escape sequence
            if not escape_next_char and c == ESCAPE:
                escape_next_char = True
                continue
            # handle string context change
            if c in STRING_CHANGE:
                token.append(c)
                in_string = not in_string
                continue
            # handle nest frame change
            if not in_string and c in FRAME_CHANGE:
                direction = 124 - c
                if direction == 1 and frame == 0:
                    stack.append({"definition_type": tokens.pop().decode("utf-8")})
                elif direction == 1 and frame == 1:
                    stack[-1]["entity_id"] = tokens.pop().decode("utf-8")
                    stack[-1]["entity_type"] = tokens.pop().decode("utf-8")
                elif direction == 1:
                    stack.append({})
                elif direction == -1 and frame == 2:
                    pass
                elif direction == -1 and frame == 1:
                    key = stack[-1]["entity_id"]
                    entities[key] = stack.pop()
                    count += 1
                elif direction == -1:
                    key = tokens.pop().decode("utf-8")
                    value = stack.pop()
                    stack[-1][key] = value
                frame += direction
                continue
            # handle last two tokens as key/value pair
            if not in_string and c == VALUE_END:
                key = tokens.pop().decode("utf-8")
                value = _parse_value(token)
                token.clear()
                stack[-1][key] = value
                continue
            if not in_string and c in WHITESPACE:
                if token:
                    tokens.append(bytes(token))
                    token.clear()
                continue
            token.append(c)
        buff = f.read(1024)
    assert frame == 0, f"Invalid data, left on frame {frame} with {tokens}, {stack}!"
    assert count == len(entities), "Invalid data, duplicate keys detected!"
    return entities


def _parse_value(token):
    c = token[0]
    if c == 0x4E:  # NULL
        return None
    if c == 0x74:  # true
        return True
    if c == 0x66:  # false
        return False
    if c in STRING_CHANGE:
        token = bytes(token)
        try:
            return token.decode("utf-8")
        except UnicodeDecodeError:
            return token
    # NOTE: some fields use values with units (200m, 2.5km) so leave them alone
    return bytes(token).decode("utf-8")


def spawn_pos(ent):
    return ent.get("edit", {}).get("spawnPosition")


def are_nearby(pos1, pos2, distance):
    """by taxicab distance, not real (slow) distance"""
    if pos1 is None or pos2 is None:
        return False
    x1, y1, z1 = float(pos1["x"]), float(pos1["y"]), float(pos1["z"])
    x2, y2, z2 = float(pos2["x"]), float(pos2["y"]), float(pos2["z"])
    xd, yd, zd = (x1 - x2) ** 2, (y1 - y2) ** 2, (z1 - z2) ** 2
    d2 = distance**2
    c2 = xd + yd + zd
    return c2 <= d2


def filter_nearby(ent, ents, distance=2):
    nearby = []
    pos1 = spawn_pos(ent)
    if pos1 is None:
        return nearby
    for e in ents.values():
        if are_nearby(pos1, spawn_pos(e), distance):
            nearby.append(e)
    return nearby

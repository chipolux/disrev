# .entities Files

The gist of the format is a header (always `Version 6`) and then a series of
entity definitions. The format of the definitions is similar to Steam's VDF files.

Here is a pseudo-syntax-digaram for the definitions:

```
<definition-type> {
    <entity-type> <entity-identifier> {
        <key> = <value>
    }
}
```

Everything other than `<value>` must be a valid token that would match `[a-zA-Z0-9_]+`
except for a few special cases where they can also include `[` and `]`.

Definition types are only `component` or `entity`.

There are a ton of entity type markers for `components` but only `entityDef` for
`entity` definitions.

Entity identifier's are unique per `.entities` file.

Keys are just keys, often things that are obvious class member names like `m_version`
but some that are probably special like `class`, `inherit`. I just treat 'em like
dictionary keys, they are unique within a scope.

Values are special, they can be any of these basic types:

* strings (`"` or `'` wrapped with `\` for escapes)
* true or false
* NULL
* integers or floats (potentially marked with units like `200m` or `0.2km`)

Or the most common and complex version of a value, a nested scope like this:

```
edit = {
    inherit = "worldspawn";
    class = "idWorldspawn";
    spawnPosition = {
        x = 6.9;
        y = 42.03333333;
        z = -7;
    }
}
```

There can be very deeply nested.

Finally I have also seen a few types of special value, they are identical to the nested
scope but use that special case for keys. Here is an example for what I think are matrices:

```
spawnOrientation = {
    mat = {
        mat[0] = {
            x = -0.9700315595;
            y = 0.2429793626;
        }
        mat[1] = {
            x = -0.2429793328;
            y = -0.9700314403;
        }
    }
}
```

An example for lists:

```
m_connectionPoints = {
    num = 2;
    item[0] = {
        m_id = 275855564;
        m_name = "LevelLoadedFirstTime";
    }
    item[1] = {
        m_id = 384100613;
        m_name = "NewGameStarted";
    }
}
```

A possible enum:

```
m_playerDefs = {
    enumItem[PLAYER_UNDEFINED] = NULL;
    enumItem[PLAYER_EMILY] = "models/characters/player/emily/body_tutorial.def";
    enumItem[PLAYER_CORVO] = NULL;
}
```

And something weird, maybe a kind of dictionary/map:

```
m_componentDecls = {
    item_begin = true;
    item_add["MJR_l1"] = {
        inheritedDecl = "models/winds/generic/wind_disabled.cpntwind";
    }
    item_add["atvCQ1"] = {
        mapComponentDecl = "map_cpntaudio_decl_1";
    }
    item_add["lJUhP3"] = {
        mapComponentDecl = "map_cpntpatrolsquadtree_decl_1";
    }
}
```

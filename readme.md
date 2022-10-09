# Dishonored 2 Reverse Engineering

Deconstructing the Dishonored 2 assets with the end goal of creating custom levels.

At this stage you can use the `resources.py` script to extract the assets from
the game data. It doesn't do anything automatically, but you can use the
`ResourceContainer` class within to parse the indexes and then call the `.export()`
method on the loaded `ResourceEntry` objects to export individual assets. See
the `test_export()` function for 90% of that.

**Roadmap:**
- [X] Extract 3D models, level definitions, and other assets.
- [ ] Modify plaintext assets and repack. (subtitles, etc.)
- [ ] Interpret custom asset data formats. (bimage7, bwm, etc.)
- [ ] Modify binary assets and repack. (move wall/table/chair or something)

<details>
<summary><h3>Installation Directory</h3></summary>

Here is an example installation directory:

```
Dishonored2/
└───base/
    ├───cfg/
    ├───pck/
    ├───shaderCache/
    ├───video/
    ├───game1.index
    ├───game1.resources
    ├───game1_001.index
    ├───game1_001.resources
    ~~~~
    ├───game1_005.index
    ├───game1_005.resources
    ├───game1_patch.index
    ├───game1_patch.resources
    ~~~~
    ├───game4_patch.index
    ├───game4_patch.resources
    ├───master.index
    ├───shared_2_3.sharedrsc
    ~~~~
```

The files in the `pck` and `video` directories seem to be sound effect, voice line,
and video assets which would be dependent on localization so are likely delivered
in separate per region depots. So we are mostly concerned with the `.index`,
`.resources`, and `.sharedrsc` files.
</details>

<details>
<summary><h3>gameN Containers</h3></summary>

I'm not sure what the significance of each `gameN` group is, they seem to just
contain chunks of the game in an order that roughly matches a normal playthrough.

I call these groups with matching `gameN` prefixes "containers".

Within each container it seems you can have up to 1000 patch levels, `_001` thru
`_999` and a final `_patch` entry. In reality each container always has `_001`
thru `_005` and a final `_patch`.

Each level appears to supersede the previous levels. For example, all of the
entries in `game1.index` are present in `game1_001.index` with only minor changes
and a few additions.

It does seem like later patch levels can de-list entries. Leaving chunks of some
`.resources` files orphaned.

But `.resources` files themselves never go unused! Later patch levels always
reference data in the earlier `.resources`. Though an entry in a later patch level
can point the same "destination" asset to a different `.resources` file, this
should be obvious as a way to let devs bundle new versions of textures/models/etc.
without breaking old builds or modifying files in place once they have been shipped.

Interestingly there is nothing stored in the `game4` container!

Oh, and entries in an index never reference `.resources` in different container
aside from the special shared resource file.
</details>

<details>
<summary><h3>.index Files</h3></summary>

Each `.index` file can be at most `uint32 - 32` bytes long.

***Note:*** All values are big-endian unless noted.
Here is a pseudo-code representation of the format:

```
struct Index {
    uint8     // Type Indicator (always 0x05)
    char[3]   // Format Indicator (always "SER")
    uint32    // Size Of Index (minus 32 bytes for this header section)
    uint8[24] // Padding? (always seems to be filled with null bytes)
    uint32    // Count Of Entries
    Entry[N]  // List Of Entries (exactly matching count)
}

struct Entry {
    uint32    // ID Of Entry (seems to follow position in index, but not always)
    uint32    // Little-Endian Type String Size
    char[N]   // Type String (seems to be a type identifier)
    uint32    // Little-Endian Source String Size
    char[N]   // Source String (seems to be a source name, pre-build file/variable)
    uint32    // Little-Endian Destination String Size
    char[N]   // Destination String (seems to be the destination name, built asset)
    uint64    // Resource File Byte Offset
    uint32    // Actual Byte Size
    uint32    // Packed Byte Size
    uint16    // Flags
    uint16    // Flags
}
```

You will see how the resource offset, actual, and packed byte sizes are used in
the section on `.resources` files.

The flags are interesting. If you shift the second set right by 2 bits it refers
to which `.resources` file the data is stored in, so if you see `0` that is
`gameN.resources`, `1` is `gameN_001.resources`, and on with `6` being
`gameN_patch.resources`. You can verify this by seeing that the indexes will
only have flags set for their own patch level and previous patch levels.

One exception is if the top bit is set in the second set of flags. This seems to
indicate that the data is stored in `shared_2_3.sharedrsc`. No idea why, maybe
this was done to avoid some kind of max open files limit for specific platforms.
In any case the data does seem to be in there and correct!

No idea what the first set of flags means, sometimes you see 32 in there!
</details>

<details>
<summary><h3>.resources And .sharedrsc Files</h3></summary>

Each `.resources` file can be at most `uint64 - 4` bytes long and `.sharedrsc`
files are identical.

The format here is like the index but with basically no metadata other than a
4 byte header, `04 53 45 52`. Always the `0x04` followed by the `"SER"` string
just like in the index!

The rest is just raw data, we use the index to interpret it!

Each entry in the index contains flags that tell you which `.resources` file to
use (within the same container). And within that resource file you simply grab
the packed number of bytes starting from the byte offset.

If the actual size recorded in the index entry is different (larger) than the
packed size then we need to decompress the bytes we just read using zlib.

Once that's done you should have a chunk of data that exactly matches the actual
size and can be written out or viewed however you like.

There are a ton of files packed in that are plaintext (shader definitions, subtitles,
parameters), but the vast majority of the data is binary files, I will document
their formats as I interpret them.
</details>

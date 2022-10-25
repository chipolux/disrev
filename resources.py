import os
import struct
import zlib

import entlib


class ResourceContainer:
    def __init__(self, path, explicit=False):
        """
        path = full path to <some>.index or <some>.resources file
               the game<id> will be extracted and used to build an
               entry list and accessors for the resources files.
        explicit = force the use of the explicitly specified path as the index
        """
        path = os.path.abspath(path)
        self.dir = os.path.dirname(path)
        self.name = os.path.basename(path)[:5]
        assert self.name.startswith("game"), f"Invalid container: {path}"

        # full paths and sizes with id as key
        self.idx_id = -1  # indexes get overriden with later versions
        self.idx_cache = {
            0: [os.path.join(self.dir, f"{self.name}.index"), 0],
            1: [os.path.join(self.dir, f"{self.name}_001.index"), 0],
            2: [os.path.join(self.dir, f"{self.name}_002.index"), 0],
            3: [os.path.join(self.dir, f"{self.name}_003.index"), 0],
            4: [os.path.join(self.dir, f"{self.name}_004.index"), 0],
            5: [os.path.join(self.dir, f"{self.name}_005.index"), 0],
            6: [os.path.join(self.dir, f"{self.name}_patch.index"), 0],
        }
        for k, v in self.idx_cache.items():
            if os.path.exists(v[0]):
                v[1] = os.stat(v[0]).st_size
                if not explicit:
                    self.idx_id = k  # use the latest index that exists
                elif v[0] == path:
                    self.idx_id = k  # use the exact index specified in path
        assert self.idx_id != -1, f"Failed to find index: {path}"

        # full paths and sizes with id as key
        self.rsc_cache = {
            0: [os.path.join(self.dir, f"{self.name}.resources"), 0],
            1: [os.path.join(self.dir, f"{self.name}_001.resources"), 0],
            2: [os.path.join(self.dir, f"{self.name}_002.resources"), 0],
            3: [os.path.join(self.dir, f"{self.name}_003.resources"), 0],
            4: [os.path.join(self.dir, f"{self.name}_004.resources"), 0],
            5: [os.path.join(self.dir, f"{self.name}_005.resources"), 0],
            6: [os.path.join(self.dir, f"{self.name}_patch.resources"), 0],
            8192: [os.path.join(self.dir, f"shared_2_3.sharedrsc"), 0],
        }
        for v in self.rsc_cache.values():
            if os.path.exists(v[0]):
                v[1] = os.stat(v[0]).st_size

        # read minimal entries from the active index
        self.entry_count = 0
        self.entries = []
        with open(self.idx_path, "rb") as f:
            fmt = ">4s28xL"
            (
                magic,
                self.entry_count,
            ) = struct.unpack(fmt, f.read(struct.calcsize(fmt)))
            # .index should start with 0x05, .resources should be 0x04
            # master.index is a mystery
            assert magic == b"\x05SER", f"Invalid index: {self.idx_path}"
            for i in range(self.entry_count):
                entry = ResourceEntry(self, f)
                # ignore entries with no size (deleted?)
                if entry.size and entry.rsc_size:
                    self.entries.append(entry)

    @property
    def idx_path(self):
        return self.idx_cache[self.idx_id][0]

    def does_fit(self, entry):
        return self.rsc_cache[entry.rsc_id][1] >= (entry.rsc_offset + entry.rsc_size)

    def read(self, entry, raw=False):
        success = True
        data = b""
        if entry.rsc_size == 0:
            return data
        with open(self.rsc_cache[entry.rsc_id][0], "rb") as f:
            f.seek(entry.rsc_offset)
            data = f.read(entry.rsc_size)
        if entry.zipped and not raw:
            try:
                data = zlib.decompress(data)
            except Exception as e:
                raise Exception(f"Failed to decompress: {e}")
        return data

    def write(self, entry, data):
        if not isinstance(data, (bytes, bytearray)):
            # user passed a path we need to read
            with open(data, "rb") as f:
                data = f.read()
        size = len(data)
        rsc_size = len(data)
        if entry.zipped:
            co = zlib.compressobj(wbits=10)
            data = co.compress(data)
            data += co.flush(zlib.Z_FINISH)
            # pad data out to match previous data size
            data += b"\x00" * (entry.rsc_size - len(data))
            rsc_size = len(data)
        if rsc_size > entry.rsc_size:
            return False
        # write the new data into the resource file
        with open(self.rsc_cache[entry.rsc_id][0], "r+b") as f:
            f.seek(entry.rsc_offset)
            f.write(data)
        # update the index if our data size changed
        if size != entry.size or rsc_size != entry.rsc_size:
            entry.size = size
            entry.rsc_size = rsc_size
            with open(self.idx_cache[self.idx_id][0], "r+b") as f:
                f.seek(entry.idx_offset)
                f.write(entry.bytes())
        return True

    def export(self, entry, path=None, explicit=False, dry_run=False):
        if entry.rsc_size == 0 or not entry.dst:
            return True
        if path is None and explicit == True:
            raise Exception("No path provided for explicit export!")
        if path and not explicit:
            path = os.path.join(path, entry.dst)
        elif not explicit:
            path = entry.dst
        path = os.path.abspath(path)
        if os.path.exists(path) and explicit:
            raise Exception(f"Explicit export path already exists: {path}")
        elif os.path.exists(path):
            return True
        data = self.read(entry)
        if not dry_run:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "wb") as f:
                f.write(data)
        return True

    def __repr__(self):
        return f"<ResourceContainer(name={self.name}, idx_id={self.idx_id}, entries={self.entry_count})>"


class ResourceEntry:
    def __init__(self, container, f):
        """
        container = parent ResourceContainer
        f = open index file positioned at the start of an entry
        """
        self.container = container
        self.idx_offset = f.tell()
        self.id = struct.unpack(">L", f.read(4))[0]
        for k in ("type", "src", "dst"):
            name_size = struct.unpack("<L", f.read(4))[0]
            name = f.read(name_size).decode("utf-8")
            setattr(self, k, name)
            setattr(self, f"{k}_basename", os.path.basename(name))
        fmt = ">QLL6sHH"
        (
            self.rsc_offset,
            self.size,
            self.rsc_size,
            self.unk,
            self.flags_1,
            self.flags_2,
        ) = struct.unpack(fmt, f.read(struct.calcsize(fmt)))
        self.zipped = bool(self.size != self.rsc_size)
        self.rsc_id = self.flags_2 >> 2
        assert self.does_fit(), f"Resource does not fit: {self}"

    @property
    def src_ext(self):
        return os.path.splitext(self.src)[1]

    @property
    def dst_ext(self):
        return os.path.splitext(self.dst)[1]

    def bytes(self):
        return b"".join(
            (
                struct.pack(">L", self.id),
                struct.pack("<L", len(self.type)),
                self.type.encode("utf-8"),
                struct.pack("<L", len(self.src)),
                self.src.encode("utf-8"),
                struct.pack("<L", len(self.dst)),
                self.dst.encode("utf-8"),
                struct.pack(
                    ">QLL6sHH",
                    self.rsc_offset,
                    self.size,
                    self.rsc_size,
                    self.unk,
                    self.flags_1,
                    self.flags_2,
                ),
            )
        )

    def does_fit(self):
        return self.container.does_fit(self)

    def read(self, raw=False):
        return self.container.read(self, raw)

    def write(self, data):
        return self.container.write(self, data)

    def export(self, path=None, explicit=False, dry_run=False):
        return self.container.export(self, path, explicit, dry_run)

    def entities(self):
        if self.dst_ext != ".entities":
            return None
        return entlib.load_entities(self.read())

    def __repr__(self):
        return f"<ResourceEntry(type={self.type}, src={self.src_basename}, dst={self.dst_basename})>"


class MapResources:
    def __init__(self, path, data=None):
        if data is None:
            with open(path, "rb") as f:
                data = f.read()
        assert len(data) >= 12, f".mapresources files must be at least 12 bytes!"
        # TODO: what are the first 8 bytes of the header?
        self.header_unk = data[:8]
        self.entry_count = struct.unpack(">L", data[8:12])[0]
        self.entries = []
        for i in range(self.entry_count):
            offset = 12 + (28 * i)
            self.entries.append(
                {
                    "offset": offset,
                    # sometimes there are multiple entries that have the same
                    # id, but they always appear to be images (base w/ mip levels)
                    "id": struct.unpack(">L", data[offset : offset + 4])[0],
                    # NOTE: this data is always null bytes with an 0x80 terminator
                    # "data": data[offset + 4 : offset + 28],
                }
            )


def filter_entries(func, containers):
    for c in containers:
        for e in c.entries:
            if func(e):
                yield e


def test_export(container):
    for entry in container.entries:
        assert entry.export(dry_run=True), f"Failed to export {entry}!"


def test_modify_subtitles(container):
    """Pass in game1.index based container."""
    # this is the first line of subtitles for the regular campaign (not tutorial)
    old_str = b"Why do we celebrate the anniversary of an assassination?"
    new_str = b"WE HAVE LIFTOFF MY FRIENDS!"
    for entry in container.entries:
        if entry.dst.endswith("english_m.lang"):
            break
    old_data = entry.read()
    new_data = old_data.replace(old_str, new_str)
    entry.write(new_data)


def load_containers(base_path="C:\\Steam\\steamapps\\common\\Dishonored2\\base"):
    return [
        ResourceContainer(os.path.join(base_path, "game1.index")),
        ResourceContainer(os.path.join(base_path, "game2.index")),
        ResourceContainer(os.path.join(base_path, "game3.index")),
        ResourceContainer(os.path.join(base_path, "game4.index")),
    ]

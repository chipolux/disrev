import os
import struct
import zlib


class ResourceContainer:
    def __init__(self, path):
        """
        path = full path to <some>.index or <some>.resources file
               the game<id> will be extracted and used to build an
               entry list and accessors for the resources files.
        """
        path = os.path.abspath(path)
        self.dir = os.path.dirname(path)
        self.name = os.path.basename(path)[:5]
        assert self.name.startswith("game"), f"Invalid container: {path}"

        # full paths and sizes with id as key
        self.idx_id = 0  # indexes get overriden with later versions
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
                self.idx_id = k  # use the latest index that exists

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
                self.entries.append(ResourceEntry(self, f))

    @property
    def idx_path(self):
        return self.idx_cache[self.idx_id][0]

    def does_fit(self, entry):
        return self.rsc_cache[entry.rsc_id][1] >= (entry.rsc_offset + entry.rsc_size)

    def load(self, entry):
        success = True
        data = b""
        if entry.rsc_size == 0:
            return success, data
        with open(self.rsc_cache[entry.rsc_id][0], "rb") as f:
            f.seek(entry.rsc_offset)
            data = f.read(entry.rsc_size)
        if entry.zipped:
            try:
                data = zlib.decompress(data)
            except Exception as e:
                data = f"Failed to decompress: {e}".encode("utf-8")
                success = False
        if success and len(data) != entry.size:
            data = f"Retrieved data is incorrect size: {len(data)} != {entry.size}"
            success = False
        return success, data

    def export(self, entry, path=None, dry_run=False):
        if entry.rsc_size == 0 or not entry.dst:
            return True
        if path:
            path = os.path.join(path, entry.dst)
        else:
            path = entry.dst
        path = os.path.abspath(path)
        if os.path.exists(path):
            return True
        success, data = self.load(entry)
        if success and not dry_run:
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "wb") as f:
                f.write(data)
        return success

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
        fmt = ">QLL6xHH"
        (
            self.rsc_offset,
            self.size,
            self.rsc_size,
            self.flags_1,
            self.flags_2,
        ) = struct.unpack(fmt, f.read(struct.calcsize(fmt)))
        self.zipped = bool(self.size != self.rsc_size)
        self.rsc_id = self.flags_2 >> 2
        assert self.does_fit(), f"Resource does not fit: {self}"

    def does_fit(self):
        return self.container.does_fit(self)

    def load(self):
        return self.container.load(self)

    def export(self, path=None, dry_run=False):
        return self.container.export(self, path, dry_run)

    def __repr__(self):
        return f"<ResourceEntry(type={self.type}, src={self.src_basename}, dst={self.dst_basename})>"


def test_export(container):
    for entry in container.entries:
        assert entry.export(dry_run=True), f"Failed to export {entry}!"


if __name__ == "__main__":
    base_dir = "C:\\Steam\\steamapps\\common\\Dishonored2\\base"
    print("Loading known containers...")
    containers = [
        ResourceContainer(os.path.join(base_dir, "game1.index")),
        ResourceContainer(os.path.join(base_dir, "game2.index")),
        ResourceContainer(os.path.join(base_dir, "game3.index")),
        ResourceContainer(os.path.join(base_dir, "game4.index")),
    ]
    print(f"Done!")

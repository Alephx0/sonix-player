"""Bounded static PE inventory. Does not execute code or collect credentials."""
import hashlib
import json
from pathlib import Path
import re
import struct
import sys


def inspect(path):
    data = path.read_bytes()
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[:2] != b"MZ" or data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("not a PE image")
    machine, count, timestamp, _, _, optional_size, flags = struct.unpack_from("<HHIIIHH", data, pe + 4)
    optional = pe + 24
    sections = []
    for i in range(count):
        pos = optional + optional_size + i * 40
        name, vsize, va, size, offset = struct.unpack_from("<8sIIII", data, pos)
        sections.append({"name": name.rstrip(b"\0").decode("ascii", "replace"),
                         "virtual_size": vsize, "rva": va, "raw_size": size, "offset": offset})
    def rva_offset(rva):
        for s in sections:
            if s["rva"] <= rva < s["rva"] + s["raw_size"]:
                return s["offset"] + rva - s["rva"]
        raise ValueError("RVA outside file-backed sections")
    magic = struct.unpack_from("<H", data, optional)[0]
    directories = optional + (112 if magic == 0x20b else 96)
    imports_rva = struct.unpack_from("<I", data, directories + 8)[0]
    dlls = []
    if imports_rva:
        pos = rva_offset(imports_rva)
        for _ in range(256):
            entry = struct.unpack_from("<IIIII", data, pos)
            if not any(entry):
                break
            off = rva_offset(entry[3])
            dlls.append(data[off:data.index(b"\0", off)].decode("ascii", "replace"))
            pos += 20
    # Restrict outputs to Go symbol identifiers. Never dump arbitrary strings,
    # app settings, token stores, URLs with queries, or embedded credentials.
    symbols = []
    pattern = rb'(?:github\.com/afkarxyz/spotbye/|main\.)[A-Za-z0-9_./()*-]{3,200}'
    for match in re.finditer(pattern, data):
        value = match.group().decode("ascii")
        if re.search(r'search|download|metadata|lyrics|album|artist|playlist|queue|sanitize|duplicate|quality|tag', value, re.I):
            symbols.append({"offset": match.start(), "symbol": value})
    return {"sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data),
            "machine": hex(machine), "optional_header_magic": hex(magic),
            "coff_timestamp": timestamp, "characteristics": hex(flags),
            "sections": sections, "import_dlls": dlls,
            "workflow_symbol_candidates": symbols[:1000],
            "candidate_count": len(symbols),
            "limitations": ["Symbol presence is not behavioral proof.",
                            "Version resources and embedded module paths are claims, not authenticated provenance."]}


if __name__ == "__main__":
    result = inspect(Path(sys.argv[1]))
    Path(sys.argv[2]).write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({k: result[k] for k in ["sha256", "machine", "import_dlls", "candidate_count"]}, indent=2))

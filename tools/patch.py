import os
import re

HEADERS = [
    "3libs/sdsalloc.h",
    "3libs/sds.h",
    "3libs/yyjson.h",
    "3libs/log.h",
    "3libs/hashmap.h",
    "3libs/stb_sprintf.h",
    "3libs/arena.h",
    "src/include/models.h",
    "src/include/cache.h",
    "src/include/inference.h",
    "src/include/async.h"
]

SOURCES = [
    "3libs/sds.c",
    "3libs/yyjson.c",
    "3libs/log.c",
    "3libs/hashmap.c",
    "src/main.c"
]

# System includes cần thiết (hardcode để tránh gom nhầm windows.h, etc.)
SYS_INCLUDES = [
    "<stdio.h>", "<stdlib.h>", "<string.h>", "<stdarg.h>",
    "<stdint.h>", "<stdbool.h>", "<stddef.h>",
    "<math.h>", "<assert.h>", "<time.h>", "<unistd.h>",
    "<signal.h>", "<errno.h>", "<float.h>", "<limits.h>", "<ctype.h>",
    "<pthread.h>", "<sys/time.h>", "<sys/mman.h>",
    "<curl/curl.h>", "<uv.h>", "<microhttpd.h>",
]

def strip_all_includes(text):
    """Xóa tất cả #include khỏi nội dung file."""
    lines = text.split('\n')
    filtered = []
    for line in lines:
        s = line.strip()
        if s.startswith('#include'):
            continue
        filtered.append(line)
    return '\n'.join(filtered)

def remove_impl_defines(text):
    """Loại bỏ #define _IMPLEMENTATION vì core.c đã define ở đầu file."""
    lines = text.split('\n')
    filtered = []
    for line in lines:
        s = line.strip()
        if s in ('#define STB_SPRINTF_IMPLEMENTATION', '#define ARENA_IMPLEMENTATION'):
            continue
        filtered.append(line)
    return '\n'.join(filtered)

def build_core():
    output_file = 'core.c'
    print(f"Building {output_file}...")

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write("/* ========================================================= */\n")
        f.write("/* AUTO-GENERATED AMALGAMATION FILE: core.c                  */\n")
        f.write("/* Do not edit! Regenerate with: python3 patch.py            */\n")
        f.write("/* ========================================================= */\n\n")

        f.write("#define STB_SPRINTF_IMPLEMENTATION\n")
        f.write("#define ARENA_IMPLEMENTATION\n\n")

        f.write("/* --- SYSTEM INCLUDES --- */\n")
        for inc in SYS_INCLUDES:
            f.write(f"#include {inc}\n")
        f.write("\n")

        for file_list, phase in [(HEADERS, 'HEADERS'), (SOURCES, 'SOURCES')]:
            for path in file_list:
                if not os.path.exists(path):
                    print(f" Warning: Missing file {path}")
                    continue


                with open(path, 'r', encoding='utf-8') as sf:
                    content = sf.read()
                content = strip_all_includes(content)
                content = remove_impl_defines(content)
                f.write(content + "\n")

    print(f"Success: Generated {output_file}")
    print(f"Build:   gcc -O3 core.c -o a35m_bot -lcurl -lm -lmicrohttpd -luv")

if __name__ == '__main__':
    build_core()

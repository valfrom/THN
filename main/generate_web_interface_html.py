#!/usr/bin/env python3
"""Generate compressed WebInterfaceHTML.h from index.html.

The script picks the smallest available compression supported by browsers
(currently Brotli preferred, falling back to gzip) and emits a PROGMEM array
suitable for serving with a `Content-Encoding` header.
"""

from __future__ import annotations

import gzip
import subprocess
from pathlib import Path
from typing import Iterable, Tuple

INDEX_PATH = Path(__file__).with_name("index.html")
OUTPUT_PATH = Path(__file__).with_name("WebInterfaceHTML.h")


def _compress_with_brotli(data: bytes) -> Tuple[str, bytes] | None:
    try:
        import brotli  # type: ignore
    except ImportError:
        return None
    return "br", brotli.compress(data, quality=11)


def _compress_with_gzip(data: bytes) -> Tuple[str, bytes]:
    return "gzip", gzip.compress(data)


def _format_bytes(data: Iterable[int]) -> str:
    formatted_lines = []
    byte_list = list(data)
    for index in range(0, len(byte_list), 12):
        chunk = ", ".join(f"0x{value:02x}" for value in byte_list[index : index + 12])
        formatted_lines.append(f"    {chunk},")
    return "\n".join(formatted_lines)


def _get_short_commit_id() -> str:
    try:
        result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=INDEX_PATH.parent,
            check=True,
            capture_output=True,
            text=True,
        )
        commit_id = result.stdout.strip()
        if not commit_id:
            return "unknown"

        dirty_check = subprocess.run(
            ["git", "status", "--porcelain"],
            cwd=INDEX_PATH.parent,
            capture_output=True,
            text=True,
            check=True,
        )
        if dirty_check.stdout.strip():
            commit_id = f"{commit_id}-dirty"

        return commit_id
    except Exception:
        return "unknown"


def main() -> None:
    html_text = INDEX_PATH.read_text()
    commit_id = _get_short_commit_id()
    html = html_text.replace("__VERSION__", commit_id).encode()

    options: list[Tuple[str, bytes]] = []
    brotli_result = _compress_with_brotli(html)
    if brotli_result:
        options.append(brotli_result)
    options.append(_compress_with_gzip(html))

    encoding, payload = min(options, key=lambda pair: len(pair[1]))

    header = f"""#pragma once

#include <pgmspace.h>
#include <stddef.h>

namespace interface {{

static const uint8_t kWebInterfaceHtmlCompressed[] PROGMEM = {{
{_format_bytes(payload)}
}};

static const size_t kWebInterfaceHtmlCompressedLength = sizeof(kWebInterfaceHtmlCompressed);
static const char kWebInterfaceContentEncoding[] = \"{encoding}\";

}}  // namespace interface

"""

    OUTPUT_PATH.write_text(header)

    print(
        f"Wrote {OUTPUT_PATH} using {encoding}: {len(html)} bytes -> {len(payload)} bytes",
        flush=True,
    )


if __name__ == "__main__":
    main()

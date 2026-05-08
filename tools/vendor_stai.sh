#!/usr/bin/env bash
# Requires bash >= 4 (associative arrays). On macOS install via: brew install bash
# Re-vendor lib/stai/libstai/ from a STEdgeAI install.
# Usage: tools/vendor_stai.sh [STEDGEAI_AI_DIR]
#   default STEDGEAI_AI_DIR: $HOME/STEdgeAI/4.0/Middlewares/ST/AI
set -euo pipefail

SRC="${1:-$HOME/STEdgeAI/4.0/Middlewares/ST/AI}"
REPO="$(git rev-parse --show-toplevel)"
DST="$REPO/lib/stai/libstai"

[ -d "$SRC" ] || { echo "ERROR: source dir not found: $SRC" >&2; exit 1; }
[ -d "$DST" ] || { echo "ERROR: dest dir not found: $DST" >&2; exit 1; }

# Header source roots, in priority order (first match wins for duplicates).
# STM32N6xx wins over generic ll_aton/Inc so device-specific ATON.h/cache.h are picked.
HDR_ROOTS=(
    "$SRC/Npu/Devices/STM32N6xx"
    "$SRC/Npu/ll_aton"
    "$SRC/Reloc/Inc"
    "$SRC/Inc"
)

# Source search roots (first match wins).
SRC_ROOTS=(
    "$SRC/Npu/Devices/STM32N6xx"
    "$SRC/Npu/ll_aton"
    "$SRC/Reloc/Src"
)

# --- Snapshot current vendored layout (for diff report) -----------------------
OLD_HDRS="$(cd "$DST/include" && ls *.h 2>/dev/null | sort)"
OLD_SRCS="$(cd "$DST/ll_aton" && ls *.c 2>/dev/null | sort)"

# --- Mirror headers (clean-slate) ---------------------------------------------
# Dedupe by filename: first root in HDR_ROOTS to provide a name wins.
TMP_INC="$(mktemp -d)"
declare -A picked=()
for root in "${HDR_ROOTS[@]}"; do
    [ -d "$root" ] || continue
    while IFS= read -r -d '' f; do
        base="$(basename "$f")"
        [ -n "${picked[$base]:-}" ] && continue
        picked["$base"]="$root"
        cp "$f" "$TMP_INC/$base"
    done < <(find "$root" -maxdepth 1 -type f -name '*.h' -print0)
done

rm -f "$DST/include/"*.h
cp "$TMP_INC/"*.h "$DST/include/"
rm -rf "$TMP_INC"

# --- Mirror sources by current openmv list ------------------------------------
missing=()
for c in $OLD_SRCS; do
    found=""
    for root in "${SRC_ROOTS[@]}"; do
        [ -f "$root/$c" ] && { found="$root/$c"; break; }
    done
    if [ -z "$found" ]; then
        missing+=("$c")
        continue
    fi
    cp "$found" "$DST/ll_aton/$c"
done

# --- Diff report --------------------------------------------------------------
NEW_HDRS="$(cd "$DST/include" && ls *.h 2>/dev/null | sort)"
NEW_SRCS="$(cd "$DST/ll_aton" && ls *.c 2>/dev/null | sort)"

ver="$SRC/Npu/ll_aton/ll_aton_version.h"
echo
echo "=== Vendored from: $SRC ==="
[ -f "$ver" ] && grep -E '^#define LL_ATON_VERSION_NAME' "$ver" || true

echo
echo "=== Headers ==="
diff <(echo "$OLD_HDRS") <(echo "$NEW_HDRS") | sed -n 's/^< /  removed: /p; s/^> /  added:   /p' || true

echo
echo "=== Sources ==="
if [ ${#missing[@]} -gt 0 ]; then
    echo "  MISSING in install (kept old copy, fix stai.mk by hand):"
    printf '    %s\n' "${missing[@]}"
fi
diff <(echo "$OLD_SRCS") <(echo "$NEW_SRCS") | sed -n 's/^< /  removed: /p; s/^> /  added:   /p' || true

# --- stai.mk sanity check -----------------------------------------------------
echo
echo "=== stai.mk references vs. vendored sources ==="
mk="$REPO/lib/stai/stai.mk"
stale=0
while read -r p; do
    if [ ! -f "$REPO/lib/stai/$p" ]; then
        echo "  STALE: $p (in stai.mk but not vendored)"
        stale=1
    fi
done < <(grep -oE 'libstai/ll_aton/[a-z0-9_]+\.c' "$mk" | sort -u)
[ "$stale" -eq 0 ] && echo "  OK"

echo
echo "Done. Review with: git -C \"$REPO\" status -- lib/stai/libstai stai.mk"

#!/usr/bin/env python3
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
PATCHES_DIR = ROOT / "patches"

PATCHES: list[tuple[str, str]] = [
    ("third_party/imgui", "imgui-add-cmakelists.patch"),
]


def run(cmd: list[str], cwd: Path | None = None, check: bool = True) -> subprocess.CompletedProcess:
    where = f"  ({cwd.relative_to(ROOT)})" if cwd and cwd != ROOT else ""
    print(f"$ {' '.join(cmd)}{where}")
    return subprocess.run(cmd, cwd=cwd, check=check)


def patch_state(submodule: Path, patch_path: Path) -> str:
    """Return 'applied', 'pending', or 'conflict'."""
    forward = subprocess.run(
        ["git", "apply", "--check", str(patch_path)],
        cwd=submodule, capture_output=True,
    )
    if forward.returncode == 0:
        return "pending"
    reverse = subprocess.run(
        ["git", "apply", "--reverse", "--check", str(patch_path)],
        cwd=submodule, capture_output=True,
    )
    if reverse.returncode == 0:
        return "applied"
    return "conflict"


def main() -> int:
    if not (ROOT / ".gitmodules").exists():
        print("error: .gitmodules not found — not a git repo with submodules", file=sys.stderr)
        return 1

    run(["git", "submodule", "update", "--init", "--recursive"], cwd=ROOT)

    for sub_rel, patch_name in PATCHES:
        submodule = ROOT / sub_rel
        patch_path = PATCHES_DIR / patch_name
        if not submodule.exists():
            print(f"[skip] submodule missing: {sub_rel}")
            continue
        if not patch_path.exists():
            print(f"[skip] patch missing: {patch_path.relative_to(ROOT)}")
            continue

        state = patch_state(submodule, patch_path)
        if state == "applied":
            print(f"[ok ] {patch_name}: already applied in {sub_rel}")
        elif state == "pending":
            run(["git", "apply", str(patch_path)], cwd=submodule)
            print(f"[ok ] {patch_name}: applied to {sub_rel}")
        else:
            print(f"[err] {patch_name}: conflicts in {sub_rel} — resolve manually", file=sys.stderr)
            return 2

    print("init complete.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

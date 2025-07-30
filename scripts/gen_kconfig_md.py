import argparse
from pathlib import Path
import kconfiglib

def main():
    parser = argparse.ArgumentParser(description="Generate Markdown from Kconfig")
    parser.add_argument("kconfig_path", help="Path to the top-level Kconfig file")
    parser.add_argument("--output", default="docs/kconfig.md", help="Output markdown file (default: docs/kconfig.md)")
    args = parser.parse_args()

    kconfig = kconfiglib.Kconfig(args.kconfig_path)

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    with output.open("w") as f:
        f.write("# Kconfig Options\n\n")
        for sym in kconfig.unique_defined_syms:
            f.write(f"## `{sym.name}`\n\n")
            if sym.help:
                f.write(f"{sym.help.strip()}\n\n")
            f.write(f"- **Type**: {sym.type_str}\n")
            if sym.def_val:
                f.write(f"- **Default**: `{sym.def_val[0]}`\n")
            f.write("\n---\n\n")

    print(f"✅ Generated: {output}")

if __name__ == "__main__":
    main()

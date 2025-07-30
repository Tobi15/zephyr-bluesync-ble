import argparse
from pathlib import Path
import re

def parse_kconfig_file(kconfig_path: Path) -> list:
    entries = []
    current_entry = None
    help_text = []
    inside_help = False

    with kconfig_path.open("r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()

            if stripped.startswith("config "):
                if current_entry:
                    current_entry["help"] = "\n".join(help_text).strip()
                    entries.append(current_entry)

                current_entry = {
                    "name": stripped.split(" ")[1],
                    "type": None,
                    "default": None,
                    "help": None
                }
                help_text = []
                inside_help = False

            elif current_entry:
                if stripped.startswith(("bool", "int", "hex", "string", "tristate")):
                    current_entry["type"] = stripped
                elif stripped.startswith("default "):
                    current_entry["default"] = stripped.replace("default", "").strip()
                elif stripped == "help":
                    inside_help = True
                elif inside_help:
                    if line.startswith(" "):  # help lines must be indented
                        help_text.append(stripped)
                    else:
                        inside_help = False

        # Append last entry
        if current_entry:
            current_entry["help"] = "\n".join(help_text).strip()
            entries.append(current_entry)

    return entries

def write_markdown(entries: list, output_path: Path):
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", encoding="utf-8") as f:
        f.write("# Kconfig Options\n\n")

        for entry in entries:
            f.write(f"## `{entry['name']}`\n\n")
            if entry["help"]:
                f.write(f"{entry['help']}\n\n")
            f.write(f"- **Type**: `{entry['type'] or 'unspecified'}`\n")
            if entry["default"]:
                f.write(f"- **Default**: `{entry['default']}`\n")
            f.write("\n---\n\n")

    print(f"✅ Markdown generated at: {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Simple Kconfig to Markdown converter")
    parser.add_argument("kconfig_path", help="Path to the standalone Kconfig file")
    parser.add_argument("--output", default="docs/KCONFIG.md", help="Output .md file (default: docs/KCONFIG.md)")
    args = parser.parse_args()

    kconfig_file = Path(args.kconfig_path)
    output_file = Path(args.output)

    if not kconfig_file.exists():
        print(f"❌ Error: {kconfig_file} not found.")
        return

    entries = parse_kconfig_file(kconfig_file)
    write_markdown(entries, output_file)

if __name__ == "__main__":
    main()

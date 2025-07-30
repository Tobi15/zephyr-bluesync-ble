import argparse
from pathlib import Path

def parse_kconfig_file(kconfig_path: Path) -> list:
    entries = []
    current_entry = None
    help_lines = []
    inside_help = False

    with kconfig_path.open("r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()

            if stripped.startswith("config "):
                if current_entry:
                    current_entry["help"] = "\n".join(help_lines).strip()
                    entries.append(current_entry)

                current_entry = {
                    "name": stripped.split(" ", 1)[1],
                    "type": None,
                    "title": None,
                    "default": None,
                    "help": ""
                }
                help_lines = []
                inside_help = False

            elif current_entry:
                tokens = stripped.split(maxsplit=1)
                if tokens and tokens[0] in ("bool", "int", "hex", "string", "tristate"):
                    current_entry["type"] = tokens[0]
                    if len(tokens) > 1:
                        current_entry["title"] = tokens[1].strip('"')
                elif stripped.startswith("default "):
                    current_entry["default"] = stripped.replace("default", "").strip()
                elif stripped == "help":
                    inside_help = True
                elif inside_help:
                    if line.startswith(" "):  # Only indented lines count as help
                        help_lines.append(stripped)
                    else:
                        inside_help = False

    if current_entry:
        current_entry["help"] = "\n".join(help_lines).strip()
        entries.append(current_entry)

    return entries

def write_markdown(entries: list, output_path: Path):
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with output_path.open("w", encoding="utf-8") as f:
        f.write("# Kconfig Options\n\n")

        for entry in entries:
            f.write(f"## `{entry['name']}`\n\n")
            # Title from type line
            if entry["title"]:
                f.write(f"{entry['title']}\n\n")
            # Help text
            if entry['help']:
                f.write(f"{entry['help']}\n\n")
            f.write(f"- **Type**: `{entry['type'] or 'unspecified'}`\n")
            if entry["default"]:
                f.write(f"- **Default**: `{entry['default']}`\n")
            f.write("\n---\n\n")

    print(f"Markdown generated: {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Generate Markdown from standalone Kconfig")
    parser.add_argument("kconfig_path", help="Path to the standalone Kconfig file")
    parser.add_argument("--output", default="docs/KCONFIG.md", help="Output Markdown file (default: docs/KCONFIG.md)")
    args = parser.parse_args()

    kconfig_file = Path(args.kconfig_path)
    output_file = Path(args.output)

    if not kconfig_file.exists():
        print(f"Error: Kconfig file not found: {kconfig_file}")
        return

    entries = parse_kconfig_file(kconfig_file)
    write_markdown(entries, output_file)

if __name__ == "__main__":
    main()

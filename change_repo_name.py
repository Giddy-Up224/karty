import os
import glob
import subprocess

TEMPLATE_NAME = 'PIO_Template'  # Change if your template repo name changes

PATTERNS = [
    'README.md',
    '*.code-workspace',
]

def replace_in_file(filepath, old, new):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    new_content = content.replace(old, new)
    if new_content != content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"Updated {filepath}")

def main():
    # Remove the setup section from README.md
    readme_path = 'README.md'
    if os.path.exists(readme_path):
        with open(readme_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        new_lines = []
        skip = False
        for line in lines:
            if line.strip() == '## Setup':
                skip = True
                continue
            if skip and line.strip().startswith('## '):
                skip = False
            if not skip:
                new_lines.append(line)
        if len(new_lines) != len(lines):
            with open(readme_path, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            print("Removed setup section from README.md")

    # Automatically get the repo name from the current directory
    repo_name = os.path.basename(os.getcwd())
    if not repo_name or repo_name == TEMPLATE_NAME:
        print("No personalization needed.")
        return

    # Replace in files
    for pattern in PATTERNS:
        for filepath in glob.glob(pattern):
            replace_in_file(filepath, TEMPLATE_NAME, repo_name)

    # Rename .code-workspace file using git mv if possible
    for filepath in glob.glob('*.code-workspace'):
        if TEMPLATE_NAME in filepath:
            new_name = filepath.replace(TEMPLATE_NAME, repo_name)
            try:
                subprocess.run(['git', 'mv', filepath, new_name], check=True)
                print(f"Renamed {filepath} to {new_name} using git mv")
            except Exception as e:
                os.rename(filepath, new_name)
                print(f"Renamed {filepath} to {new_name} using os.rename (git mv failed: {e})")

if __name__ == '__main__':
    main()
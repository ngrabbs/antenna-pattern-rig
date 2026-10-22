"""Export a reviewed Markdown record with the repository's Pandoc lab style.

Requires existing pandoc and a TeX engine; installs nothing. On macOS the auto
profile uses XeLaTeX and system fonts. Elsewhere it uses the LuaLaTeX preset. The Markdown
file and adjacent assets may live outside the repository. Input is trusted local
Pandoc Markdown, which can include raw TeX; do not use on untrusted documents.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    root = Path(__file__).resolve().parents[2]
    style = root / 'templates' / 'antenna-lab' / 'style'
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--mainfont', help='Override the portable Latin Modern font')
    parser.add_argument('--sansfont', help='Override the heading font')
    parser.add_argument('--monofont', help='Override the code font')
    parser.add_argument('--engine', choices=['lualatex', 'xelatex'])
    parser.add_argument('--profile', choices=['auto', 'portable', 'macos'], default='auto')
    args = parser.parse_args()
    source = args.input.resolve(strict=True)
    target = args.output.resolve()
    if source.suffix.lower() != '.md' or target.suffix.lower() != '.pdf':
        parser.error('Input must be .md and output must be .pdf')
    macos = args.profile == 'macos' or (args.profile == 'auto' and sys.platform == 'darwin')
    engine = args.engine or ('xelatex' if macos else 'lualatex')
    mainfont = args.mainfont or ('Palatino' if macos else None)
    sansfont = args.sansfont or ('Arial' if macos else None)
    monofont = args.monofont or ('Menlo' if macos else None)
    if not shutil.which('pandoc') or not shutil.which(engine):
        parser.error('pandoc and the selected PDF engine must already be available on PATH')
    target.parent.mkdir(parents=True, exist_ok=True)
    cmd = ['pandoc', str(source), '--defaults', str(style / 'pandoc-lab.yaml'),
           '--include-in-header', str(style / 'lab-header.tex'),
           '--lua-filter', str(style / 'pagebreak.lua'), '--standalone',
           '--resource-path', str(source.parent), '--pdf-engine', engine, '--output', str(target)]
    if mainfont:
        cmd.extend(['--variable', 'mainfont=' + mainfont])
    if sansfont:
        cmd.extend(['--variable', 'sansfont=' + sansfont])
    if monofont:
        cmd.extend(['--variable', 'monofont=' + monofont])
    subprocess.run(cmd, cwd=source.parent, check=True)
    print(target)


if __name__ == '__main__':
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)

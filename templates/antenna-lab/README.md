# Antenna lab worksheet and report

Markdown is the lab record. Copy the worksheet for each antenna/configuration,
work through it with the assistant one stage at a time, review the changes in
GitHub, and export the reviewed Markdown when a PDF is useful.

## Files and scope

- [worksheet.md](worksheet.md): nine core stages, plus optional polarization and
  gain-substitution modules. Blank observations are TBD and verdicts are NOT RUN.
- [report.md](report.md): a shorter reader-facing report. Populate it from the
  completed worksheet; do not copy checkboxes into an unsupported compliance claim.
- [current-rig.md](current-rig.md): dated starting facts and implementation limits
  for the Uno/encoder, NanoVNA-F V2, LibreSDR/log and tinySA Ultra setup.
- `style/`: a single-column Pandoc preset, TeX header and page-break filter.

The core sequence is scope -> inspect -> VNA match -> loaded positioning -> range
geometry -> RF validation -> acquire -> repeat/analyze -> close/report. The VNA
stage helps select operating frequencies before radiated testing. Unsupported or
unavailable optional measurements remain N/A with a reason; they do not block a
clearly scoped relative-pattern report.

## Start a record

From the repository root, choose your own dated antenna ID:

```sh
mkdir -p records/YYYY-MM-DD-antenna-id
cp templates/antenna-lab/worksheet.md records/YYYY-MM-DD-antenna-id/worksheet.md
cp templates/antenna-lab/report.md records/YYYY-MM-DD-antenna-id/report.md
```

Set session ID, antenna revision, objective and next action on page 1. Ask the
assistant: "Open this worksheet and work through stage 01 with me." Keep the
original template blank. Revisit stages when wiring, calibration, antenna,
firmware, coax route or environment changes; record why and retain the old run.
An interrupted attempt gets its own run ID. Never overwrite raw records to make a
clean-looking result. Keep sensitive/private raw data out of a public repo and
include sufficient reviewed configuration and derived evidence for the claim.

For a qualified scan, declare criteria first: matching limit over a named band,
position closure/repeatability, acceptable RF drift, minimum signal margin,
attenuator-check tolerance, timing/settling and repeat-scan agreement. No universal
pass limits are silently supplied. For exploratory work, say so and record actual
results. Three repeats per direction are a useful starting plan, not a proof of a
statistical accuracy specification.

## One-command PDF export

Requires Python 3, Pandoc, a functioning TeX installation and the selected fonts.
The script installs nothing, accepts paths with spaces and finds the repository
style automatically. Images are resolved relative to the input Markdown file.

```sh
python3 tools/report/build_pdf.py records/YYYY-MM-DD-antenna-id/worksheet.md --output output/pdf/antenna-worksheet.pdf
python3 tools/report/build_pdf.py records/YYYY-MM-DD-antenna-id/report.md --output output/pdf/antenna-report.pdf
```

On macOS, the default auto profile uses XeLaTeX with Palatino, Arial and Menlo
system fonts. This was rendered and visually checked on the current machine;
its BasicTeX LuaLaTeX installation lacks `lualatex-math.sty`, and the original
TeX Gyre fonts are not available locally. No system packages were installed.

Elsewhere the default is LuaLaTeX with Latin Modern fonts. Force that profile on
a complete TeX installation with `--profile portable`. You can override fonts
with `--mainfont`, `--sansfont`, `--monofont`, and engine with `--engine`.
For the typography of the original readable preset, when those fonts are installed:

```sh
python3 tools/report/build_pdf.py records/YYYY-MM-DD-antenna-id/report.md --output output/pdf/antenna-report.pdf --profile portable --mainfont 'TeX Gyre Pagella' --sansfont 'TeX Gyre Heros' --monofont 'DejaVu Sans Mono'
```

The LuaLaTeX defaults also work directly with Pandoc. From the repository root:

```sh
pandoc records/YYYY-MM-DD-antenna-id/report.md --defaults templates/antenna-lab/style/pandoc-lab.yaml --include-in-header templates/antenna-lab/style/lab-header.tex --lua-filter templates/antenna-lab/style/pagebreak.lua --resource-path records/YYYY-MM-DD-antenna-id -o output/pdf/antenna-report.pdf
```

`<!-- pagebreak -->` comments are invisible on GitHub and start PDF pages. The
blank worksheet is designed as eleven stages/pages; longer filled answers may
flow to additional pages. The short report has natural pagination. Replace the
figure instruction with `![Caption](relative/path/to/figure.png)`. Use relative
links to raw archives/plots in Markdown. Prefer PNG figures for painless PDF
export; an SVG-only figure may require an additional converter.

## Styling provenance and review

The layout adapts the single-column serif text, readable spacing, restrained
color and table treatment of Nick's `/Volumes/notes/reports/markdown_to_pdf`
iPad/homework presets. It is a new repository-local style; the shared originals
are untouched. This avoids their hard-coded `/workspace/notes/...` header paths.
It is not an IEEE submission template. Use the existing IEEE presets if a later
publication requires that format.

Review Markdown evidence and verdicts first. After exporting, inspect the PDF for
clipped tables, missing images, glyph errors, page breaks and unresolved TBDs.
An unchecked task is a pending action, not automatically a FAIL. Remove or resolve
TBDs before issuing a final report. A printable PDF does not have interactive
AcroForm fields; all editing remains in Markdown.

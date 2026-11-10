# plan-29 ordering-key brief (pending rows)

Repo: `/Users/justinzaun/Development/logic/.claude/worktrees/P-29` (a git
worktree; work only there, absolute paths, never `cd` to the main checkout,
no git commands that change state). DigiKey calls are not allowed.

`planning/plan-29-data/candidates.tsv` (tab-separated; columns: 1
category, 2 maker, 3 mpn, … 23 datasheet_url, 24 decision, 25 reason)
has `pending` rows: in-stock DigiKey part numbers whose datasheet PDF does
not print the number verbatim. `planning/plan-29-data/specs.tsv` has one
row per (maker, category, datasheet, supplier_package); the rows with an
empty `mpn_regex` are the groups that hold pending rows. The `datasheet`
column is the PDF path; `datasheets/diode/datasheets.tsv` maps a row's
`datasheet_url` to that path.

Your job (plan-29-B § 4.5): for every spec row of your scope with an empty
`mpn_regex`, read that PDF's ordering information (part-number key,
"ORDERING INFORMATION (Example)" tables, "ORDERING CODE", "PART NUMBER /
PACKAGE CODE", series tables, marking keys; `pdftotext -layout <pdf> -`;
read the page as an image when the table is a picture) and write a Python
regular expression (used with `re.fullmatch`) that matches exactly the part
numbers the datasheet's ordering table covers, built only from what the
datasheet prints:

- the base part numbers it covers (title, ratings or ordering table;
  `1N4001`–`1N4007` → `1N400[1-7]`),
- the suffixes / package codes / packing codes / grade letters its ordering
  information shows (Vishay `1n4001.pdf` example table shows `-E3/54` and
  `-E3/73` only → `^1N400[1-7]-E3/(54|73)$`; a code printed nowhere in that
  PDF, such as `/53`, is not covered),
- a pattern the datasheet itself states (Taiwan Semi `SS1x`, "x defines
  voltage from 20V(SS12) to 150V(SS115)" → the voltages it lists).

Do not widen a key to admit a DigiKey number the PDF does not support; a
non-matching pending row becomes `skip` ("not in the datasheet's ordering
table"), which is the correct outcome for it. Do not narrow a key below
what the PDF prints. The key may also match `add` rows of the group (those
are verbatim already); it must not need to.

Output (write only this file):
`planning/plan-29-data/work/regex-<X>.tsv`, header
`maker	category	datasheet	supplier_package	mpn_regex	note`, one line
per spec row of your scope with an empty mpn_regex (maker and category
exactly as in specs.tsv). `note` = where the key is printed, e.g.
`p.3 ORDERING INFORMATION (Example): codes 54, 73; bases from p.1 table`.
If a PDF has no ordering information at all, write the regex built from the
part numbers it prints (so only verbatim numbers match) and say so in note.

Check: copy specs.tsv and candidates.tsv to /tmp/<you>/, put your regexes
into the copy of specs.tsv (match the four key columns), and run
`python3 -I planning/plan-29-data/census/scheme-check.py --tsv /tmp/<you>/candidates.tsv --specs /tmp/<you>/specs.tsv --dry-run <your --maker/--category filters>`;
it must print `0 still pending` for your scope. Look at what becomes
`skip` and make sure each skip is right per the PDF (a sample of each
group with skips, by reading the ordering table again).

Report back: rows written, how many pending rows of your scope become add
and skip, the PDFs whose ordering table you could not read, and any
doubtful key.

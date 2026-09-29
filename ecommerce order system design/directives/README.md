# Directives (Layer 1)

This directory contains Standard Operating Procedures (SOPs) written in Markdown.

## Structure of a Directive
Each directive should clearly specify:
1. **Goal**: What objective does this procedure achieve?
2. **Inputs**: Required inputs, file paths, credentials, or parameters.
3. **Execution Tools**: Which deterministic script in `execution/` to run.
4. **Outputs**: Where outputs are saved (e.g., deliverables in cloud or intermediate files in `.tmp/`).
5. **Edge Cases & Failure Modes**: Known issues, rate limits, retry strategies, and learnings.

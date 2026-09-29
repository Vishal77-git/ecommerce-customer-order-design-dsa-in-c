# Execution (Layer 3)

This directory contains deterministic Python scripts that handle:
- API calls and integrations
- Data processing and transformations
- File operations and database interactions

## Guidelines
- Keep scripts deterministic, reliable, and testable.
- Read credentials and keys from `.env`.
- Write temporary/intermediate files to `.tmp/`.
- Handle errors gracefully and emit clear error outputs for Layer 2 (Orchestration).

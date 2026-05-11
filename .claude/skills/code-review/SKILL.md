---
name: code-review
description: Review a git commit for code quality, potential issues, and adherence to project standards.
model: sonnet
---

Review the commit $ARGUMENTS.

# Check list
* Logic errors. (wrong logics, memory leaks...)
* Human errors. (typos, debug log and codes...)

# Additional notes
* Show as many suggestions as you can.
* Refer other similar functions in the code if you can.

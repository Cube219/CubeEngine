---
name: code-review
description: Review a git commit for code quality, potential issues, and adherence to project standards.
model: sonnet
---

Review the commit $ARGUMENTS.

# Check list
* Logic errors. (wrong logics, memory leaks...)
* Human errors. (typos, debug log and codes...)
* Typos and grammar mistakes in the commit messages.

# Additional notes
* Refer other similar functions in the code if you can.
* Does not consider TODO comments.
* Suggest adding comments if the added codes has a specific intent.

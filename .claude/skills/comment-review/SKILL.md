---
name: comment-review
description: Review a git commit message and code comments.
model: sonnet
---

Review the code comments and commit message in commit '$ARGUMENTS'.

# Check list
* Typos and grammar mistakes in the code comments.
* Typos and grammar mistakes in the commit messages but ignore '*' or '*WIP' on prefix.
* Refine awkward expressions in the code comments and the commit messages.

# Additional notes
* Show as many suggestions as you can.
* Do not review TODO comments.
* Suggest adding comments if the added code has a specific intent.

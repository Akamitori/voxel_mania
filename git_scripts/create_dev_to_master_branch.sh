#!/bin/bash

# 1️⃣ Checkout master and create a temporary branch with clean history
git checkout master
git pull origin master
git checkout -b master-clean

# 2️⃣ Cherry-pick all non-merge commits from master..dev (inclusive) with original timestamps
git log --no-merges --reverse --format="%H" master...dev | while read commit; do
    DATE="$(git show -s --format=%aI $commit)"
    MSG="$(git log -1 --format=%B $commit)"
    git cherry-pick --no-commit $commit
    GIT_COMMITTER_DATE="$DATE" git commit --date "$DATE" -m "$MSG"
done

# 3️⃣ Replace master with the clean history
git checkout master
git reset --hard master-clean
git push origin master --force

# 4️⃣ Update dev to match master's clean history
git checkout dev
git reset --hard master
git push origin dev --force

# 5️⃣ Cleanup temporary branch
git branch -D master-clean

echo "✅ Done! Both master and dev now have clean linear history."
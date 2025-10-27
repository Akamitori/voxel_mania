# 1️⃣ Checkout master and create a temporary branch
git checkout master
git pull origin master        # make sure it's up to date
git checkout -b dev-on-master

# 2️⃣ Cherry-pick all non-merge commits from dev onto the new branch
git log --no-merges --reverse --format="%H" master..dev | while read commit; do
    DATE="$(git show -s --format=%aI $commit)"
    MSG="$(git log -1 --format=%B $commit)"
    git cherry-pick --no-commit $commit
    GIT_COMMITTER_DATE="$DATE" git commit --date "$DATE" -m "$MSG"
done

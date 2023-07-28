#!/usr/bin/env bash

# Akimov D. 2023/26/05 @ MACH UNIT

# Regular Colors
Black='\033[0;30m'        # Black
Red='\033[0;31m'          # Red
Green='\033[0;32m'        # Green
Yellow='\033[0;33m'       # Yellow
Blue='\033[0;34m'         # Blue
Purple='\033[0;35m'       # Purple
Cyan='\033[0;36m'         # Cyan
White='\033[0;37m'        # White


# Bold
BBlack='\033[1;30m'       # Black
BRed='\033[1;31m'         # Red
BGreen='\033[1;32m'       # Green
BYellow='\033[1;33m'      # Yellow
BBlue='\033[1;34m'        # Blue
BPurple='\033[1;35m'      # Purple
BCyan='\033[1;36m'        # Cyan
BWhite='\033[1;37m'       # White

git stash
git stash clear

set -e

local_branch=$(git rev-parse --symbolic-full-name --abbrev-ref HEAD)
remote_branch=$(git rev-parse --abbrev-ref --symbolic-full-name @{u})
remote=$(git config branch.$local_branch.remote)

echo -e "${BYellow} Полученние данных из $remote...${White}"
git fetch $remote

if git merge-base --is-ancestor $remote_branch HEAD; then
    echo -e 'Already up-to-date'
    exit 0
fi

if git merge-base --is-ancestor HEAD $remote_branch; then
    echo -e '${BGreen} Текущая ветка имеет несколько дополнительных коммитов. Слияние...${White}'
    git merge --ff-only --stat $remote_branch
else
    echo -e '${BGreen} Текущая ветка не имеет дополнительных коммитов. Перебазирование...${White}'
    git rebase --preserve-merges --stat $remote_branch
fi

echo -e "${BYellow} Сборка исходного кода....${White}"
make

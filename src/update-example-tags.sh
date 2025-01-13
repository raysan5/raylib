#!/usr/bin/env bash

set -Eeuo pipefail

# allow for simple recursive file globs
shopt -s globstar

# the left entries will be replaced by the right entries
# the following variables can be used in the replacement: \$filename, \$title, \$imgname, \$category
declare -A replacements=(
    "<title>raylib web game</title>"                        "<title>\$title</title>"
    "<meta name=\"title\" content=\"raylib web game\">"     "<meta name=\"title\" content=\"\$title\">"
    "New raylib web videogame\, developed using raylib videogames library" "raylib [\$category] example - \$title"
    "raylib - example"                                      "raylib - \$title"
    "https://www.raylib.com/common/raylib_logo.png"         "https://raw.githubusercontent.com/raysan5/raylib/master/examples/\$category/\$imgname"
    "https://www.raylib.com/games.html"                     "https://www.raylib.com/examples/\$category/\$filename"
    "raylib web game"                                       "\$title"
)

function update_tags {
    echo "update tags for: $1"
    export filename=$(basename $1)
    noext="${filename%.*}"
    export title="${noext//_/ }"
    export imgname="${noext}.png"
    parts=($title)
    export category="${parts[0]}"

    for key in "${!replacements[@]}"; do
        r=${replacements[$key]}
        r=$(envsubst <<< "$r")
        bash -c "sed -i 's,$key,$r,g' $1"
    done
}

pushd ../examples

for i in **/*.html; do
    update_tags "$i"
done

#!/bin/bash

shared_path="/m/work/courses/unix/T/cs-e4580/nn/weights.bin"
local_path="$(git rev-parse --show-toplevel)"
weights_name="weights.bin"
weights_url="https://users.aalto.fi/~suomelj1/ppc-2018/nn/vgg19-weights.bin"

tasks=(nn9a nn9b)

function findlocalweights() {
    for task in "${tasks[@]}"
    do
        file="$local_path/$task/$weights_name"
        if [ -e "$file" ]
        then
            findlocalweights="$file"
            return 0
        fi
    done
    return 1
}
function checkmissingweights() {
    missing=()
    for task in "${tasks[@]}"
    do
        file="$local_path/$task/$weights_name"
        if [ ! -e "$file" ]
        then
            missing+=("$file")
        fi
    done
    checkmissingweights="${missing[@]}"
    if [ ${#missing[@]} -gt 0 ]
    then
        return 1
    else
        return 0
    fi
}

function linkweights() {
    target="$1"
    for task in "${tasks[@]}"
    do
        file="$local_path/$task/$weights_name"
        if [ ! -e "$file" ]
        then
            echo "Adding $file"
            ln -s "$target" "$file"
        fi
    done
}

function checkpermission() {
    prompt="$1"
    while true
    do
        read -p "$prompt [yn]" yn
        case "$yn" in
            [Yy]* ) return 0 ;;
            [Nn]* ) return 1 ;;
            * ) echo "Please answer yes or no" ;;
        esac
    done
}

# First check if local file exists
echo "Checking if the weights file already exists"
findlocalweights
if [ $? -eq 0 ]
then
    # Local file found
    echo "Weights file found at $findlocalweights"

    # Make sure that all task have the weights file
    checkmissingweights
    if [ $? -ne 0 ]
    then
        echo "Some tasks are missing the weights file"
        checkpermission "Do you want to link the above file to them?"
        if [ $? -eq 0 ]
        then
            # Do the linking
            file="$(readlink --canonicalize-existing "$findlocalweights")"
            linkweights "$file"
            exit 0
        fi
        exit 1
    fi
    echo "All tasks have the weights file"
    exit 0
fi

# Local file not found
echo "No weights file found in local directory"

# Check if shared file is available
echo "Checking if a shared weights file is available"
if [ -e "$shared_path" ]
then
    echo "Shared weights file found at $shared_path"
    checkpermission "Do you want to link it to the tasks?"
    if [ $? -eq 0 ]
    then
        # Do the linking
        linkweights "$shared_path"
        exit 0
    fi
    exit 1
fi

# The shared file is not available
echo "No shared weights file found"
# Ask the user for a permission to download it
echo "The file needs to be downloaded"
destination="$local_path/${tasks[0]}/$weights_name"
echo "Please note that the file is big (~550MB)"
checkpermission "Do you want to download the file?"
if [ $? -ne 0 ]
then
    echo "You can manually download the file by running"
    echo "    wget \"$weights_url\" -O \"$destination\""
    exit 1
fi

echo "Downloading file"
tmp_file=$(mktemp)
trap "{ rm -f $tmp_file; }" EXIT        # Remove tmp_file on exit
curl "$weights_url" > "$tmp_file"
if [ $? -ne 0 ]
then
    rm "$tmp_file"
    echo "Download failed, exiting"
    exit 1
fi
mv "$tmp_file" "$destination"

echo "File downloaded"
checkpermission "Do you want to link the file downloaded file to other task?"
if [ $? -eq 0 ]
then
    # Do the linking
    linkweights "$destination"
    exit 0
fi
exit 1

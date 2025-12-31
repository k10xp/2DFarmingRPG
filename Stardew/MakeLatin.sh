#!/bin/bash

# Translate C keywords to latin

POSITIONAL_ARGS=()
HEADER_PATH=""
DIR=""

while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--dir)
      DIR="$2"
      shift # past argument
      shift # past value
      ;;
    -h|--header_path)
      HEADER_PATH="$2"
      shift
      shift 
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

array=()
while IFS=  read -r -d $'\0'; do
    array+=("$REPLY")
done < <(find "$DIR" -type f -name "*.c" -print0)

# for each file, replace all instances of (key) with (value)
declare -A replacements=()

# parse C header file for defines
while IFS=" " read -r a b; do
    replacements["$b"]="$a"
done < <(
    grep -E "#define (([a-z])\w+) (([a-z])\w+)" "$HEADER_PATH" | sed s/"#define "//g
    )


for i in "${!replacements[@]}"
do
    echo $i
    echo ${replacements[$i]}
done

# translate files into latin
tmp="temp.c"
for file in "${array[@]}"; do
    # add include for latin macros
    printf "%s\n" "#include \"LatinMacros.h\"" | cat - "$file" > "$tmp" && mv "$tmp" "$file"

    # prevent false matches from preprocessor directives
    perl -pi -e "s/#[ ]+if/#if/g" "$file"
    perl -pi -e "s/#[ ]+else/#else/g" "$file"
    perl -pi -e "s/#[ ]+else[ ]+if/#elif/g" "$file"

    # replace keywords
    for i in "${!replacements[@]}"
    do
        # replace 
        perl -pi -e "s/(?<=[\r\n\t \}\(])$i(?=[\*\n\t \(\{}])/${replacements[$i]}/g" "$file"
        perl -pi -e "s/^$i(?=[\*\n\t \(\{}])/${replacements[$i]}/g" "$file"
    done
done 

rm -f "$tmp"

#!/bin/bash
#

encode(){
    #dirty hack to change endianness
 printf "0x%016x\n" $1 |sed -r 's/0x(..)(..)(..)(..)(..)(..)(..)(..)/0x\8\7\6\5\4\3\2\1/'
}

if [ $# != 4 ]; then
    echo "usage: offimport.sh <cdroot> <label> <offroot> <dbroot>"
    exit 1
fi

find $1 -type d -printf "%P\n" |xargs -i% mkdir "$3/%"
#offmedia --add mount --directory "$1" --label "$2" --checkcmd "true" $4 ||exit
mid=`offmedia --list $4|grep -a "#MEDIUM: " |sed "s/#MEDIUM: //" |tail -1 `
mid=`encode $mid`
find $1 -type f -printf "%P %s\n"| while read file size; do
    dest="$3/$file"
    source="$1/$file"
    size=`encode $size`
    touch -r "$source" "$dest"
    chmod a-w "$dest"
    setfattr -n offlinefs.mediumid -v "$mid" "$dest"
    setfattr -n offlinefs.phid -v "$file" "$dest"
    setfattr -n offlinefs.size -v "$size" "$dest"
done

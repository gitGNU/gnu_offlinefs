#!/bin/bash
#

encode32(){
    #dirty hack to change endianness
    printf "0x%08x\n" $1 |sed -r 's/0x(..)(..)(..)(..)/0x\4\3\2\1/'
}
encode64(){
    #dirty hack to change endianness
    printf "0x%016x\n" $1 |sed -r 's/0x(..)(..)(..)(..)(..)(..)(..)(..)/0x\8\7\6\5\4\3\2\1/'
}

if [ $# -lt 3 ]; then
    echo "usage: offimport.sh <cdroot> <label> <offroot> [dbroot]"
    exit 1
fi
[ -z $4 ] && dbroot="$HOME/.offlinefs" || dbroot=$4

echo "cdroot: $1"
echo "label: $2"
echo "offroot: $3"
echo "dbroot: $dbroot"
echo "Press return to proceed..."
read
echo "Creating directory tree..."
find $1 -type d -printf "%P\n" |while read dir; do
    if [ -n "$dir" ];then
	mkdir "$3/$dir";
    fi
done
echo "Adding medium to the database..."
fingerprint=`stat -f -c %a%b%c%d%f%i%l%s%S%t $1`
offmedia --add mount --directory "$1" --label "$2" --checkcmd "test $fingerprint = \`stat -f -c %a%b%c%d%f%i%l%s%S%t $1\`" $dbroot ||exit
mid=`offmedia --list $dbroot |grep -a "#MEDIUM: " |sed "s/#MEDIUM: //" |tail -1 `
mid=`encode32 $mid`
backend=`offmedia --list $dbroot |grep -a "directory=" |sed "s/.*directory=//" |head -1 `
echo "Adding files to the database..."
find $1 -type f -printf "%s %P\n"| while read size file; do
    dest="$3/$file"
    source="$1/$file"
    size=`encode64 $size`
    touch -r "$source" "$dest"
    chmod a-w "$dest"
    setfattr -n offlinefs.mediumid -v "$mid" "$dest"
    setfattr -n offlinefs.phid -v "$file" "$dest"
    setfattr -n offlinefs.size -v "$size" "$dest"
    rm "$backend/$file"
done
echo "Done"

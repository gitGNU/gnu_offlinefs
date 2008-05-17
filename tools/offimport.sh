#!/bin/bash
# FIXME: the Medium::addfile function should me called when adding a file...
#


#dirty hack to change endianness
encode32(){
    printf "0x%08x\n" $1 |sed -r 's/0x(..)(..)(..)(..)/0x\4\3\2\1/'
}
encode64(){
    printf "0x%016x\n" $1 |sed -r 's/0x(..)(..)(..)(..)(..)(..)(..)(..)/0x\8\7\6\5\4\3\2\1/'
}
usage(){
    echo "usage: offimport.sh -i <cdroot> -l <label> [--insert <insert script>] [-o <offroot>] [--dbroot <dbroot>]"
    exit 1
}


while getopts l:i:o:d:s: opt; do
    case $opt in
	l) label=$OPTARG;;
	i) cdroot=$OPTARG;;
	s) insert=$OPTARG;;
	o) offroot=$OPTARG;;
	d) dbroot=$OPTARG;;
    esac
done
[ $((OPTIND-1)) != $# ] && usage

[ -z $dbroot ] && dbroot="$HOME/.offlinefs" 
[ -z $offroot ] && offroot=`mount |grep offlinefs |cut -d' ' -f3`
[ -z $insert ] && insert="/usr/local/etc/offlinefs/insert"
[ -z $label ] && usage
[ -z $cdroot ] && usage

echo "cdroot: $cdroot"
echo "label: $label"
echo "offroot: $offroot"
echo "dbroot: $dbroot"
echo "insert script: $insert"
echo "Press return to proceed..."
read
echo "Creating directory tree..."
find $cdroot -type d -printf "%P\n" |while read dir; do
    if [ -n "$dir" ];then
	mkdir "$offroot/$dir" &>/dev/null
    fi
done

echo "Adding medium to the database..."
fingerprint=`stat -f -c %a%b%c%d%f%i%l%s%S%t $cdroot`
offmedia --add insert --directory "$cdroot" --label "$label" --checkcmd "test $fingerprint = \`stat -f -c %a%b%c%d%f%i%l%s%S%t $cdroot\`" --insertscript $insert $dbroot ||exit
mid=`offmedia --list $dbroot |grep -a "#MEDIUM: " |sed "s/#MEDIUM: //" |tail -1 `
mid=`encode32 $mid`
backend=`offmedia --list $dbroot |grep -a "directory=" |sed "s/.*directory=//" |head -1 `
echo "Adding files to the database..."
find $cdroot -type f -printf "%s %P\n"| while read size file; do
    dest="$offroot/$file"
    source="$cdroot/$file"
    size=`encode64 $size`
    touch -r "$source" "$dest"
    chmod a-w "$dest"
    setfattr -n offlinefs.mediumid -v "$mid" "$dest"
    setfattr -n offlinefs.phid -v "$file" "$dest"
    setfattr -n offlinefs.size -v "$size" "$dest"
#    rm "$backend/$file"
done
echo "Done"

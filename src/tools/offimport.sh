#!/bin/bash
#     Copyright (C) 2008 Francisco Jerez
#     This file is part of offlinefs.

#     offlinefs is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.

#     offlinefs is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

usage(){
    echo "usage: offimport.sh -i <cdroot> -l <label> [-s <insertion script>] [-o <offroot>] [-d <dbroot>]"
    echo "Import a directory tree (<cdroot>) into the offlinefs mounted at <offroot> and with database at <dbroot>. <insert script> will get called when trying to access any of the imported files."
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
[ -z $offroot ] && offroot=`mount |grep offlinefs |cut -d' ' -f3 |head -n 1` 
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

echo "Adding files to the database..."
find $cdroot -type f -printf "%P\n"| while read file; do
    source="$cdroot/$file"
    offmedia --addfile "$mid" "$file" "$source" "$dbroot"
done
echo "Done"

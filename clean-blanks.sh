#!/bin/sh
#
# We want to remove duplicated blank lines and blank lines at the end of a file (in Unix means).
# The latter cannot be directly performed by sed, so we add additional two blank lines at the end of
# each file, remove duplicated blank lines and then remove one blank line at the end of each file.
#

files=`git ls-files | \
       egrep -v '^include/|^doc/licences|\.patch$|\.po$' | \
       egrep -v 'README' | \
       egrep -v 'header\.glsl'`

for file in $files; do
  # Remove trailing blanks at the end of line.
  sed -r 's|[ \t]*$||' -i $file

  # Remove duplicated empty lines.
  printf '\n\n' >> $file
  sed -r '/./,/^$/ !d' -i $file
  sed -r '$ d' -i $file
done

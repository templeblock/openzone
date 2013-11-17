#!/bin/sh
#
# autogen.sh
#
# Some targets have lots of source files and it would be hard to keep them all consistent and
# up-to-date manually. Lists of source files in corresponding CMakeLists.txt are thus generated by
# this script.
#
# Additionally this scripts updates version numbers in various files.
#

version=0.3.84
components=( ozCore ozDynamics ozEngine ozFactory
             common matrix nirvana modules client builder
             unittest )

# Generate CMakeLists.txt files.
for component in ${components[@]}; do
  echo "Generating src/$component/CMakeLists.txt"

  cd src/$component

  sources=`echo *.{hh,cc} */*.{hh,cc}`
  # Remove uninstantiated *.hh, *.cc, */*.hh and */*.cc expressions.
  sources=`echo $sources | sed -r 's|(\*/)?\*\...||g'`
  # Remove PCH trigger library.
  sources=`echo $sources | sed -r 's|pch\.cc||g'`
  # Remove duplicated spaces that may have been introduced by the previous removals.
  sources=`echo $sources | sed -r 's| +| |g'`
  # Make file list newline-separated and indented.
  sources=`echo $sources | sed -r 's| |\\\\n  |g'`

  # Insert source file list between "#BEGIN SOURCES" and "#END SOURCES" tags in CMakeLists.txt.
  sed -r '/^#BEGIN SOURCES$/,/^#END SOURCES$/ c\#BEGIN SOURCES\n  '"$sources"'\n#END SOURCES' \
      -i CMakeLists.txt

  cd ../..
done

# Fix version numbers.
echo "Updating version in CMakeLists.txt"
sed -r 's|^(set\( OZ_VERSION ).*$|\1'"$version"' )|' -i CMakeLists.txt

echo "Updating version in doc/Doxyfile*"
sed -r 's|^(PROJECT_NUMBER *= *).*$|\1"'"$version"'"|' -i doc/Doxyfile*

echo "Updating HTML READMEs doc/*.html"
sed -r 's|(<!--OZ_VERSION-->)[^<"]*|\1'"$version"'|' -i doc/*.html

echo "Updating version in etc/openzone.spec"
sed -r 's|^(Version: *).*$|\1'"$version"'|' -i etc/openzone.spec

echo "Updating version in etc/PKGBUILD*"
sed -r 's|^(pkgver=).*$|\1'"$version"'|' -i etc/PKGBUILD*

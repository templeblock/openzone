#!/bin/sh
#
# autogen.sh
#
# Some targets have lots of source files and it would be hard to keep them all consistent and
# up-to-date manually. Lists of source files in corresponding CMakeLists.txt are thus generated by
# this script.
#
# Additionally this scripts also updates version numbers in various files.
#

version=0.3.81
components=( ozCore ozDynamics common matrix nirvana modules client builder unittest )

# Generate CMakeLists.txt files.
for component in ${components[@]}; do
  echo "Generating src/$component/CMakeLists.txt"

  cd src/$component
  # The sed statement removes uninstantiated *.hh, *.cc, */*.hh and */*.cc expressions and makes the
  # file list newline-separated and indented.
  sources=`echo *.{hh,cc} */*.{hh,cc} | sed -r 's| (\*/)?\*\...||g; s| |\\\\n  |g'`

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

echo "Updating version in etc/PKGBUILD*"
sed -r 's|^(pkgver=).*$|\1'"$version"'|' -i etc/PKGBUILD*

echo "Updating version in etc/openzone.spec"
sed -r 's|^(Version: *).*$|\1'"$version"'|' -i etc/openzone.spec

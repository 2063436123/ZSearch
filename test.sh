build=$1 # $1 maybe 'cmake-build-debug'

cd $build || exit 1

for file in *; do
  if [ -f $file ]; then
    if [[ $file == gtest* ]]; then
        echo "testing $file ..."
      ./$file
      if [[ $? -ne 0 ]]; then
        echo "$file failed."
        exit 1
      fi
    fi
  fi
done

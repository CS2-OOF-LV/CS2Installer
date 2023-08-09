# 将源码编码从GB18030转换为UTF-8
find . -type f \( -name "*.h" -o -name "*.cpp" \) -exec sh -c 'iconv -f GB18030 -t UTF-8 "{}" > "{}.utf8" && mv "{}.utf8" "{}"' \;
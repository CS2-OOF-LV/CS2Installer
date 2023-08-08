# 将源码编码从UTF-8转换为GB18030
find . -type f \( -name "*.h" -o -name "*.cpp" \) -exec sh -c 'iconv -f UTF-8 -t GB18030 "{}" > "{}.gb18030" && mv "{}.gb18030" "{}"' \;

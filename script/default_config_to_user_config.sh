#!/bin/bash


# 输入文件和输出文件
input_file="../src/sox_config.h"
output_file="../src/sox_user_config.h"

# 检查输入文件是否存在
if [ ! -f "$input_file" ]; then
    echo "错误: 输入文件 $input_file 不存在"
    exit 1
fi


echo "#warning \"<this file is generate by default_config_to_user_config.sh>, \
your are useing default config, \
please define SOX_USER_CONFIG_FILE to your own config file,\
or delete this warning, edit this file accord your project\"" > "$output_file"

echo "#ifndef SOX_USER_CONFIG_H" >> "$output_file"
echo "#define SOX_USER_CONFIG_H" >> "$output_file"



# 处理文件，只输出脚本区域处理后的内容
awk '
# 标志变量，标记是否在脚本处理区域内
in_script_section == 0 && /\/\/ script process start/ {
    in_script_section = 1
    next
}

in_script_section == 1 && /\/\/ script process end/ {
    in_script_section = 0
    exit  # 处理完脚本区域就退出
}

# 在脚本处理区域内
in_script_section == 1 {
    # 如果行包含 #ifndef，开始记录这个条件块
    if (/^[[:space:]]*#ifndef/) {
        in_ifndef_block = 1
        next
    }
    
    # 如果在一个 #ifndef 块中
    if (in_ifndef_block == 1) {
        # 如果遇到 #endif，结束这个块
        if (/^[[:space:]]*#endif/) {
            in_ifndef_block = 0
            next
        }
        
        # 如果遇到 #define，输出它
        if (/^[[:space:]]*#define/) {
            print
            next
        }
        
        # 跳过其他行（如空行、注释等）
        next
    }
    
    # 不在任何 #ifndef 块中的行，直接输出
    print
}
' "$input_file" >> "$output_file"


echo "#endif" >> "$output_file"





echo "处理完成,处理后的内容-->: $output_file"
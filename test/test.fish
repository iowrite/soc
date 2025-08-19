#! /usr/bin/fish

# 初始化静态计数器
set -g run_in_tempdir_counter 0

function run_in_tempdir
    # 获取命令和参数
    set cmd $argv[1]
    set args $argv[2..-1]
    
    # 增加计数器
    set -g run_in_tempdir_counter (math $run_in_tempdir_counter + 1)
    
    # 创建带序号的工作目录
    set tempdir "work_dir/$run_in_tempdir_counter"
    mkdir -p $tempdir
    echo "Created directory: $tempdir"

    # 在后台执行命令并清理目录
    begin
        pushd $tempdir > /dev/null
        eval $cmd $args
        popd > /dev/null
        echo "Cleaned up directory: $tempdir"
    end &
    
    # 返回临时目录路径
    echo $tempdir
end

rm -rf "./work_dir/*"

run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d1c/chg.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d2c/chg.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d3c/chg.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d4c/chg.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d5c/chg.xlsx -s 0

# 等待所有后台任务完成
wait
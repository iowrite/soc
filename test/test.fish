#!/usr/bin/fish

# 初始化静态计数器
set -g run_in_tempdir_counter 0

function run_in_tempdir
    # 获取命令和参数
    set cmd $argv[1]
    set args $argv[2..-1]
    
    # 增加计数器并创建唯一目录
    set -g run_in_tempdir_counter (math $run_in_tempdir_counter + 1)
    set tempdir "work_dir/$run_in_tempdir_counter"
    mkdir -p $tempdir
    echo "Created directory: $tempdir"

    # 在子shell中执行命令并放入后台
    fish -c "
        cd $tempdir
        $cmd $args &
    "
    
    # 返回临时目录路径
    echo $tempdir
end

# 清理工作目录
rm -rf ./work_dir/*

# batch run








# -15 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/-15d1c/dsg.xlsx -s 100 -c


# -5 degree discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/-5d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/-5d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/-5d5c/dsg.xlsx -s 100 -c


# 5 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d1c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d2c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d4c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/5d5c/dsg.xlsx -s 100 -c



# 15 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d1c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d2c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d2c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d4c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/15d5c/dsg.xlsx -s 100 -c

# 25 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d1c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d2c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d3c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d4c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d5c/chg_retest.xlsx -s 0 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d2c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d4c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/25d5c/dsg.xlsx -s 100 -c

# 35 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d1c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d2c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d3c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d4c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d5c/chg_retest.xlsx -s 0 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d2c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d4c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/35d5c/dsg.xlsx -s 100 -c

# 45 degree charge and discharge
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d1c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d2c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d3c/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d5c/chg.xlsx -s 0 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d1c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d2c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d3c/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/45d5c/dsg.xlsx -s 100 -c


# self test
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/0_chg_test_3.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/0_chg4.xlsx -s 0
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/10_chg_dsg.xlsx -s 10
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/95_dsg_test.xlsx -s 95
run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/100_dsg4.xlsx -s 100
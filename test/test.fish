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
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/0_chg_test_3.xlsx -s 0
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/0_chg4.xlsx -s 0
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/10_chg_dsg.xlsx -s 10
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/95_dsg_test.xlsx -s 95
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test/100_dsg4.xlsx -s 100


# self test factory
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/0_25d_chg.xlsx -s 0
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/12_5d_chg.xlsx -s 12
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/13_35d_chg.xlsx -s 13
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/96_35d_dsg.xlsx -s 96
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/99_5d_dsg.xlsx -s 99
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/self_test_factory/99_25d_dsg.xlsx -s 99

# ZhangLei test: first round
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/-5d/dsg.xlsx -s 100 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/5d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/5d/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/5d/0_dynamic.xlsx -s 0

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/15d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/15d/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/15d/dsg_retest_1.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/15d/dsg_retest_2.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/15d/0_dynamic.xlsx -s 0

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/25d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/25d/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/25d/100_dynamic.xlsx -s 100

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/0_dynamic.xlsx -s 0
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/100_dynamic.xlsx -s 100

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/dsg.xlsx -s 100 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/0_dynamic.xlsx -s 0
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test/zl_test_1/data/35d/100_dynamic.xlsx -s 100


# ZhangLei test: second round
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/5d/dsg.xlsx -s 100 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/15d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/15d/dsg.xlsx -s 100 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/25d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/25d/dsg.xlsx -s 100 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/35d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/35d/dsg.xlsx -s 100 -c

# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/45d/chg.xlsx -s 0 -c
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/data/test2/45d/dsg.xlsx -s 100 -c


# cat /home/hm/Desktop/mysoc/data/test2/diynamic_long_time/low_temp_part_* > /home/hm/Desktop/mysoc/data/test2/diynamic_long_time/50_long_low_temp.xlsx
# cp /home/hm/Desktop/mysoc/data/test2/diynamic_long_time/50_long_low_temp.xlsx /home/hm/Desktop/mysoc/test/work_dir/
# run_in_tempdir /home/hm/Desktop/mysoc/test/mysoc -i /home/hm/Desktop/mysoc/test/work_dir/50_long_low_temp.xlsx -s 50
clear;
cellchg = readtable('cellchgl.xlsx');


global kcurve
global chg_curl_table
load chg_k_curve.mat
load chg_curve.mat

soc_cell = zeros(height(cellchg), 16);
soc_cell_vol = zeros(height(cellchg), 16);

cal = zeros(height(cellchg), 1);
kcal = zeros(height(cellchg), 1);



soc_cell_vol(:,1) = cellchg.cel_vol_1;
soc_cell_vol(:,2) = cellchg.cel_vol_2;
soc_cell_vol(:,3) = cellchg.cel_vol_3;
soc_cell_vol(:,4) = cellchg.cel_vol_4;
soc_cell_vol(:,5) = cellchg.cel_vol_5;
soc_cell_vol(:,6) = cellchg.cel_vol_6;
soc_cell_vol(:,7) = cellchg.cel_vol_7;
soc_cell_vol(:,8) = cellchg.cel_vol_8;
soc_cell_vol(:,9) = cellchg.cel_vol_9;
soc_cell_vol(:,10) = cellchg.cel_vol_10;
soc_cell_vol(:,11) = cellchg.cel_vol_11;
soc_cell_vol(:,12) = cellchg.cel_vol_12;
soc_cell_vol(:,13) = cellchg.cel_vol_13;
soc_cell_vol(:,14) = cellchg.cel_vol_14;
soc_cell_vol(:,15) = cellchg.cel_vol_15;
soc_cell_vol(:,16) = cellchg.cel_vol_16;




real = readtable("std_chg_0.5c.xlsx");


for j = 1:16

    [soc_cell_tmp,soc_er2,cal(1), kcal(1)] = socEKF(50, 2500, cellchg.cur(1), soc_cell_vol(1,j), 1/3600);
    soc_cell(1,j) = soc_cell_tmp;

    for i = 2:height(cellchg)
        if abs(cellchg.cur(i)) > 1
           [soc_cell_tmp, soc_er2, cal(i), kcal(i)]  = socEKF(soc_cell_tmp, soc_er2, cellchg.cur(i), soc_cell_vol(i,j), 1/3600);
           if soc_cell_tmp > 100
               soc_cell_tmp = 100;
           end
           soc_cell(i, j) = soc_cell_tmp;
        else
           soc_cell(i, j) = soc_cell(i-1, j);
        end
        
    end

    subplot(4, 4, j)
    hold on
    plot(soc_cell(:, j))
    plot(real.cap)
    

end


% plot(soc_cell(1, :))
% hold on
% 








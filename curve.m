
load chg_curve.mat

figure
scatter(chg_curl_table.soc, chg_curl_table.vol);

kcurve = zeros(101,1);

for i = 2:height(chg_curl_table)-1
    disp('=====================')
    curvetmp = [chg_curl_table.vol(i-1) chg_curl_table.vol(i) chg_curl_table.vol(i+1)];
    disp(curvetmp)
    x = [chg_curl_table.soc(i-1) chg_curl_table.soc(i) chg_curl_table.soc(i+1)];
    disp(x)
    p = polyfit(x, curvetmp, 2);
    disp(p)
    y_fit = polyval(p, x);
    disp(y_fit);
    if i == 2
        kcurve(1) = 2*p(1)*chg_curl_table.soc(i-1) + p(2);
    end
    kcurve(i) = 2*p(1)*chg_curl_table.soc(i) + p(2);
    if i == 100
        kcurve(101) = 2*p(1)*chg_curl_table.soc(i+1) + p(2);
    end
     
end



for i = 1:101
    if kcurve(i) < 0.1
        kcurve(i) = 0.1;
    end
end

figure
scatter(0:100, kcurve);



save('chg_k_curve.mat', 'kcurve')
clear;



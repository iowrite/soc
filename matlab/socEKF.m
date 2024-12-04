function [soc, soc_er2, cal, kcal] = socEKF(soc_last, soc_er2_last, cur, vol, difft)
    global kcurve
    global chg_curl_table
    soc_cal = soc_last + difft*cur;
    soc_er2_cal = soc_er2_last + (50*0.01/100)^2;
    if soc_cal < 0
        soc_cal = 0;
    end

    if soc_cal > 100
        soc_cal = 100;
        H = kcurve(fix(soc_cal)+1);
        vol_est = chg_curl_table.vol(fix(soc_cal)+1);
    else
        % disp(soc_cal)
        % disp(fix(soc_cal)+1)
        H_prev = kcurve(fix(soc_cal)+1);
        H_next = kcurve(fix(soc_cal)+2);
        H = H_prev + (soc_cal-fix(soc_cal))*(H_next-H_prev);
        vol_est_prev = chg_curl_table.vol(fix(soc_cal)+1);
        vol_est_next = chg_curl_table.vol(fix(soc_cal)+2);
        vol_est = vol_est_prev + (soc_cal-fix(soc_cal))*(vol_est_next-vol_est_prev);
    end
    K = soc_er2_cal*H /(H*soc_er2_cal*H+10*10);

    cal = difft*cur/100;
    kcal = K*(vol-vol_est);

    soc = soc_cal + K*(vol-vol_est);

    soc_er2 = (1-K*H)*soc_er2_cal;
    update  = 1;
    if soc < 0
        soc = 0;
    end
    if soc > 100
         soc = 100;   
    end
    
end
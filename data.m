filename = 'bms_chg_0.5c.xlsx'; 
opts = detectImportOptions(filename); 
opts = setvaropts(opts, 'time', 'DatetimeFormat', 'hh:mm:ss.SSS');
bmschg0 = readtable(filename, opts);




filename = 'std_chg_0.5c.xlsx'; 
opts = detectImportOptions(filename); 
opts = setvaropts(opts, 'time', 'DatetimeFormat', 'hh:mm:ss.SSS');
stdchg0 = readtable(filename, opts);






for i = 1:height(stdchg0)
    milliseconds = rem(stdchg0.time(i).Second, 1) * 1000;
    millisecondsInt = round(milliseconds);
    if millisecondsInt == 1000
        stdchg0.time(i).Second = stdchg0.time(i).Second + 1;
    end
    stdchg0.time(i) = stdchg0.time(i)-seconds(81);
    stdchg0.time(i).Second = floor(stdchg0.time(i).Second);
end


for i = 1:height(bmschg0)
    milliseconds = rem(bmschg0.time(i).Second, 1) * 1000;
    millisecondsInt = round(milliseconds);
    if millisecondsInt == 1000
        bmschg0.time(i).Second = bmschg0.time(i).Second + 1;
    end
     bmschg0.time(i).Second = floor(bmschg0.time(i).Second);
end








newTable = table('Size', [101, 3], 'VariableTypes', {'datetime', 'double', 'double'}, ...
                'VariableNames', {'time', 'cur', 'cap'});
for i = 0:100
    minDiff = abs(i-stdchg0.cap(1));
    minDiff_j = 1;
    for j = 1:height(stdchg0)
        if abs(i-stdchg0.cap(j)) < minDiff
            minDiff = abs(i-stdchg0.cap(j));
            minDiff_j = j;
        end
    end
    newTable(i+1, :)= stdchg0(minDiff_j, :);
end

chg_curl = zeros(101,2);
chg_curl(:,1) = 0:100;
chg_curl_table = array2table(chg_curl, 'VariableNames', {'soc', 'vol'});


for i = 0:100
    for j = 1:height(bmschg0)
        if newTable.time(i+1) == bmschg0.time(j)

            % disp("====")
            % disp(i)
            % disp(newTable.time(i+1))
            % disp(bmschg0.time(j))
            % disp(bmschg0.avg_vol(j))
            chg_curl_table.vol(i+1) = bmschg0.avg_vol(j);
        end
    end
end


% scatter(chg_curl_table.soc, chg_curl_table.vol)

save('chg_curve.mat', 'chg_curl_table');

clear;





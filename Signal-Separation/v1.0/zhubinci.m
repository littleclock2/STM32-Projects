close all
% 读取 data.log 文件
data = load('D:\Desktop\ElectricDesign\NationalCompetition\SETP-Tester_\keil\data.log');
data = uint32(data);
%data(1)=0;
data = sym(data);
figure;
plot(data, 'LineWidth', 1.5, 'MarkerSize', 4);
title('Data.log 数据趋势图');
xlabel('数据点序号');
ylabel('数值');
grid on;

% 设置坐标轴范围（根据数据范围自动调整）
axis tight;
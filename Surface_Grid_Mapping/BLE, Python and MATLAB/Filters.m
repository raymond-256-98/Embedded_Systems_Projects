% MATLAB Script to compare Butterworth, Chebyshev I, and Bessel Filters

% Sampling Frequency
fs = 10e9;          % Sampling Frequency (10 GHz)
order = 5;          % Filter Order for all Filters
cutoff = 2e9;       % Cutoff Frequency (2 GHz)

% Normalized Cutoff Frequency for Digital Filter Design
Wn = cutoff / (fs / 2);

% Design Butterworth Filter
[b_butter, a_butter] = butter(order, Wn, 'low');
[H_butter, f] = freqz(b_butter, a_butter, 1024, fs);

% Design Chebyshev Type I Filter
[b_cheby1, a_cheby1] = cheby1(order, 0.5, Wn, 'low');
[H_cheby1, ~] = freqz(b_cheby1, a_cheby1, 1024, fs);

% Design Bessel Filter using Bilinear Transformation
[b_bessel, a_bessel] = besself(order, 2 * pi * cutoff);
[b_bessel_d, a_bessel_d] = bilinear(b_bessel, a_bessel, fs);
[H_bessel, ~] = freqz(b_bessel_d, a_bessel_d, 1024, fs);

% Plotting the Frequency Responses of the Filters
figure;
plot(f/1e9, 20*log10(abs(H_butter)), 'k', 'LineWidth', 1.5); % Butterworth (Black)
hold on;
plot(f/1e9, 20*log10(abs(H_cheby1)), 'r', 'LineWidth', 1.5); % Chebyshev I (Red)
plot(f/1e9, 20*log10(abs(H_bessel)), 'b', 'LineWidth', 1.5); % Bessel (Blue)
hold off;

% Plot Customizations
title('Comparison of Butterworth, Chebyshev Type I, and Bessel Filters', 'FontWeight', 'bold');
xlabel('Frequency (GHz)', 'FontWeight', 'bold');
ylabel('Attenuation (dB)', 'FontWeight', 'bold');
legend({'Butterworth', 'Chebyshev Type I', 'Bessel'}, 'FontWeight', 'bold', 'Location', 'Best');

ax = gca;
ax.XAxis.FontWeight = 'bold';
ax.YAxis.FontWeight = 'bold';

grid on;
ylim([-45 5]); % Set Y-Axis Range
xlim([0 5]); % Set X-Axis Range

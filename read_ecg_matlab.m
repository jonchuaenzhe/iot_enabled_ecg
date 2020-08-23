clear
delete(instrfindall);

arduino = serial('/dev/cu.SLAB_USBtoUART'); % '/dev/cu.usbmodem14301' or '/dev/cu.usbmodem14501' or '/dev/cu.SLAB_USBtoUART'
arduino.Baudrate=115200; % 9600 for Arduino or 115200 for TinyPico
fopen(arduino);

sampleSize = 3000;
a=1;
dataECG = zeros(sampleSize, 1);

while (a <= sampleSize)
    disp(a/sampleSize*100);
    
    ecgRaw = fgets(arduino);
    dataECG(a, 1) = str2double(ecgRaw)/4095 * 3.3; % 1023 * 5 for Arduino or 4095 * 3.3 for TinyPico
    
    a = a + 1;
end

sfz = 160; % Sampling Frequency
ecg = dataECG;
t = 1/sfz * (1:size(ecg, 1));

figure;
plot(t, ecg(:,1));
xlabel('time (sec)');
ylabel('x(t) magnitude');
drawnow;
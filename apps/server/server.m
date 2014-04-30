clear all; close all; clc;
A=[];
t = tcpip('130.89.20.144',4646);
fopen(t);
while(1)
  while (t.bytesavailable==0)
  end;
  A = [A; fread(t,t.bytesavailable/4,'float32');
  B = reshape(A,[2 length(A)/2])';

  if (length(B)>100)
    C = (B(:,1)-mean(B(1:100,1)))/std(B(1:100,1));
    D = (B(:,1)-mean(B(1:100,2)))/std(B(1:100,2));

    figure(1);
    hold off;
    plot(C);
    hold on;
    plot(D,'r-');
    axis tight;
    
    figure(2);
    plot(D-C);
    axis tight;
  end;
end;
fclose(t);
delete(t);

clear all; close all; clc;
d=[];
t = tcpip('130.89.20.144',4646);
fopen(t);
while(1)
  while (t.bytesavailable==0)
  end;
  A = fread(t,t.bytesavailable);
  A = fliplr(reshape(A,4,length(A)/4)');
  a = (((A(:,1)*256)+A(:,2))*256+A(:,3))*256+A(:,4);
  a = (a-8388608)./16777215.*8.192;
  a = reshape(a,2,length(a)/2)';
  d = [d; a];

  figure(1); newplot;
  plot(d(:,1)-d(1,1),'r-'); hold on;
  plot(d(:,2)-d(1,2),'b-');

  if(mod(length(d),100)==0)
    figure(2);
    subplot(2,1,1);
    hist(d(:,1),100);
    subplot(2,1,2);
    hist(d(:,2),100);    
  end;
end;
fclose(t);
delete(t);

lpfiltradius = 4;
sig = 1;

[x,y] = meshgrid((-lpfiltradius:lpfiltradius));
X = x/(2*lpfiltradius +1);
Y = y/(2*lpfiltradius +1);

f = 1/(sig.^2*2*pi)*exp(-(x.^2+y.^2)/(2*sig.^2));

dx = -x/sig.^2 .* f;

F = fftshift(fft2(ifftshift(f)));

figure(1);
subplot(221); surf(f); title f
subplot(222); surf(dx); title dx


figure(2); 
surf(X,Y,abs(F));
xlabel \theta_x
ylabel \theta_y
zlabel |F|


[sum(f(:)) sum(dx(:))]
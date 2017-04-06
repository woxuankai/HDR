hdr = imread('smalloffice.tiff');
%hdr = imread('jersey_solux-3500.tif');
Lin= single(hdr)/65535;


Lcone = 0.2127*Lin(:,:,1) + 0.7152*Lin(:,:,2) + 0.0722*Lin(:,:,3);
Lrod = -0.0602*Lin(:,:,1) + 0.5436*Lin(:,:,2) + 0.3598*Lin(:,:,3);
morethan0 = @(x) (x>0).*x;
lessthan1 = @(x) (x<1).*(x-1)+1;
Lrod = morethan0(Lrod);
Lrod = lessthan1(Lrod);
Lcone = morethan0(Lcone);
Lcone = lessthan1(Lcone);

alpha=0.67;
beta_cone=4;
beta_rod=2;
n=0.8;
Rmax=2.5;

beta_cone_pow_n = beta_cone^n; 
beta_rod_pow_n = beta_rod^n; 
alpha_mul_n = alpha * n;

table_L_pow_n = zeros(1,2^16);
table_L_pow_alpha_mul_n = zeros(1,2^16);
for i=1:2^16
    table_L_pow_n(i) = ((i-1)^n)*(65535)/(65535)^n;
    table_L_pow_alpha_mul_n(i) = ((i-1)^alpha_mul_n)*(65535)/(65535)^alpha_mul_n;
end

Lcone_pow_n = table_L_pow_n(floor(Lcone*65535 + 1 + 0.5));
Lcone_pow_alpha_mul_n = table_L_pow_alpha_mul_n(floor(Lcone*65535 + 1 + 0.5));
%Lcone_pow_n = table_L_pow_n(floor(Lcone*65535 + 1 + 0.5))/65535;
%Lcone_pow_alpha_mul_n = table_L_pow_alpha_mul_n(floor(Lcone*65535 + 1 + 0.5))/65535;
%Lcone_pow_n1 = Lcone.^n;
%Lcone_pow_alpha_mul_n = Lcone.^alpha_mul_n;
Lrod_pow_n = table_L_pow_n(floor(Lrod*65535 + 1 + 0.5));
Lrod_pow_alpha_mul_n = table_L_pow_alpha_mul_n(floor(Lrod*65535 + 1 + 0.5));
%Lrod_pow_n = table_L_pow_n(floor(Lrod*65535 + 1 + 0.5))/65535;
%Lrod_pow_alpha_mul_n = table_L_pow_alpha_mul_n(floor(Lrod*65535 + 1 + 0.5))/65535;
%Lrod_pow_n = Lrod.^n;
%Lrod_pow_alpha_mul_n = Lrod.^alpha_mul_n;


%sigma_cone=(Lcone.^alpha)*beta_cone;
%R_cone=Rmax*(Lcone.^n)./(Lcone.^n+((Lcone.^alpha)*beta_cone).^n+eps);
R_cone=Rmax*Lcone_pow_n...
    ./(Lcone_pow_n+Lcone_pow_alpha_mul_n*beta_cone_pow_n+eps);
%sigma_rod=(Lrod.^alpha)*beta_rod
%R_rod=Rmax*(Lrod.^n)./(Lrod.^n+((Lrod.^alpha)*beta_rod).^n+eps);
R_rod=Rmax*Lrod_pow_n...
    ./(Lrod_pow_n+Lrod_pow_alpha_mul_n*beta_rod_pow_n+eps);


hh=fspecial('gaussian',[21 21],1)-fspecial('gaussian',[21 21],4);
DOG_cone= imfilter(R_cone,hh,'conv','same','replicate');
DOG_rod = imfilter(R_rod,hh,'conv','same','replicate');

KK=2.5;
DOG_cone=R_cone+KK*DOG_cone;
DOG_rod=R_rod+KK*DOG_rod;
maxd=max(DOG_rod(:));
if(maxd<=1)
    DOG_cone=(DOG_cone-min(DOG_cone(:)))/(max(DOG_cone(:))-min(DOG_cone(:)))+eps;
    DOG_rod=(DOG_rod-min(DOG_rod(:)))/(max(DOG_rod(:))-min(DOG_rod(:)))+eps;
end

t=0.1;
%mina = min(a(:));
mina = max(Lcone(:))^(-t);
a=Lcone.^(-t);
w=1./(1-mina+a);
Lout=w.*DOG_cone+(1-w).*DOG_rod;

s=0.8;
RGB(:,:,1)=((Lin(:,:,1)./(Lcone+eps)).^s).*Lout;
RGB(:,:,2)=((Lin(:,:,2)./(Lcone+eps)).^s).*Lout;
RGB(:,:,3)=((Lin(:,:,3)./(Lcone+eps)).^s).*Lout;

hdr = imread('smalloffice.tiff');
Lin= single(hdr)/65535;

hL2R = @(L,Rmax,n,alpha,beta) Rmax*(L.^n)./((L.^n)+((L.^alpha).*beta).^n);
alpha=0.67;
beta_cone=4;
beta_rod=2;
n=0.8;
Rmax=2.5;
index = 0 : (2^16-1);
tableL2R_cone = hL2R(double(index)./65535,Rmax,n,alpha,beta_cone);
tableL2R_rod = hL2R(double(index)./65535,Rmax,n,alpha,beta_rod);



Lcone = 0.2127*Lin(:,:,1) + 0.7152*Lin(:,:,2) + 0.0722*Lin(:,:,3);
Lrod = -0.0602*Lin(:,:,1) + 0.5436*Lin(:,:,2) + 0.3598*Lin(:,:,3);
morethan0 = @(x) (x>0).*x;
lessthan1 = @(x) (x<1).*(x-1)+1;
Lrod = morethan0(Lrod);
Lrod = lessthan1(Lrod);
Lcone = morethan0(Lcone);
Lcone = lessthan1(Lcone);



R_cone = tableL2R_cone(round(Lcone*65535)+1);
R_rod = tableL2R_rod(round(Lrod*65535)+1);

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

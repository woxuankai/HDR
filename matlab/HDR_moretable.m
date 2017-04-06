clear all;
hdr = imread('smalloffice.tiff');
Lin= single(hdr);

%parameters
alpha=0.67;
beta_cone=4;
beta_rod=2;
n=0.8;
Rmax=2.5;
KK=2.5;
t=0.1;
s=0.8;

%table: L2R(container in table is float)
hL2R = @(L,Rmax,n,alpha,beta) single(Rmax.*(L.^n)./((L.^n)+((L.^alpha).*beta).^n));
index = 0 : (2^16-1);
tableL2R_cone = hL2R(double(index)./65535,Rmax,n,alpha,beta_cone);
tableL2R_rod = hL2R(double(index)./65535,Rmax,n,alpha,beta_rod);
for i = 0 : (2^16-1)
    if isnan(tableL2R_cone(i+1))
        tableL2R_cone(i+1) = 0;
    end
    if isnan(tableL2R_rod(i+1))
        tableL2R_rod(i+1) = 0;
    end
end


%RGB2L
Lcone = (0.2127*Lin(:,:,1) + 0.7152*Lin(:,:,2) + 0.0722*Lin(:,:,3))/65535;
Lrod = (-0.0602*Lin(:,:,1) + 0.5436*Lin(:,:,2) + 0.3598*Lin(:,:,3))/65535;

morethan0 = @(x) (x>1.0/65535).*(x-1.0/65536)+1.0/65535;
lessthan1 = @(x) (x<1).*(x-1)+1;
Lrod = morethan0(Lrod);
Lrod = lessthan1(Lrod);
Lcone = morethan0(Lcone);
Lcone = lessthan1(Lcone);




%cal: L2R
R_cone = tableL2R_cone(round(Lcone*65535)+1);
R_rod = tableL2R_rod(round(Lrod*65535)+1);

%generate kernel

%fix DOG
%replace 2D guassian kernel with 1D guassian kernel
gk__ =@(ksize) 0:ksize-1;
gk_  =@(ksize,sigma) exp(-(gk__(ksize)-(ksize-1)/2).^2/(2*sigma^2));
gk = @(ksize,sigma) gk_(ksize,sigma)/sum(gk_(ksize,sigma));
hh = (gk(21,1) - gk(21,4))*KK;
hh(1,11) = hh(1,11) + 1;

%hh=fspecial('gaussian',[21 21],1)-fspecial('gaussian',[21 21],4);
%hh = hh*KK;
%hh(11,11) = hh(11,11) + 1;

%filt

DOG_cone= imfilter(R_cone,hh,'conv','same','replicate');
DOG_rod = imfilter(R_rod,hh,'conv','same','replicate');

%fix dog
%DOG_cone=R_cone+KK*DOG_cone;
%DOG_rod=R_rod+KK*DOG_rod;
%DOG_cone=R_cone+DOG_cone;
%DOG_rod=R_rod+DOG_rod;
maxd=max(DOG_rod(:));
if(maxd<=1)
    DOG_cone=(DOG_cone-min(DOG_cone(:)))/(max(DOG_cone(:))-min(DOG_cone(:)))+eps;
    DOG_rod=(DOG_rod-min(DOG_rod(:)))/(max(DOG_rod(:))-min(DOG_rod(:)))+eps;
end


mina = max(Lcone(:))^(-t);
a=Lcone.^(-t);
w=1./(1-mina+a);
Lout=w.*DOG_cone+(1-w).*DOG_rod;


RGB(:,:,1)=((Lin(:,:,1)/65535./(Lcone+eps)).^s).*Lout;
RGB(:,:,2)=((Lin(:,:,2)/65535./(Lcone+eps)).^s).*Lout;
RGB(:,:,3)=((Lin(:,:,3)/65535./(Lcone+eps)).^s).*Lout;



statisc = ...
    [...
    min(min(Lrod)), max(max(Lrod)), mean(mean(Lrod));...
    min(min(Lcone)), max(max(Lcone)), mean(mean(Lcone)) ;...
    min(min(R_rod)), max(max(R_rod)), mean(mean(R_rod))  ;...
    min(min(R_cone)), max(max(R_cone)), mean(mean(R_cone))...
    ];

%statisc
%close all;
figure;
imshow(RGB)


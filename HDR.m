%pkg load all;
%clear all;
%close all;
% load matlab
% Lin=double(imread('43.bmp'))/255;

%hdr=read_rle_rgbe('smallOffice.hdr');
%Lin = hdr;
%hdr = imread('smalloffice.tiff');
%Lin= single(hdr)/255/255;

%RGBתXYZ
%Xin=0.5141*Lin(:,:,1)+0.3239*Lin(:,:,2)+0.1604*Lin(:,:,3);
%Yin=0.2651*Lin(:,:,1)+0.6702*Lin(:,:,2)+0.0641*Lin(:,:,3);
%Zin=0.0241*Lin(:,:,1)+0.1228*Lin(:,:,2)+0.8444*Lin(:,:,3);
Xin=0.412453*Lin(:,:,1)+0.357580*Lin(:,:,2)+0.180423*Lin(:,:,3);
Yin=0.212671*Lin(:,:,1)+0.715160*Lin(:,:,2)+0.072169*Lin(:,:,3);
Zin=0.019334*Lin(:,:,1)+0.119193*Lin(:,:,2)+0.950227*Lin(:,:,3);


Lcone=Yin;
Lrod=-0.702*Xin+1.039*Yin+0.433*Zin;

morethan0 = @(x) (x>0).*x;
Lrod = morethan0(Lrod);


arfa=0.67;
beita=4;
beita2=2;
sigma_cone=(Lcone.^arfa)*beita;
sigma_rod=(Lrod.^arfa)*beita2;

n=0.8;
Rmax=2.5;
R_cone=Rmax*(Lcone.^n)./(Lcone.^n+sigma_cone.^n+eps);
R_rod=Rmax*(Lrod.^n)./(Lrod.^n+sigma_rod.^n+eps);
%R_cone=Rmax*(Lcone.^n)./(Lcone.^n+sigma_cone.^n);
%R_rod=Rmax*(Lrod.^n)./(Lrod.^n+sigma_rod.^n);



hh=fspecial('gaussian',[21 21],1)-fspecial('gaussian',[21 21],4);%Myresult3 2.5
%DOG_cone= imfilter(abs(R_cone),hh,'conv','same','replicate');
%DOG_rod = imfilter(abs(R_rod),hh,'conv','same','replicate');
DOG_cone= imfilter(R_cone,hh,'conv','same','replicate');
DOG_rod = imfilter(R_rod,hh,'conv','same','replicate');
KK=2.5;
DOG_cone=R_cone+KK*DOG_cone;
DOG_rod=R_rod+KK*DOG_rod;


t=0.1;
a=Lcone.^(-t);
w=1./(1-(min(a(:)))+a);
maxd=max(DOG_rod(:));
if (maxd<=1)
    DOG_cone=(DOG_cone-min(DOG_cone(:)))/(max(DOG_cone(:))-min(DOG_cone(:)))+eps;
    DOG_rod=(DOG_rod-min(DOG_rod(:)))/(max(DOG_rod(:))-min(DOG_rod(:)))+eps;
end    
Lout=w.*DOG_cone+(1-w).*DOG_rod;



s=0.8;
RGB(:,:,1)=((Lin(:,:,1)./(Lcone+eps)).^s).*Lout;
RGB(:,:,2)=((Lin(:,:,2)./(Lcone+eps)).^s).*Lout;
RGB(:,:,3)=((Lin(:,:,3)./(Lcone+eps)).^s).*Lout;


%imshow(RGB);
%rgb1 = RGB;
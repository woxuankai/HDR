#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import cv2
import numpy as np




alpha = 0.67;
beta_cone = 4;VideoCapture([0, 1, 0],dtype='float32')
# XYZ2L_rod = np.array([-0.702, 1.039, 0.433],dtype='float32')
# BGR2L_cone = BGR2XYZ.dot(XYZ2L_cone)
# BGR2L_rod = BGR2XYZ.dot(XYZ2L_rod)
BGR2L_cone = np.array([0.0722, 0.7152, 0.2127], dtype='float32')
BGR2L_rod = np.array([0.3598, 0.5436, -0.0602], dtype='float32')
BGR2L_cone.shape = (-1,1)
BGR2L_rod.shape = (-1,1)

h1 = cv2.getGaussianKernel(21,1)
hh1 = h1.dot(h1.T)
h2 = cv2.getGaussianKernel(21,4)
hh2 = h2.dot(h2.T)
hh = hh1 - hh2
hh[10,10] = KK*hh[11,11] + 1

def hdr_do(src):
    assert(src.dtype == np.dtype('uint8'))
    src = np.array(src, dtype='float32') / 255
    
    L_cone = np.squeeze(src.dot(BGR2L_cone))
    L_rod = np.squeeze(src.dot(BGR2L_rod))

    ret, L_cone = \
        cv2.threshold(L_cone, 0, 0,type=cv2.THRESH_TOZERO, dst=L_cone)
    ret, L_rod = \
        cv2.threshold(L_rod, 0, 0, type=cv2.THRESH_TOZERO, dst=L_rod)

    sigma_cone = beta_cone * np.power(L_cone, alpha)
    sigma_rod = beta_rod * np.power(L_rod, alpha)
    
    eps = np.finfo(np.float32).eps
    
    R_cone = R_max*np.divide(np.power(L_cone, n), eps + \
                             np.power(sigma_cone, n) + np.power(L_cone, n))
    R_rod = R_max*np.divide(np.power(L_rod, n), eps + \
                            np.power(sigma_rod, n) + np.power(L_rod, n))
    
    G_cone = cv2.filter2D(R_cone, -1, hh)
    G_rod = cv2.filter2D(R_rod, -1, hh)

    # cv2.GaussianBlur(src,(21,21),1) - cv2.GaussianBlur(src,(21,21),4)
    # not linear seperable, thus cannot use sepFilter

    a = np.power(L_cone, -t)
    a = np.subtract(a, np.min(a.ravel()), out=a)
    w = 1/(1 + a)
    Lout = w*G_cone + (1-w)*G_rod
    rst = src*np.expand_dims(Lout/np.power(L_cone,s), axis=3)
    
    return rst

if __name__ == '__main__':
    if(len(sys.argv) > 1):
        ifvideo = False
        src = cv2.imread(sys.argv[1])
        cv2.imshow('input',src.copy())
        rst = hdr_do(src)
        cv2.imshow('output',rst)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    else:
        ifvideo = True
        cap = cv2.VideoCapture(0)
        cap.set(cv2.CAP_PROP_FRAME_WIDTH,640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT,480)
        while(True):
            ret, src = cap.read()
            cv2.imshow('input',src)
            rst = hdr_do(src)
            # rst = src
            cv2.imshow('output',rst)
            if cv2.waitKey(1)& 0xFF == ord('q'):
                break
        cap.release()
        cv2.destroyAllWindows()




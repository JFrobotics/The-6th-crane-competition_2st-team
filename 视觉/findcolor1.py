from shapedetector import ShapeDetector
import numpy as np
import cv2
import cv2 as cv
import imutils
import time
import serial
capture=cv.VideoCapture(0)
#capture.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M', 'J', 'P', 'G'))
kernel=np.ones((5,5),np.uint8)
ser = serial.Serial('/dev/ttyAMA0',115200,timeout=1)
lower_red = np.array([160, 135, 42])     #红色阈值下界
higher_red = np.array([179, 255, 255])    #红色阈值上界

lower_blue = np.array([93, 95, 50])      #蓝色阈值下界
higher_blue = np.array([139, 255, 255]) #蓝色阈值上界
def video_demo():
    
    while(True):
        ref,frame=capture.read()
        start=time.perf_counter()
        Area=2800
        sd = ShapeDetector()
        img_hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        mask_red = cv2.inRange(img_hsv, lower_red, higher_red)  # 可以认为是过滤出红色部分，获得红色的掩膜  
        mask_blue = cv2.inRange(img_hsv, lower_blue, higher_blue)  # 获得blue色部分掩膜

        gray_red = cv2.morphologyEx(mask_red, cv2.MORPH_OPEN, kernel)
        gray_blue = cv2.morphologyEx(mask_blue, cv2.MORPH_OPEN, kernel)
        
        edges_red = cv2.Canny(gray_red, 50, 100)  # 边缘识别
        edges_blue = cv2.Canny(gray_blue, 50, 100)  # 边缘识别
        
        ret,red=cv2.threshold(edges_red,234,255,0)
        ret,blue=cv2.threshold(edges_blue,234,255,0)
        
        cnts1, hierarchy1 = cv2.findContours(red, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)#轮廓检测红色
        cnts2, hierarchy2 = cv2.findContours(blue, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)#轮廓检测蓝色
        for cnt in cnts1:
            area=cv2.contourArea(cnt)
            
            if area>Area:
                print(area)
                shape=sd.detect(cnt,edges_red)
            #cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                if shape==3 :
                    ser.write('T'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter()
                    print('T')
                if shape==4 :
                    ser.write('Q'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter()
                    print('Q')
                if shape==5 :
                    ser.write('R'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter()
                    print('R')
            
        for cnt in cnts2:
            area=cv2.contourArea(cnt)
            if area>Area:
                shape=sd.detect(cnt,edges_blue)
                print(area)
                if shape==3 :
                    ser.write('t'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter()
                   
                    print('t')
                if shape==4 :
                    ser.write('q'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter()
                    print('q')
                if shape==5 :
                    ser.write('r'.encode("utf-8"))
                    cv2.drawContours(frame,[cnt], -1, (0, 255, 0), 2)
                    cv2.imshow('frame', frame)
                    
                    end=time.perf_counter() 
                    print('r')  
            
            #                 cv2.imshow('frame', frame)
        
        c= cv.waitKey(30) & 0xff
        if c==27:
            capture.release()
            break


video_demo()
cv.waitKey()
cv.destroyAllWindows()



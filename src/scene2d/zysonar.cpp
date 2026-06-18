#include "zysonar.h"

/*!
 **************************************************************************
 * @file
 * @brief           声呐滚动图像的图形项
 * @author     		Toslon-ZY
 * @date            2017.06.07
 * @version 		V0.1
 * @par 			Copyright (c): 2017 Toslon.Co.Ltd. All rights reserved.
 * @note            图像缓存数组内的数据进行滚动，可提高程序运行效率
 **************************************************************************
*/
#define COLOR_LINE 7   //偏移量

/*---------------------------------------------------------------------------------------------------*/
// ZySonarImage::ZySonarImage(QWidget *parent, int imageHeight, QString mode)
// {
//     setMouseTracking(true);

//     sonarImageWidth = 512;
//     sonarImageHeight = imageHeight;
//     initImageBuffer();

//     currentViewerMode = mode;

//     slideScanBoat_.load(":/navi/boat.png");
//     boatWidth_ = slideScanBoat_.width();
//     boatHeight_ = slideScanBoat_.height();

// }
// ZySonarImage::~ZySonarImage(){}


// //初始化图像缓冲区
// void ZySonarImage::initImageBuffer()
// {
//     sonarImageBuffer = new int[sonarImageWidth*sonarImageHeight];
//     sonarImage = new QImage((uchar *)sonarImageBuffer, sonarImageWidth, sonarImageHeight, QImage::Format_RGB32);
//     sonarImage->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);

//     sonarImageBuffer_2 = new int[sonarImageWidth*sonarImageHeight];
//     sonarImage_2 = new QImage((uchar *)sonarImageBuffer_2, sonarImageWidth, sonarImageHeight, QImage::Format_ARGB32);
//     sonarImage_2->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);

//     sonarImageBuffer_channel1 = new int[sonarImageWidth*sonarImageHeight];
//     sonarImage_channel1 = new QImage((uchar *)sonarImageBuffer_channel1, sonarImageWidth, sonarImageHeight, QImage::Format_RGB32);
//     sonarImage_channel1->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);

//     sonarImageBuffer_channel2 = new int[sonarImageWidth*sonarImageHeight];
//     sonarImage_channel2 = new QImage((uchar *)sonarImageBuffer_channel2, sonarImageWidth, sonarImageHeight, QImage::Format_RGB32);
//     sonarImage_channel2->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
// }


// void ZySonarImage::reInitImageBuffer(int width)
// {
//     sonarImageWidth = width;

//     delete sonarImageBuffer;
//     delete sonarImage;

//     delete sonarImageBuffer_2;
//     delete sonarImage_2;

//     delete sonarImageBuffer_channel1;
//     delete sonarImage_channel1;

//     delete sonarImageBuffer_channel2;
//     delete sonarImage_channel2;

//     initImageBuffer();
// }

// void ZySonarImage::reFillBackgroundColor()
// {
//     sonarImage->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     sonarImage_2->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     sonarImage_channel1->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     sonarImage_channel2->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);

//     list_bottomLinePos.clear();
//     list_bottomLinePos_2.clear();
//     list_bottomLinePos_channel1.clear();
//     list_bottomLinePos_channel2.clear();

//     list_filterLinePos.clear();
//     list_filterLinePos_2.clear();
//     list_filterLinePos_channel1.clear();
//     list_filterLinePos_channel2.clear();

//     list_markingPos.clear();
//     markLineIndex = -1;
//     flag_inAlarm = false;
//     flag_notAgain = false;
//     flag_chBox_check = false;
//     this->update();
// }

// void ZySonarImage::setDualFreqMode(bool isDual)
// {
//     isDualFreqMode = isDual;
// }

// void ZySonarImage::setCurrentCorrectColor(QString color)
// {
//     currentCorrectColor = color;
// }

// void ZySonarImage::setCurrentSonarImage(int num)
// {
//     currChannelValue = num;
// }
// int ZySonarImage::getCurrentSonarImage()
// {
//     return currChannelValue;
// }


// QList<int> colorData;
// void ZySonarImage::drawImagePixel(int column, StructSonarInfo sonarInfo, float scale, int colorNum, bool is2nd,int flagViewMode)
// {
//     if(scale <= 0) { return; }

//     /*
//         * 最后两个参数值的不同组合所表示的意思:
//         * 1、 colorNum = 1、is2nd = false 组合:把低频像素数据 赋给 sonarImage，只有一个窗口，并且只绘制一个频率
//         * 2、 colorNum = 2、is2nd = false 组合:把高频像素数据 赋给 sonarImage，意思是分频在两个窗口分别绘制
//         * 3、 colorNum = 2、is2nd = true  组合:把高频像素数据 赋给 sonarImage_2，意思是要在一个窗口中合并绘制双频
//         * colorNum = 3,获取channel1，colorNum = 4,获取channel2
//     */

//     colorData.clear();
//     if(colorNum == 1) {
//         /*-水表-*/
//         for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++)
//         {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }  else  {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_fish[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish[0]);
//                 } else  {
//                     colorData.append(ZyColorScheme::colorScheme_fish[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarImageHeight; j++)
//         {
//             if(sonarInfo.rawData[j] == 0)  {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)  {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[0]);
//                 }  else {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//     }
//     else if(colorNum == 2)
//     {
//         /*-水表-*/
//         for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++)
//         {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++)
//         {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254){
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarImageHeight; j++)
//         {
//             if(sonarInfo.rawData[j] <5) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }  else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[0]);
//                 } else{
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//     }
//     else if(colorNum == 3)
//     {
//         if(flagViewMode == 3)
//         {
//             /*-水表-*/
//             for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageWidth); j++)
//             {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水中-*/
//             for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageWidth)); j++)
//             {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水底-*/
//             for(int j = sonarInfo.btStart; j < sonarImageWidth; j++)
//             {
//                 if(sonarInfo.rawData[j] < 5) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//         }
//         else
//         {
//             /*-水表-*/
//             for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++) {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水中-*/
//             for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++) {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[254]);
//                     }  else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[0]);
//                     }  else {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水底-*/
//             for(int j = sonarInfo.btStart; j < sonarImageHeight; j++) {
//                 if(sonarInfo.rawData[j] < 5) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 }  else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[0]);
//                     }  else {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//         }
//     }
//     else if(colorNum == 4)
//     {
//         if(flagViewMode == 3) {
//             /*-水表-*/
//             for(int j = 0; (j<sonarInfo.sfEnd) && (j<sonarInfo.btStart) && (j<sonarImageWidth); j++){
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)  {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水中-*/
//             for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageWidth)); j++)
//             {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水底-*/
//             for(int j = sonarInfo.btStart; j < sonarImageWidth; j++)
//             {
//                 if(sonarInfo.rawData[j] < 5) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//         }
//         else
//         {
//             /*-水表-*/
//             for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++)
//             {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_surface_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水中-*/
//             for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++)
//             {
//                 if(sonarInfo.rawData[j] == 0) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[0]);
//                     } else {
//                         colorData.append(ZyColorScheme::colorScheme_fish_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//             /*-水底-*/
//             for(int j = sonarInfo.btStart; j < sonarImageHeight; j++)
//             {
//                 if(sonarInfo.rawData[j] < 5) {
//                     colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//                 } else {
//                     if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[254]);
//                     } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[0]);
//                     }  else  {
//                         colorData.append(ZyColorScheme::colorScheme_bottom_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                     }
//                 }
//             }
//         }
//     }

//     if(scale < 1) {
//         scale = 1/scale;
//         compressImagePixel(column, sonarInfo, scale, colorNum, is2nd,flagViewMode);
//     } else {
//         stretchImagePixel(column, sonarInfo, scale, colorNum, is2nd,flagViewMode);
//     }
// }

// //压缩图像像素
// void ZySonarImage::compressImagePixel(int column, StructSonarInfo sonarInfo, float scale, int colorNum,bool is2nd,int flagViewMode)
// {
//     if(is2nd == true)
//     {
//         if(colorNum == 2)
//         {
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 if(colorData[sonarInfo.startIdx+(int)(j*scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex])
//                 {
//                     sonarImageBuffer_2[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_2[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)]+0xFF000000;
//                 }
//             }
//             /*-自动补齐-*/
//             for(; j<sonarImageHeight; j++) {
//                 sonarImageBuffer_2[column+j*sonarImageWidth] = 0x00000000;
//             }
//         }
//         else if(colorNum == 3)
//         {
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 if(colorData[sonarInfo.startIdx+(int)(j*scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex])
//                 {
//                     sonarImageBuffer_channel1[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_channel1[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)]+0xFF000000;
//                 }
//             }

//             /*-自动补齐-*/
//             for(; j<sonarImageHeight; j++) {
//                 sonarImageBuffer_channel1[column+j*sonarImageWidth] = 0x00000000;
//             }

//         }
//         else if(colorNum == 4)
//         {
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 if(colorData[sonarInfo.startIdx+(int)(j*scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex])
//                 {
//                     sonarImageBuffer_channel2[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_channel2[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)]+0xFF000000;
//                 }
//             }
//             /*-自动补齐-*/
//             for(; j<sonarImageHeight; j++) {
//                 sonarImageBuffer_channel2[column+j*sonarImageWidth] = 0x00000000;
//             }
//         }

//     }
//     else
//     {
//         if(colorNum == 3 && flagViewMode == 3)
//         {
//             column = sonarImageHeight-1-column;
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 sonarImageBuffer[sonarImageWidth-1-j+column*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)];
//             }
//             for(; j<sonarImageWidth; j++) {
//                 sonarImageBuffer[sonarImageWidth-1-j+column*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }
//         }
//         else if(colorNum == 4 && flagViewMode == 3)
//         {
//             column = sonarImageHeight-1-column;
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 sonarImageBuffer[j+column*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)];
//             }
//             for(; j<sonarImageWidth; j++) {
//                 sonarImageBuffer[j+column*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }
//         }
//         else //低频模式；高频模式；1、2通道的下扫模式
//         {
//             int j = 0;
//             for(; (((int)(j*scale)+sonarInfo.startIdx) < sonarInfo.rawData.count()); j++) {
//                 sonarImageBuffer[column + j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j*scale)];
//             }
//             /*-自动补齐-*/
//             for(; j<sonarImageHeight; j++) {
//                 /*-底层的部分设置成1不使用透明模式-*/
//                 sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }
//         }
//     }
// }



// //拉伸图像像素
// void ZySonarImage::stretchImagePixel(int column, StructSonarInfo sonarInfo, float scale,int colorNum, bool is2nd,int flagViewMode)
// {
//     if(is2nd == true)
//     {
//         if(colorNum == 2)
//         {

//             for(int j = 0; ((j<sonarImageHeight)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 if(colorData[sonarInfo.startIdx+(int)(j/scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex]) {
//                     sonarImageBuffer_2[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_2[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j/scale)]+0xFF000000;
//                 }
//             }
//         }
//         else if(colorNum == 3)
//         {
//             for(int j = 0; ((j<sonarImageHeight)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 if(colorData[sonarInfo.startIdx+(int)(j/scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex]) {
//                     sonarImageBuffer_channel1[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_channel1[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j/scale)]+0xFF000000;
//                 }
//             }
//         }
//         else if(colorNum == 4)
//         {
//             for(int j = 0; ((j<sonarImageHeight)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 if(colorData[sonarInfo.startIdx+(int)(j/scale)] == ZyColorScheme::background[ZyColorScheme::backgroundIndex]) {
//                     sonarImageBuffer_channel2[column+j*sonarImageWidth] = 0x00000000;
//                 } else {
//                     sonarImageBuffer_channel2[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j/scale)]+0xFF000000;
//                 }
//             }
//         }
//     }

//     else
//     {
//         if(colorNum == 3 && flagViewMode == 3) {
//             column = sonarImageHeight-1- column;
//             for(int j = 0; j<sonarImageWidth; j++)
//             {
//                 sonarImageBuffer[column*sonarImageWidth+sonarImageWidth-1-j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }

//             for(int j = 0; ((j<sonarImageWidth)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 sonarImageBuffer[column*sonarImageWidth+sonarImageWidth-1-j] = colorData[sonarInfo.startIdx+(int)(j/scale)];
//             }
//         }
//         else if(colorNum == 4 && flagViewMode == 3){
//             column = sonarImageHeight-1- column;
//             for(int j = 0; j<sonarImageWidth; j++)
//             {
//                 sonarImageBuffer[column*sonarImageWidth+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }

//             for(int j = 0; ((j<sonarImageWidth)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 sonarImageBuffer[column*sonarImageWidth+j] = colorData[sonarInfo.startIdx+(int)(j/scale)];
//             }
//         }
//         else {
//             for(int j = 0; j<sonarImageHeight; j++)
//             {
//                 sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//             }
//             for(int j = 0; ((j<sonarImageHeight)&&(sonarInfo.startIdx+(int)(j/scale)<colorData.count())); j++)
//             {
//                 sonarImageBuffer[column+j*sonarImageWidth] = colorData[sonarInfo.startIdx+(int)(j/scale)];
//             }
//         }
//     }

// }


// void ZySonarImage::drawImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale)
// {

//     if(scale <= 0) {return;}

//     if(scale < 1)
//     {
//         scale = 1/scale;
//         compressImagePixelOnline(column, sonarInfo, scale);
//     }
//     else
//     {
//         stretchImagePixelOnline(column, sonarInfo, scale);
//     }
// }
// void ZySonarImage::stretchImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale)
// {
//     if(scale <= 0) {
//         return;
//     }

//     if(column == 0) {
//         lastBtStart = sonarInfo.btStart;
//         lastScale = scale;
//         lastOp = "stretch";
//     }
//     if(useBottomLine == true) {
//         // qDebug()<<"scale:" << scale <<"  lastBtStart:" << lastBtStart<<"  lastScale:"<<lastScale;
//         if(scale != lastScale) {
//             if(lastOp == "stretch") lastBtStart = (lastScale /scale) *lastBtStart;
//             if(lastOp == "compress") lastBtStart = lastBtStart /lastScale /scale;
//         }
//         // qDebug() << "afterChange: " << lastBtStart;
//         for(int i=sonarInfo.btStart;i<lastBtStart;i++) {
//             sonarInfo.rawData[i] = 80;
//         }
//         for(int i=lastBtStart;i<sonarInfo.btStart ;i++) {
//             sonarInfo.rawData[i] = 80;
//         }
//     }
//     lastBtStart = sonarInfo.btStart;
//     lastScale = scale;
//     lastOp = "stretch";

//     sonarInfo.btStart *= scale;

//     for(int j=0; j<sonarImageHeight; j++)
//     {
//         if(sonarInfo.rawData[(int)(j/scale)] == 0) {
//             sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         else {
//             sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::onlineColor;
//         }
//     }
// }
// void ZySonarImage::compressImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale)
// {
//     if(scale <= 0) {
//         return;
//     }

//     if(column == 0) {
//         lastBtStart = sonarInfo.btStart;
//         lastScale = scale;
//         lastOp = "compress";
//     }
//     if(useBottomLine == true) {
//         // qDebug()<<"compressImagePixelOnline  scale:"<<scale<<"  lastBtStart:"<<lastBtStart<<"  lastScale:"<<lastScale;
//         if(scale != lastScale) {
//             if(lastOp == "stretch") lastBtStart = lastBtStart *lastScale *scale;
//             if(lastOp == "compress") lastBtStart = (scale /lastScale) *lastBtStart;
//         }
//         // qDebug() << "compressImagePixelOnline  afterChange:" << lastBtStart;
//         for(int i = sonarInfo.btStart;i < lastBtStart;i++) {
//             sonarInfo.rawData[i] = 80;
//         }
//         for(int i = lastBtStart;i < sonarInfo.btStart;i++) {
//             sonarInfo.rawData[i] = 80;
//         }
//     }

//     lastBtStart = sonarInfo.btStart;
//     lastScale = scale;
//     lastOp = "compress";

//     sonarInfo.btStart = sonarInfo.btStart/scale;

//     for(int j = 0;j < sonarImageHeight/scale; j++) {
//         if(sonarInfo.rawData[(int)(j*scale)] == 0) {
//             sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         else {
//             sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::onlineColor;
//         }
//     }

//     /*-自动补齐-*/
//     for(int j = sonarImageHeight/scale; j<sonarImageHeight; j++)
//     {
//         sonarImageBuffer[column+j*sonarImageWidth] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//     }

// }

// void ZySonarImage::refreshTextInfo_AboveImage(float depth, float hdg, float spd, double lon, double lat, float temp)
// {
//     flag_showExtraInfo = false;

//     QString string_depth, string_spd, string_ns, string_lat, string_ew, string_lon, string_hdg, string_temp;
//     string_depth = QString::number(depth, 'f', 2);
//     string_temp = QString::number(temp, 'f', 1);
//     string_spd = QString::number(spd, 'f', 2);
//     string_hdg = QString::number(hdg, 'f', 1);
//     if(lon >= 0) {
//         string_ew = "E";
//         string_lon = QString::number(lon,'f',6);
//     } else {
//         string_ew = "W";
//         string_lon = QString::number(-lon,'f',6);
//     }
//     if(lat >= 0) {
//         string_ns = "N";
//         string_lat = QString::number(lat,'f',6);
//     } else {
//         string_ns = "S";
//         string_lat = QString::number(-lat,'f',6);
//     }

//     info_depth = "Depth:" + string_depth + "m";
//     info_hdg = "HDG:" + string_hdg +"°";
//     info_spd = "SPD:" + string_spd + "m/s" ;
//     info_lon = string_ew + string_lon + "°";
//     info_lat = string_ns + string_lat +"°";
//     info_temp = "Temp:" + string_temp + "℃" ;
// }
// void ZySonarImage::showMarkLine(int x_pos, float depth, double lon, double lat, quint32 time, float alt1, float alt2)
// {
//     markLineIndex = x_pos;

//     QString string_depth, string_ns, string_lat, string_ew, string_lon, string_alt1, string_alt2;

//     string_depth = QString::number(depth, 'f', 2);

//     if(lon >= 0) {
//         string_ew = "E";
//         string_lon = QString::number(lon,'f',6);
//     } else {
//         string_ew = "W";
//         string_lon = QString::number(-lon,'f',6);
//     }

//     if(lat >= 0) {
//         string_ns = "N";
//         string_lat = QString::number(lat,'f',6);
//     } else {
//         string_ns = "S";
//         string_lat = QString::number(-lat,'f',6);
//     }

//     quint16 year = GET_YEAR(time);
//     quint8 month = GET_MONTH(time);
//     quint8 day = GET_DAY(time);
//     quint8 hour = GET_HOUR(time);
//     quint8 minute = GET_MINUTE(time);
//     quint8 second = GET_SECOND(time);

//     string_alt1 = QString::number(alt1, 'f', 2);

//     string_alt2 = QString::number(alt2, 'f', 2);

//     markInfo_depth = "Depth: " + string_depth + "m";
//     markInfo_alt1 = "Ant-Alt: " + string_alt1 + "m";
//     markInfo_alt2 = "Alt_Diff: " + string_alt2 + "m";
//     markInfo_lon =  string_ew + " " + string_lon + "°";
//     markInfo_lat = string_ns + " " + string_lat +"°";
//     markInfo_date = QString("20")+QString("%1").arg(year, 2, 10, QLatin1Char('0'))+QString("/")+QString("%1").arg(month, 2, 10, QLatin1Char('0'))+QString("/")+QString("%1").arg(day, 2, 10, QLatin1Char('0'));
//     markInfo_time = QString("%1").arg(hour, 2, 10, QLatin1Char('0'))+":"+QString("%1").arg(minute, 2, 10, QLatin1Char('0'))+":"+QString("%1").arg(second, 2, 10, QLatin1Char('0'));
// }

// QString ZySonarImage::getMarkLineInfo()
// {
//     QString list;
//     list.clear();
//     list.append(markInfo_depth);list.append(", ");
//     list.append(markInfo_alt1);list.append(", ");
//     list.append(markInfo_alt2);list.append(", ");
//     list.append(markInfo_lon);list.append(", ");
//     list.append(markInfo_lat);list.append(", ");
//     list.append(markInfo_date);list.append(", ");
//     list.append(markInfo_time);list.append(", ");
//     return list;
// }

// void ZySonarImage::setSonarFreqString(QString freq)
// {
//     currentSonarFreq = freq;
// }

// void ZySonarImage::showAlarm()
// {
//     flag_inAlarm = true;
//     this->update();
// }

// void ZySonarImage::hideAlarm()
// {
//     flag_inAlarm = false;
//     if(flag_chBox_check == true)
//     {
//         flag_notAgain = true;
//     }
//     this->update();
// }

// void ZySonarImage::showMarkLine(int x_pos, float depth, double lon, double lat, QString dateTime)
// {
//     markLineIndex = x_pos;

//     QString string_depth, string_ns, string_lat, string_ew, string_lon;

//     string_depth = QString::number(depth, 'f', 2);

//     if(lon >= 0){
//         string_ew = "E";
//         string_lon = QString::number(lon,'f',6);
//     }
//     else{
//         string_ew = "W";
//         string_lon = QString::number(-lon,'f',6);
//     }

//     if(lat >= 0) {
//         string_ns = "N";
//         string_lat = QString::number(lat,'f',6);
//     }
//     else {
//         string_ns = "S";
//         string_lat = QString::number(-lat,'f',6);
//     }

//     QString date = dateTime.left(8);
//     date = date.insert(6,"-").insert(4,"-");

//     QString time = dateTime.right(6);
//     time = time.insert(4,":").insert(2,":");

//     markInfo_depth = "Depth:" + string_depth + "m   ";
//     markInfo_lon =  string_ew + "" + string_lon + "°    ";
//     markInfo_lat = string_ns + "" + string_lat +"°   ";
//     markInfo_date = date;
//     markInfo_time = time;

// }
// void ZySonarImage::hideMarkLine()
// {
//     markLineIndex = -1;
// }
// bool ZySonarImage::markLineIsShow()
// {
//     if(markLineIndex != -1){
//         return true;
//     } else{
//         return false;
//     }
// }

// void ZySonarImage::setUseBottomLine(bool value)
// {
//     useBottomLine = value;
// }
// void ZySonarImage::setChartSpeedLevel(int value)
// {
//     chartSpeedLevel = value;
// }

// void ZySonarImage::markingClear()
// {
//     list_markingPos.clear();
//     list_markingString.clear();
// }
// void ZySonarImage::markingAppend(int pos, QString string)
// {
//     list_markingPos.append(pos);
//     list_markingString.append(string);
// }

// void ZySonarImage::bottomLineClear()
// {
//     list_bottomLineIdx.clear();
//     list_bottomLinePos.clear();
//     list_bottomLineIdx_2.clear();
//     list_bottomLinePos_2.clear();
//     list_bottomLineIdx_channel1.clear();
//     list_bottomLinePos_channel1.clear();
//     list_bottomLineIdx_channel2.clear();
//     list_bottomLinePos_channel2.clear();

// }
// void ZySonarImage::filterLineClear()
// {
//     list_filterLineIdx.clear();
//     list_filterLinePos.clear();
//     list_filterLineIdx_2.clear();
//     list_filterLinePos_2.clear();
//     list_filterLineIdx_channel1.clear();
//     list_filterLinePos_channel1.clear();
//     list_filterLineIdx_channel2.clear();
//     list_filterLinePos_channel2.clear();
// }

// void ZySonarImage::bottomLineAppend(int pos, int btStart)
// {
//     list_bottomLinePos.append(pos);
//     list_bottomLineIdx.append(btStart);
// }

// void ZySonarImage::bottomLineAppend_2(int pos, int btStart)
// {
//     list_bottomLinePos_2.append(pos);
//     list_bottomLineIdx_2.append(btStart);
// }
// void ZySonarImage::bottomLineAppend_channel1(int pos, int btStart)
// {
//     list_bottomLinePos_channel1.append(pos);
//     list_bottomLineIdx_channel1.append(btStart);
// }
// void ZySonarImage::bottomLineAppend_channel2(int pos, int btStart)
// {
//     list_bottomLinePos_channel2.append(pos);
//     list_bottomLineIdx_channel2.append(btStart);
// }

// void ZySonarImage::filterLineAppend(int pos, int btStart)
// {
//     list_filterLinePos.append(pos);
//     list_filterLineIdx.append(btStart);
// }
// void ZySonarImage::filterLineAppend_2(int pos, int btStart)
// {
//     list_filterLinePos_2.append(pos);
//     list_filterLineIdx_2.append(btStart);
// }
// void ZySonarImage::filterLineAppend_channel1(int pos, int btStart)
// {
//     list_filterLinePos_channel1.append(pos);
//     list_filterLineIdx_channel1.append(btStart);
// }
// void ZySonarImage::filterLineAppend_channel2(int pos, int btStart)
// {
//     list_filterLinePos_channel2.append(pos);
//     list_filterLineIdx_channel2.append(btStart);
// }

// void ZySonarImage::setMultiCorrect(bool value,bool small2)
// {
//     flag_inMultiCorrect = value;
//     flag_useSmall = small2;
// }

// int ZySonarImage::getRemoveModeStep() const
// {
//     return removeModeStep;
// }

// void ZySonarImage::setRemoveIndex(int indexFrom, int indexTo)
// {
//     removeFromIndex = indexFrom;
//     removeToIndex = indexTo;
// }

// void ZySonarImage::doRemoveModeStep_1(int numFrom)
// {
//     removeFromNum = QString::number(numFrom+1);
//     removeModeStep = 2;
// }

// void ZySonarImage::doRemoveModeStep_2(int numTo)
// {
//     removeToNum = QString::number(numTo+1);
//     removeModeStep = 3;
// }

// void ZySonarImage::getInRemoveMode()
// {
//     removeModeStep = 1;
// }

// void ZySonarImage::getOutRemoveMode()
// {
//     removeModeStep = -1;
// }

// int ZySonarImage::getSonarImageHeight() const
// {
//     return sonarImageHeight;
// }

// int ZySonarImage::getSonarImageWidth() const
// {
//     return sonarImageWidth;
// }

// void ZySonarImage::paintEvent(QPaintEvent *event)
// {
//     /*- 声呐图像的重绘事件，滚动图像就是在这里显示 -*/
//     //if(event->isAccepted())
//     {
//         QPainter painter(this);
//         QFont font;
//         QPen pen;

//         int windowWidth = this->width();
//         int windowHeight = this->height();
//         painter.drawPixmap(QPoint(-sonarImage->width()*(chartSpeedLevel-1),0),
//                            QPixmap::fromImage(sonarImage->scaled(sonarImage->width()*chartSpeedLevel,sonarImage->height(),Qt::IgnoreAspectRatio)));

//         //当有多屏时、并且多屏合并时，在一个窗口中一起绘出
//         int slashCount = currentSonarFreq.count("/");
//         //        qDebug() << "slashCount:" << slashCount;
//         if(slashCount == 1) {
//             painter.setOpacity(0.7);
//             painter.drawPixmap(QPoint(-sonarImage->width()*(chartSpeedLevel-1),0),
//                                QPixmap::fromImage(sonarImage_2->scaled(sonarImage->width()*chartSpeedLevel,sonarImage->height(),Qt::IgnoreAspectRatio)));
//             painter.setOpacity(1);
//         }
//         else if(slashCount == 2) {
//             painter.setOpacity(0.7);
//             painter.drawPixmap(QPoint(-sonarImage->width()*(chartSpeedLevel-1),0),
//                                QPixmap::fromImage(sonarImage_channel1->scaled(sonarImage->width()*chartSpeedLevel,sonarImage->height(),Qt::IgnoreAspectRatio)));
//             painter.drawPixmap(QPoint(-sonarImage->width()*(chartSpeedLevel-1),0),
//                                QPixmap::fromImage(sonarImage_channel2->scaled(sonarImage->width()*chartSpeedLevel,sonarImage->height(),Qt::IgnoreAspectRatio)));
//             painter.setOpacity(1);
//         }

//         if(!list_bottomLinePos.isEmpty()) {
//             QPen bottomLinePen;
//             bottomLinePen.setColor(Qt::red);
//             bottomLinePen.setStyle(Qt::SolidLine);
//             painter.setOpacity(1);
//             painter.setBrush(QBrush(QColor(255,255,255,150)));

//             bottomLinePen.setWidth(1);
//             painter.setPen(bottomLinePen);
//             for(int i = 1; i < list_bottomLinePos.count(); i++) {
//                 int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos.at(i)*chartSpeedLevel+chartSpeedLevel/2;
//                 int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos.at(i-1)*chartSpeedLevel+chartSpeedLevel/2;
//                 int idx_now = list_bottomLineIdx.at(i);
//                 int idx_last = list_bottomLineIdx.at(i-1);
//                 painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                 //qDebug()<<"pos_1="<<list_bottomLinePos.at(i)<<"idx_1="<<list_bottomLineIdx.at(i);
//             }
//         }

//         if(!list_filterLinePos.isEmpty()) {
//             QPen filterLinePen;
//             filterLinePen.setColor(Qt::green);
//             filterLinePen.setStyle(Qt::SolidLine);
//             painter.setOpacity(1);
//             painter.setBrush(QBrush(QColor(255,255,255,150)));
//             filterLinePen.setWidth(1);
//             painter.setPen(filterLinePen);
//             for(int i = 1;i < list_filterLinePos.count(); i++) {
//                 int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                 int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                 int idx_now = list_filterLineIdx.at(i);
//                 int idx_last = list_filterLineIdx.at(i-1);
//                 painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                 //qDebug()<<"pos_1="<<list_filterLinePos.at(i)<<"idx_1="<<list_filterLineIdx.at(i);
//             }

//         }

//         if(!list_bottomLinePos_2.isEmpty()) {
//             QPen bottomLinePen;
//             bottomLinePen.setColor(Qt::cyan);
//             bottomLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 bottomLinePen.setWidth(1);
//                 painter.setPen(bottomLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_bottomLinePos_2.count(); i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1)+list_bottomLinePos_2.at(i)*chartSpeedLevel+chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1)+list_bottomLinePos_2.at(i-1)*chartSpeedLevel+chartSpeedLevel/2;
//                     int idx_now = list_bottomLineIdx_2.at(i);
//                     int idx_last = list_bottomLineIdx_2.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_bottomLinePos_2.at(i)<<"idx_2="<<list_bottomLineIdx_2.at(i);
//                 }
//             }
//         }

//         if(!list_bottomLinePos_channel1.isEmpty()) {
//             QPen bottomLinePen;
//             bottomLinePen.setColor(Qt::cyan);
//             bottomLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 bottomLinePen.setWidth(1);
//                 painter.setPen(bottomLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_bottomLinePos_channel1.count(); i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos_channel1.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos_channel1.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                     int idx_now = list_bottomLineIdx_channel1.at(i);
//                     int idx_last = list_bottomLineIdx_channel1.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_bottomLinePos_2.at(i)<<"idx_2="<<list_bottomLineIdx_2.at(i);
//                 }
//             }
//         }

//         if(!list_bottomLinePos_channel2.isEmpty()) {
//             QPen bottomLinePen;
//             bottomLinePen.setColor(Qt::darkYellow);
//             bottomLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 bottomLinePen.setWidth(1);
//                 painter.setPen(bottomLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_bottomLinePos_channel2.count(); i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos_channel2.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_bottomLinePos_channel2.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                     int idx_now = list_bottomLineIdx_channel2.at(i);
//                     int idx_last = list_bottomLineIdx_channel2.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_bottomLinePos_2.at(i)<<"idx_2="<<list_bottomLineIdx_2.at(i);
//                 }
//             }
//         }

//         if(!list_filterLinePos_2.isEmpty()) {
//             QPen filterLinePen;
//             filterLinePen.setColor(Qt::green);
//             filterLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 filterLinePen.setWidth(1);
//                 painter.setPen(filterLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_filterLinePos_2.count();i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_2.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_2.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                     int idx_now = list_filterLineIdx_2.at(i);
//                     int idx_last = list_filterLineIdx_2.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_filterLinePos_2.at(i)<<"idx_2="<<list_filterLineIdx_2.at(i);
//                 }
//             }
//         }

//         if(!list_filterLinePos_channel1.isEmpty()) {
//             QPen filterLinePen;
//             filterLinePen.setColor(Qt::green);
//             filterLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 filterLinePen.setWidth(1);
//                 painter.setPen(filterLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_filterLinePos_channel1.count();i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_channel1.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_channel1.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                     int idx_now = list_filterLineIdx_channel1.at(i);
//                     int idx_last = list_filterLineIdx_channel1.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_filterLinePos_2.at(i)<<"idx_2="<<list_filterLineIdx_2.at(i);
//                 }
//             }
//         }

//         if(!list_filterLinePos_channel2.isEmpty()) {
//             QPen filterLinePen;
//             filterLinePen.setColor(Qt::green);
//             filterLinePen.setStyle(Qt::SolidLine);
//             if(currentViewerMode == "offline") {
//                 filterLinePen.setWidth(1);
//                 painter.setPen(filterLinePen);
//                 painter.setOpacity(1);
//                 painter.setBrush(QBrush(QColor(255,255,255,150)));

//                 for(int i = 1;i < list_filterLinePos_channel2.count();i++) {
//                     int pos_now = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_channel2.at(i)*chartSpeedLevel +chartSpeedLevel/2;
//                     int pos_last = -sonarImage->width()*(chartSpeedLevel-1) + list_filterLinePos_channel2.at(i-1)*chartSpeedLevel +chartSpeedLevel/2;
//                     int idx_now = list_filterLineIdx_channel2.at(i);
//                     int idx_last = list_filterLineIdx_channel2.at(i-1);
//                     painter.drawLine(pos_last, idx_last, pos_now, idx_now);
//                     //qDebug()<<"pos_2="<<list_filterLinePos_2.at(i)<<"idx_2="<<list_filterLineIdx_2.at(i);
//                 }
//             }
//         }

//         /*-批量深度校正-*/
//         if(flag_inMultiCorrect == true) {
//             int rr = 3;
//             int penWidth = 6;
//             if(flag_useSmall) {
//                 rr = 2;
//                 penWidth = 3;
//             }

//             QPen multiCorrectPen;
//             if(currentCorrectColor == "red") {
//                 multiCorrectPen.setColor(Qt::red);
//             } else if(currentCorrectColor == "cyan") {
//                 multiCorrectPen.setColor(Qt::cyan);
//             }
//             multiCorrectPen.setStyle(Qt::SolidLine);
//             multiCorrectPen.setWidth(penWidth);
//             painter.setPen(multiCorrectPen);
//             painter.setOpacity(0.8);
//             painter.setBrush(QBrush(QColor(255,0,0,100)));
//             painter.drawEllipse(mousePos,rr,rr);

//             if(!list_multiCorrect.isEmpty()) {
//                 foreach (QPoint point, list_multiCorrect) {
//                     painter.drawPoint(point);
//                 }
//             }
//         }

//         /*-打标-*/
//         if(!list_markingPos.isEmpty()) {
//             QPen markLinePen;
//             markLinePen.setColor(Qt::red);
//             markLinePen.setStyle(Qt::DashDotLine); /*-点状的虚线-*/
//             painter.setPen(markLinePen);
//             painter.setOpacity(1);

//             for(int i = 0;i < list_markingPos.count();i++) {
//                 int pos = -sonarImage->width()*(chartSpeedLevel-1)+list_markingPos.at(i)*chartSpeedLevel;
//                 painter.drawLine(pos, 0,pos,this->height());

//                 painter.save();
//                 painter.translate(pos,this->height());
//                 painter.rotate(-90);
//                 QString string = list_markingString.at(i);
//                 painter.drawText(5,-2,string);
//                 painter.restore();
//             }
//         }

//         /*-这里是标记线，数据显示的都是这个线上的数据包-*/
//         if(markLineIndex != -1) {
//             QPen markLinePen;
//             markLinePen.setColor(Qt::red);
//             markLinePen.setStyle(Qt::DashDotLine);/*-点状的虚线-*/
//             painter.setPen(markLinePen);
//             painter.setOpacity(1);
//             painter.drawLine(markLineIndex, 0,markLineIndex,this->height());

//             QPen markTextPen;
//             markTextPen.setColor(Qt::red);
//             markTextPen.setStyle(Qt::SolidLine);
//             painter.setPen(markTextPen);
//             painter.setBrush(Qt::black);
//             painter.setOpacity(0.5);
//             int textX = 0;
//             int textY = -120;
//             QFont markTextFont;
//             markTextFont.setPixelSize(10);
//             markTextFont.setBold(true);
//             markTextPen.setColor(Qt::white);
//             painter.setFont(markTextFont);
//             painter.setPen(markTextPen);
//             painter.setOpacity(1);
//             if(markLineIndex < this->width()/2) {
//                 painter.drawRoundedRect(markLineIndex+5, this->height()/2-145, 140, 160, 10, 10);
//                 textX = 15;
//             } else {
//                 painter.drawRoundedRect(markLineIndex-145, this->height()/2-145, 140, 160, 10, 10);
//                 textX = -135;
//             }
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY, markInfo_depth);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +20, markInfo_alt1);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +40, markInfo_alt2);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +60, markInfo_lon);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +80, markInfo_lat);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +105, markInfo_date);
//             painter.drawText(markLineIndex+textX, this->height()/2 +textY +120, markInfo_time);
//         }

//         if(removeModeStep == 1) {
//             int offset = 0;
//             if(toolTipPos.x() >= this->width()/2) {
//                 offset = -160;
//             }

//             painter.setOpacity(0.8);
//             painter.fillRect(toolTipPos.x()+10+offset, toolTipPos.y(), 150, 20, Qt::white);
//             painter.setOpacity(1);
//             painter.drawText(toolTipPos+QPoint(12+offset,15),tr("Select Start Frame"));

//             this->update();
//         }
//         else if(removeModeStep == 2) {
//             int offset = 0;
//             if(toolTipPos.x() >= this->width()/2) {
//                 offset = -160;
//             }
//             painter.setOpacity(0.8);
//             painter.fillRect(toolTipPos.x()+10+offset, toolTipPos.y(), 150, 20, Qt::white);
//             painter.setOpacity(1);
//             painter.drawText(toolTipPos+QPoint(12+offset,15),tr("Select End Frame"));

//             if(removeFromIndex < this->width()/2) {
//                 offset = 0;
//             } else {
//                 offset = -100;
//             }
//             QPen markLinePen;
//             markLinePen.setColor(Qt::yellow);
//             painter.setPen(markLinePen);
//             painter.setOpacity(0.8);
//             painter.fillRect(removeFromIndex+10+offset, this->height()/2-20, 90, 20, Qt::black);
//             painter.setOpacity(1);
//             painter.drawLine(removeFromIndex,0,removeFromIndex,this->height());
//             painter.drawText(removeFromIndex+12+offset, this->height()/2+15-20,tr("Frame ")+removeFromNum);

//             this->update();
//         }
//         else if(removeModeStep == 3) {
//             QPen markLinePen;
//             markLinePen.setColor(Qt::yellow);
//             painter.setPen(markLinePen);
//             int offset = 0;
//             if(removeFromIndex >= this->width()/2){
//                 offset = -100;
//             }
//             painter.setOpacity(0.8);
//             painter.fillRect(removeFromIndex+10+offset, this->height()/2-20, 90, 20, Qt::black);
//             painter.setOpacity(1);
//             painter.drawLine(removeFromIndex,0,removeFromIndex,this->height());
//             painter.drawText(removeFromIndex+12+offset, this->height()/2+15-20,tr("Frame ")+removeFromNum);

//             if(removeToIndex < this->width()/2) {
//                 offset = 0;
//             } else {
//                 offset = -100;
//             }
//             painter.setOpacity(0.8);
//             painter.fillRect(removeToIndex+10+offset, this->height()/2+20, 90, 20, Qt::black);
//             painter.setOpacity(1);
//             painter.drawLine(removeToIndex,0,removeToIndex,this->height());
//             painter.drawText(removeToIndex+12+offset, this->height()/2+15+20,tr("Frame ")+removeToNum);
//         }

//         if((flag_inAlarm == true) && (flag_notAgain == false)) {
//             font.setBold(false);
//             font.setPointSize(14);
//             painter.setFont(font);
//             pen.setColor(Qt::black);
//             pen.setWidth(3);
//             painter.setPen(pen);

//             int y = windowHeight/2-40;
//             painter.fillRect(0,y,windowWidth,40,QBrush(QColor(255, 201, 14, 200)));

//             int x_close = windowWidth -40;
//             painter.drawPixmap(x_close,y+4,QPixmap(":/sonar/close.png"));

//             int x_line_close = x_close -5;
//             painter.drawLine(x_line_close, y+2, x_line_close, y+38);
//             painter.drawText(0,y,x_line_close,40,Qt::AlignCenter,tr("Depth Abnormal"));

//             int width_depthAbnormal = painter.fontMetrics().horizontalAdvance(tr("Depth Abnormal"));
//             int x_warning = x_line_close/2 -width_depthAbnormal/2 -40;
//             painter.drawPixmap(x_warning,y+4,QPixmap(":/sonar/warning.png"));

//             int width_notAgain = painter.fontMetrics().horizontalAdvance(tr("Don't show again"));
//             int x_notAgain = windowWidth - width_notAgain -5;

//             x_chBox = x_notAgain -25;
//             y_chbox = y+40+10;

//             int x_line_notAgain = x_chBox -5;

//             painter.fillRect(x_line_notAgain,y+40,windowWidth-x_line_notAgain,40,QBrush(QColor(255, 201, 14, 200)));
//             painter.drawLine(x_line_notAgain, y+41, windowWidth, y+41);
//             painter.drawText(x_notAgain,y+40,width_notAgain,40,Qt::AlignCenter,tr("Don't show again"));
//             painter.drawRect(x_chBox,y_chbox,20,20);
//             if(flag_chBox_check) {
//                 painter.drawLine(x_chBox+4,y_chbox+10,x_chBox+9,y_chbox+15);
//                 painter.drawLine(x_chBox+9,y_chbox+15,x_chBox+17,y_chbox+7);
//             }
//         }

//         /*-下方拖动控件-*/
//         if(getCurrentSonarImage() == 0) {
//             QPolygon polygon;
//             polygon.append(QPoint(0,windowHeight-50));
//             polygon.append(QPoint(20,windowHeight-30));
//             polygon.append(QPoint(20,windowHeight-0));
//             polygon.append(QPoint(0,windowHeight-0));
//             painter.setPen(QColor(240,240,240));
//             painter.setBrush(QColor(240,240,240));
//             painter.drawPolygon(polygon,Qt::OddEvenFill);
//             painter.setPen(Qt::gray);
//             painter.drawLine(0,windowHeight-40,0,windowHeight-0);
//             painter.drawLine(5,windowHeight-35,5,windowHeight-0);
//             painter.drawLine(10,windowHeight-30,10,windowHeight-0);
//             painter.drawLine(15,windowHeight-25,15,windowHeight-0);
//         }

//         /*-声呐频率提示字符-*/
//         font.setBold(true);
//         font.setPointSize(10);
//         painter.setFont(font);

//         QFontMetrics fm(font);
//         QRect rec = fm.boundingRect(currentSonarFreq);

//         //字符串所占的像素宽度,高度
//         double dpi = (logicalDpiX()+logicalDpiY())/2/96.0f;
//         int textWidth = (float)rec.width()*dpi;
//         int textHeight = (float)rec.height()*dpi;

//         painter.setOpacity(0.3);
//         painter.setPen(Qt::black);
//         painter.setBrush(Qt::black);
//         if(getCurrentSonarImage() == 1) {
//             painter.drawRect(0,this->height()-textHeight,textWidth,textHeight);
//         } else  {
//             painter.drawRect(this->width()-textWidth,this->height()-textHeight,textWidth,textHeight);
//         }

//         pen.setColor(Qt::white);
//         painter.setPen(pen);
//         painter.setOpacity(0.8);
//         if(getCurrentSonarImage() == 1) {
//             painter.drawText(QRect(0,this->height()-textHeight,textWidth,textHeight),currentSonarFreq,QTextOption(Qt::AlignCenter));
//         } else {
//             painter.drawText(QRect(this->width()-textWidth,this->height()-textHeight,textWidth,textHeight),currentSonarFreq,QTextOption(Qt::AlignCenter));
//         }

//         //当前帧的基本信息显示
//         painter.setOpacity(0.3);
//         painter.setPen(Qt::black);
//         painter.setBrush(Qt::black);
//         if(getCurrentSonarImage() == 0)
//         {
//             painter.drawRect(0,0,this->width(),40);
//             if(flag_showExtraInfo == true) painter.drawRect(0,41,140,40);

//             font.setBold(true);
//             font.setPointSize(10);
//             painter.setFont(font);
//             pen.setColor(Qt::white);
//             painter.setPen(pen);
//             painter.setOpacity(0.8);

//             //nie:这里的append有崩溃，可能是传参内容有地址不明的空指针，暂时返回
//             //            return;
//             QString line_1;
//             line_1 = line_1.append(info_depth).append("     ").append(info_hdg).append("     ").append(info_spd).append("     ");
//             QString line_2;
//             line_2 = line_2.append(info_lon).append("     ").append(info_lat).append("     ").append(info_temp).append("     ");
//             painter.drawText(0,0,this->width(),20,Qt::AlignRight|Qt::AlignVCenter, line_1);
//             painter.drawText(0,20,this->width(),20,Qt::AlignRight|Qt::AlignVCenter, line_2);

//             if(flag_showExtraInfo == true) {
//                 painter.drawText(5, 20, info_sat);
//                 painter.drawText(5, 40, info_hdop);
//                 painter.drawText(5, 60, info_status);
//                 painter.drawText(5, 80, info_wridx);
//             }
//         }
//         else if(getCurrentSonarImage() == 1)
//         {
//             painter.drawRect(this->width()-boatWidth_/2,0,boatWidth_/2,this->height());
//             painter.setOpacity(0.7);

//             QRect sourceRect(0, 0, boatWidth_/2, boatHeight_); //船的左半部分
//             QRect targetRect(this->width() - boatWidth_/2,0,boatWidth_/2,boatHeight_);
//             painter.drawPixmap(targetRect, slideScanBoat_, sourceRect);
//         }
//         else if(getCurrentSonarImage() == 2)
//         {
//             painter.drawRect(0,0,boatWidth_/2,this->height());
//             painter.setOpacity(0.8);

//             QRect sourceRect(boatWidth_/2, 0, boatWidth_/2, boatHeight_); //船的右半部分
//             QRect targetRect(0,0,boatWidth_/2,boatHeight_);
//             painter.drawPixmap(targetRect, slideScanBoat_, sourceRect);
//         }

//     }
// }

// void ZySonarImage::mouseMoveEvent(QMouseEvent *event)
// {
//     int height = this->height();
//     int width = this->width();
//     if((event->pos().x() < 20) && (event->pos().y() > height-50+event->pos().x())) {
//         this->setCursor(QCursor(Qt::SizeHorCursor));
//     } else {
//         if(this->cursor() == QCursor(Qt::SizeHorCursor)) this->setCursor(QCursor(Qt::ArrowCursor));
//     }

//     if(flag_inAlarm == true) {
//         if((event->pos().x()>width-40)&&(event->pos().y()>height/2-40)&&(event->pos().y()<height/2)) {
//             this->setCursor(QCursor(Qt::PointingHandCursor));
//         }

//         if((event->pos().x()>x_chBox)&&(event->pos().x()<x_chBox+20)&&(event->pos().y()>y_chbox)&&(event->pos().y()<y_chbox+20)) {
//             this->setCursor(QCursor(Qt::PointingHandCursor));
//         }
//     }

//     if(flag_inNaviLine == true) {
//         int temp = event->globalPos().x() - nowX;
//         //if(qAbs(temp) > 20)
//         {
//             emit signal_NaviLineMove(temp,false);
//         }
//         nowX = event->globalPos().x();
//     } else {
//         if(flag_inMultiCorrect == true) {
//             if(event->buttons() & Qt::LeftButton) {
//                 if(list_multiCorrect.isEmpty()) {
//                     list_multiCorrect.append(QPoint(event->pos()));
//                 } else if(list_multiCorrect.count() == 1) {
//                     QPoint diff_pos = event->pos() -list_multiCorrect.first();
//                     QPoint diff_single = diff_pos /qAbs(diff_pos.x());
//                     for(int i=0;i<qAbs(diff_pos.x());i++) {
//                         list_multiCorrect.append(list_multiCorrect.last()+diff_single);
//                     }
//                 }
//                 else {
//                     int diff_x = list_multiCorrect.last().x() -list_multiCorrect.first().x();
//                     if(diff_x > 0) {
//                         if(event->pos().x() > list_multiCorrect.last().x()) {
//                             QPoint diff_pos = event->pos() -list_multiCorrect.last();
//                             QPoint diff_single = diff_pos /qAbs(diff_pos.x());
//                             for(int i=0;i<qAbs(diff_pos.x());i++)
//                             {
//                                 list_multiCorrect.append(list_multiCorrect.last()+diff_single);
//                             }
//                         }
//                     }
//                     else if(diff_x < 0) {
//                         if(event->pos().x() < list_multiCorrect.last().x())
//                         {
//                             QPoint diff_pos = event->pos() -list_multiCorrect.last();
//                             QPoint diff_single = diff_pos /qAbs(diff_pos.x());
//                             for(int i=0;i<qAbs(diff_pos.x());i++)
//                             {
//                                 list_multiCorrect.append(list_multiCorrect.last()+diff_single);
//                             }
//                         }
//                     }
//                 }
//                 update();
//             }
//             else if(event->buttons() & Qt::RightButton) {
//                 int tmp;
//                 nowMousePos = event->pos().x();
//                 tmp = lastMousePos - nowMousePos;
//                 emit signal_SonarImageDrag(tmp*2);
//                 lastMousePos = nowMousePos;
//                 setCursor(QCursor(Qt::ClosedHandCursor));
//             }
//         }
//         else if(event->buttons() & Qt::LeftButton) {
//             int tmp;
//             nowMousePos = event->pos().x();
//             tmp = lastMousePos - nowMousePos;
//             emit signal_SonarImageDrag(tmp*2);
//             lastMousePos = nowMousePos;
//             setCursor(QCursor(Qt::ClosedHandCursor));
//         }

//         if((removeModeStep == 1) || (removeModeStep == 2)) {
//             toolTipPos = event->pos();
//         } else {
//             toolTipPos = QPoint(-1, -1);
//         }

//         if(flag_inMultiCorrect == true) {
//             mousePos = event->pos();
//             update();
//         }

//         emit signal_MouseMove(event->pos().x(),event->pos().y());//发送拖动信号
//     }
// }

// void ZySonarImage::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton) {
//         emit signal_MouseDoubleClickedLeft(event->pos().x());
//     }
// }

// void ZySonarImage::mouseReleaseEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::RightButton) {
//         emit signal_MouseClickedRight();
//         setCursor(QCursor(Qt::ArrowCursor));
//     }

//     if(event->button() == Qt::LeftButton) {
//         if(flag_inMultiCorrect == true) {
//             //setCursor(QCursor(Qt::CrossCursor));
//             emit signal_multiCorrect(list_multiCorrect);
//             list_multiCorrect.clear();
//             update();
//         } else if(flag_inNaviLine == true) {
//             setCursor(QCursor(Qt::ArrowCursor));

//             int temp = event->globalPos().x() - nowX;
//             emit signal_NaviLineMove(temp,true);
//             nowX = event->globalPos().x();
//         } else {
//             setCursor(QCursor(Qt::ArrowCursor));
//         }

//         flag_inNaviLine = false;
//     }
// }

// void ZySonarImage::mousePressEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton) {
//         int height = this->height();
//         int width = this->width();
//         if((event->pos().x()<20) && (event->pos().y()>height-50+event->pos().x())) {
//             flag_inNaviLine = true;
//             nowX = event->globalPos().x();
//         }

//         if((event->pos().x()>width-40) && (event->pos().y()>height/2-40) && (event->pos().y()<height/2)) {
//             this->hideAlarm();
//             this->setCursor(QCursor(Qt::ArrowCursor));
//             this->update();
//         }

//         if((event->pos().x()>x_chBox) && (event->pos().x()<x_chBox+20) && (event->pos().y()>y_chbox) && (event->pos().y()<y_chbox+20)) {
//             flag_chBox_check = !flag_chBox_check;
//             emit signal_chBox_click();
//             this->update();
//         }
//     }

//     if(event->button() == Qt::RightButton) {
//         if(removeModeStep == 2) {
//             removeModeStep = 1;
//         } else if(removeModeStep == 3) {
//             removeModeStep = 2;
//         }
//     }

//     lastMousePos = event->pos().x();
// }

// void ZySonarImage::leaveEvent(QMouseEvent *event)
// {
//     setCursor(QCursor(Qt::ArrowCursor));
// }

// void ZySonarImage::wheelEvent(QWheelEvent *event)
// {
//     if(event->delta()>0)
//     {
//         emit signal_SonarImageWheelUp();
//     }
//     else
//     {
//         emit signal_SonarImageWheelDown();
//     }
// }

// void ZySonarImage::resizeEvent(QResizeEvent *size)
// {
//     emit signal_SonarImageResized();
// }

// /*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// #define SONAR_WAVE_WIDTH 52 /*! @brief 声呐实时波形宽度*/

// ZySonarWave::ZySonarWave(QWidget *parent, int waveHeight)
// {
//     setMouseTracking(true);
//     bottomLineIndex = 0;
//     sonarWaveHeight = waveHeight;

//     initWaveBuffer();

// }
// ZySonarWave::~ZySonarWave(){}

// void ZySonarWave::initWaveBuffer()
// {
//     sonarWaveBuffer = new int[SONAR_WAVE_WIDTH*sonarWaveHeight];
//     sonarWave = new QImage((uchar *)sonarWaveBuffer, SONAR_WAVE_WIDTH, sonarWaveHeight, QImage::Format_RGB32);
//     sonarWave->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     this->setMinimumWidth(SONAR_WAVE_WIDTH);
// }

// void ZySonarWave::reFillBackgroundColor()
// {
//     sonarWave->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     bottomLineIndex = 0;
//     this->update();
// }

// void ZySonarWave::drawWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     if(scale <= 0)  {return;}

//     if(scale < 1) {
//         scale = 1/scale;
//         compressWavePixel(sonarInfo, scale, colorNum);
//     }
//     else {
//         stretchWavePixel(sonarInfo, scale, colorNum);
//     }

//     currentColor = colorNum;
// }
// void ZySonarWave::stretchWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     if(scale <= 0) {
//         return;
//     }

//     colorData.clear();

//     if(colorNum == 1) {
//         /*-水表-*/
//         for(int j = 0; (j < sonarInfo.sfEnd) && (j < sonarWaveHeight); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j < sonarInfo.btStart) && (j < sonarWaveHeight)); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_fish[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_fish[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveHeight; j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//     }
//     else if(colorNum == 2) {
//         /*-水表-*/
//         for(int j = 0; (j < sonarInfo.sfEnd) && (j < sonarWaveHeight); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j < sonarInfo.btStart) && (j < sonarWaveHeight)); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveHeight; j++) {
//             if(sonarInfo.rawData[j] < 5) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//     }


//     /*-左侧分割线-*/
//     for(int i = 0;i < sonarWaveHeight; i++) {
//         sonarWaveBuffer[i*SONAR_WAVE_WIDTH] = 0xf0f0f0;
//     }

//     /*-底色，防止拉伸在范围外-*/
//     for(int i = 0; i < sonarWaveHeight; i++) {
//         for(int j = 1;j < SONAR_WAVE_WIDTH; j++) {
//             sonarWaveBuffer[i*SONAR_WAVE_WIDTH+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }

//     for(int i = 0; ((i < sonarWaveHeight)&&((sonarInfo.startIdx+(int)(i/scale))<colorData.count())); i++) {
//         for(int j=(SONAR_WAVE_WIDTH/2-(sonarInfo.rawData[sonarInfo.startIdx+(int)(i/scale)]/ratio));j<(SONAR_WAVE_WIDTH/2+(sonarInfo.rawData[sonarInfo.startIdx+(int)(i/scale)]/ratio));j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] = colorData[sonarInfo.startIdx+(int)(i/scale)];
//         }
//     }
// }


// void ZySonarWave::compressWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     colorData.clear();
//     if(colorNum == 1) {
//         /*-水表-*/
//         for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarWaveHeight); j++)
//         {
//             if(sonarInfo.rawData[j] == 0)
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarWaveHeight)); j++)
//         {
//             if(sonarInfo.rawData[j] == 0)
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveHeight; j++)
//         {
//             if(sonarInfo.rawData[j] == 0)
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//     }
//     else if(colorNum == 2)
//     {
//         /*-水表-*/
//         for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarWaveHeight); j++)
//         {
//             if(sonarInfo.rawData[j] == 0)
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_surface_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarWaveHeight)); j++)
//         {
//             if(sonarInfo.rawData[j] == 0)
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_fish_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveHeight; j++)
//         {
//             if((sonarInfo.rawData[j] <5))
//             {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             }
//             else
//             {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[254]);
//                 }
//                 else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[0]);
//                 }
//                 else
//                 {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//     }


//     /*-左侧分割线-*/
//     for(int i=0;i<sonarWaveHeight; i++)
//     {
//         sonarWaveBuffer[i*SONAR_WAVE_WIDTH] =  0xf0f0f0;
//     }

//     /*-底色，防止拉伸在范围外-*/
//     for(int i=0; i<sonarWaveHeight; i++)
//     {
//         for(int j=1;j<SONAR_WAVE_WIDTH;j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }

//     for(int i=0; ((sonarInfo.startIdx+(int)(i*scale))<colorData.count())&&(i<sonarWaveHeight); i++)
//     {
//         for(int j=(SONAR_WAVE_WIDTH/2-(sonarInfo.rawData[sonarInfo.startIdx+(int)(i*scale)]/ratio));j<(SONAR_WAVE_WIDTH/2+(sonarInfo.rawData[sonarInfo.startIdx+(int)(i*scale)]/ratio));j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] = colorData[sonarInfo.startIdx+(int)(i*scale)];
//         }

//     }

// }

// void ZySonarWave::drawWavePixelOnline(StructSonarInfo sonarInfo, float scale)
// {
//     if(scale <= 0)  {return;}

//     if(scale < 1) {
//         scale = 1/scale;
//         compressWavePixelOnline(sonarInfo, scale);
//     } else {
//         stretchWavePixelOnline(sonarInfo, scale);
//     }
// }

// void ZySonarWave::stretchWavePixelOnline(StructSonarInfo sonarInfo, float scale)
// {
//     sonarInfo.btStart = sonarInfo.btStart*scale;

//     quint8 afterRawDat[sonarWaveHeight];//处理非自动深度范围的情况

//     for(int i=0;i<sonarWaveHeight; i++) {
//         afterRawDat[i] = sonarInfo.rawData[(int)(i/scale)];
//     }

//     for(int i=0;i<sonarWaveHeight; i++) {
//         sonarWaveBuffer[i*SONAR_WAVE_WIDTH] = 0xf0f0f0;
//     }

//     for(int i=0; (i<sonarInfo.btStart) && (i<sonarWaveHeight); i++) {

//         for(int j=1;j<(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j++) {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j<(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j++) {
//             if((sonarInfo.rawData[i]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH+j] = ZyColorScheme::colorScheme_fish[254];
//             } else if((sonarInfo.rawData[i]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH+j] = ZyColorScheme::colorScheme_fish[0];
//             } else {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH+j] = ZyColorScheme::colorScheme_fish[sonarInfo.rawData[(int)(i/scale)]+ZyColorScheme::colorLine*COLOR_LINE];
//             }
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j<SONAR_WAVE_WIDTH;j++) {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }

//     for(int i=sonarInfo.btStart; (i)<sonarWaveHeight; i++) {
//         for(int j=1;j<(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j++) {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j<(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j++) {
//             if((sonarInfo.rawData[i]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[254];
//             } else if((sonarInfo.rawData[i]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[0];
//             } else {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[sonarInfo.rawData[(int)(i/scale)]+ZyColorScheme::colorLine*COLOR_LINE];
//             }
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j<SONAR_WAVE_WIDTH;j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }

// }
// void ZySonarWave::compressWavePixelOnline(StructSonarInfo sonarInfo, float scale)
// {
//     sonarInfo.btStart = sonarInfo.btStart/scale;

//     quint8 afterRawDat[sonarWaveHeight];//处理非自动深度范围的情况
//     afterRawDat[sonarWaveHeight] = {0};

//     for(int i = 0;i < sonarWaveHeight/scale;i++) {
//         afterRawDat[i] = sonarInfo.rawData[(int)(i*scale)];
//     }
//     for(int i = sonarWaveHeight/scale;i < sonarWaveHeight;i++) {
//         afterRawDat[i] = 0;
//     }

//     for(int i = 0;i < sonarWaveHeight; i++) {
//         sonarWaveBuffer[i*SONAR_WAVE_WIDTH] = 0xf0f0f0;
//     }

//     for(int i = 0; (i<sonarInfo.btStart)&&(i<sonarWaveHeight); i++) {
//         for(int j=1;j<(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j++) {
//             sonarWaveBuffer[i*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         for(int j = (SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j<(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j++)
//         {
//             if((afterRawDat[i]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                 sonarWaveBuffer[i*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_fish[254];
//             }
//             else if((afterRawDat[i]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                 sonarWaveBuffer[i*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_fish[0];
//             }
//             else {
//                 sonarWaveBuffer[i*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_fish[afterRawDat[(int)(i*scale)]+ZyColorScheme::colorLine*COLOR_LINE];
//             }
//         }
//         for(int j = (SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j<SONAR_WAVE_WIDTH;j++) {
//             sonarWaveBuffer[i*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }
//     /*-水底波形-*/
//     for(int i=sonarInfo.btStart; (i)<sonarWaveHeight; i++)
//     {
//         for(int j=1;j<(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2-(afterRawDat[i]/ratio));j<(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j++)
//         {
//             if((afterRawDat[i]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
//             {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[254];
//             }
//             else if((afterRawDat[i]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
//             {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[0];
//             }
//             else
//             {
//                 sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::colorScheme_bottom[afterRawDat[(int)(i*scale)]+ZyColorScheme::colorLine*COLOR_LINE];
//             }
//         }
//         for(int j=(SONAR_WAVE_WIDTH/2+(afterRawDat[i]/ratio));j<SONAR_WAVE_WIDTH;j++)
//         {
//             sonarWaveBuffer[(i)*SONAR_WAVE_WIDTH +j] =  ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }
// }

// void ZySonarWave::paintEvent(QPaintEvent *event)
// {
//     /*-声呐图像的重绘事件，滚动图像就是在这里显示-*/
//     if(event->isAccepted()) {
//         /*-将图像拉伸至窗口部件大小，忽略宽高比-*/
//         QPainter painter(this);
//         QImage tmpImage = sonarWave->scaled(this->width(),sonarWave->height(),Qt::IgnoreAspectRatio);
//         painter.drawPixmap(QPoint(0,0),QPixmap::fromImage(tmpImage));

//         /*-bottomLine-*/
//         if(showBottomLine == true) {
//             QPen pen;
//             if(currentColor == 1) pen.setColor(Qt::red);
//             else pen.setColor(Qt::cyan);
//             painter.setPen(pen);
//             painter.drawLine(1, bottomLineIndex, this->width(), bottomLineIndex);
//             painter.drawLine(1, bottomLineIndex+1, this->width(), bottomLineIndex+1);

//             if(bottomLineIndex_hFreq != -1) {
//                 pen.setColor(Qt::cyan);
//                 painter.setPen(pen);
//                 painter.drawLine(1, bottomLineIndex_hFreq, this->width(), bottomLineIndex_hFreq);
//                 painter.drawLine(1, bottomLineIndex_hFreq+1, this->width(), bottomLineIndex_hFreq+1);
//             }
//             if(bottomLineIndex_channel1 != -1) {
//                 pen.setColor(Qt::cyan);
//                 painter.setPen(pen);
//                 painter.drawLine(1, bottomLineIndex_hFreq, this->width(), bottomLineIndex_hFreq);
//                 painter.drawLine(1, bottomLineIndex_hFreq+1, this->width(), bottomLineIndex_hFreq+1);
//             }
//             if(bottomLineIndex_channel2 != -1) {
//                 pen.setColor(Qt::yellow);
//                 painter.setPen(pen);
//                 painter.drawLine(1, bottomLineIndex_hFreq, this->width(), bottomLineIndex_hFreq);
//                 painter.drawLine(1, bottomLineIndex_hFreq+1, this->width(), bottomLineIndex_hFreq+1);
//             }
//         }

//     }
// }

// int ZySonarWave::getBottomLineIndex() const
// {
//     return bottomLineIndex;
// }

// void ZySonarWave::setBottomLineIndex(int value)
// {
//     bottomLineIndex = value;
// }
// void ZySonarWave::setBottomLineIndex_hFreq(int value)
// {
//     bottomLineIndex_hFreq = value;
// }
// void ZySonarWave::setBottomLineIndex_channel1(int value)
// {
//     bottomLineIndex_channel1 = value;
// }
// void ZySonarWave::setBottomLineIndex_channel2(int value)
// {
//     bottomLineIndex_channel2 = value;
// }

// void ZySonarWave::mouseMoveEvent(QMouseEvent *event)
// {
//     emit signal_MouseMoveY(event->pos().y());
// }

// void ZySonarWave::mousePressEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton)
//     {
//         emit signal_MousePressed();
//     }
// }

// void ZySonarWave::mouseReleaseEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton)
//     {
//         emit signal_MouseReleased();
//     }
// }

// void ZySonarWave::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::RightButton)
//     {
//         emit signal_MouseDoubleClickedRight();
//     }
// }

// void ZySonarWave::leaveEvent(QEvent *event)
// {
//     emit signal_MouseLeaved();
// }

// /*-----------------------------------------------------------------------------------------*/
// #define SONAR_WAVE_HEIGHT 52   /* 侧扫时，实时声纳波形宽度 */
// ZySonarWave_SlideScan::ZySonarWave_SlideScan(QWidget *parent, int waveWidth)
// {
//     setMouseTracking(true);
//     bottomLineIndex = 0;
//     sonarWaveWidth = waveWidth;

//     initWaveBuffer();
// }
// ZySonarWave_SlideScan::~ZySonarWave_SlideScan(){}

// void ZySonarWave_SlideScan::initWaveBuffer()
// {
//     sonarWaveBuffer = new int[SONAR_WAVE_HEIGHT*sonarWaveWidth];
//     sonarWave = new QImage((uchar *)sonarWaveBuffer, sonarWaveWidth, SONAR_WAVE_HEIGHT, QImage::Format_RGB32);
//     sonarWave->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     this->setMinimumHeight(SONAR_WAVE_HEIGHT);
// }

// void ZySonarWave_SlideScan::reFillBackgroundColor()
// {
//     sonarWave->fill(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     bottomLineIndex = 0;
//     this->update();
// }

// void ZySonarWave_SlideScan::drawWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     if(scale <= 0)  { return; }

//     /*
//      * colorNum = 1,表示侧扫的左侧波形
//      * colorNum = 2,表示侧扫的左侧波形
//     */

//     colorData.clear();

//     if(colorNum == 1)
//     {
//         /*-水表-*/
//         for(int j = 0; (j < sonarInfo.sfEnd) && (j < sonarWaveWidth); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel1[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel1[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j < sonarInfo.btStart) && (j < sonarWaveWidth)); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel1[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel1[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveWidth; j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel1[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel1[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel1[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }
//     }

//     else if(colorNum == 2)
//     {
//         /*-水表-*/
//         for(int j = 0; (j < sonarInfo.sfEnd) && (j < sonarWaveWidth); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_surface_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水中-*/
//         for(int j = sonarInfo.sfEnd; ((j < sonarInfo.btStart) && (j < sonarWaveWidth)); j++) {
//             if(sonarInfo.rawData[j] == 0) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_fish_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//         /*-水底-*/
//         for(int j = sonarInfo.btStart; j < sonarWaveWidth; j++) {
//             if(sonarInfo.rawData[j] < 5) {
//                 colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//             } else {
//                 if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel2[254]);
//                 } else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel2[0]);
//                 } else {
//                     colorData.append(ZyColorScheme::colorScheme_bottom_channel2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
//                 }
//             }
//         }

//     }


//     /*-下侧分割线-*/
//     for(int i = 1;i <= sonarWaveWidth; i++) {
//         sonarWaveBuffer[i*SONAR_WAVE_HEIGHT-1] = 0xf0f0f0;
//     }

//     /*-底色，防止拉伸在范围外-*/
//     for(int i = 0; i < sonarWaveWidth; i++) {
//         for(int j = 1;j < SONAR_WAVE_HEIGHT; j++) {
//             sonarWaveBuffer[i*SONAR_WAVE_HEIGHT+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
//         }
//     }


//     if(scale < 1) {
//         scale = 1/scale;
//         compressWavePixel(sonarInfo,scale,colorNum);
//     } else {
//         stretchWavePixel(sonarInfo,scale,colorNum);
//     }

//     //    currentColor = colorNum;
//     whichChannel = colorNum;
// }

// void ZySonarWave_SlideScan::compressWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     for(int i = 0; ((i<sonarWaveWidth) && ((sonarInfo.startIdx+(int)(i*scale))<colorData.count())); i++)
//     {
//         for(int j=(SONAR_WAVE_HEIGHT/2-(sonarInfo.rawData[sonarInfo.startIdx+(int)(i*scale)]/ratio));j<(SONAR_WAVE_HEIGHT/2+(sonarInfo.rawData[sonarInfo.startIdx+(int)(i*scale)]/ratio));j++)
//         {
//             if(colorNum == 1){
//                 sonarWaveBuffer[sonarWaveWidth-1-i+j*sonarWaveWidth] = colorData[sonarInfo.startIdx+(int)(i*scale)];
//             } else if(colorNum == 2) {
//                 sonarWaveBuffer[i+j*sonarWaveWidth] = colorData[sonarInfo.startIdx+(int)(i*scale)];
//             }
//         }
//     }
// }

// void ZySonarWave_SlideScan::stretchWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum)
// {
//     for(int i = 0; ((i < sonarWaveWidth)&&((sonarInfo.startIdx+(int)(i/scale))<colorData.count())); i++)
//     {
//         for(int j=(SONAR_WAVE_HEIGHT/2-(sonarInfo.rawData[sonarInfo.startIdx+(int)(i/scale)]/ratio));j<(SONAR_WAVE_HEIGHT/2+(sonarInfo.rawData[sonarInfo.startIdx+(int)(i/scale)]/ratio));j++)
//         {
//             if(colorNum == 1) {
//                 sonarWaveBuffer[sonarWaveWidth-1-i+j*sonarWaveWidth] = colorData[sonarInfo.startIdx+(int)(i/scale)];
//             } else if(colorNum == 2) {
//                 sonarWaveBuffer[i+j*sonarWaveWidth] = colorData[sonarInfo.startIdx+(int)(i/scale)];
//             }
//         }
//     }
// }


// void ZySonarWave_SlideScan::paintEvent(QPaintEvent *event)
// {
//     if(event->isAccepted()) {
//         QPainter painter(this);
//         QImage tmpImage = sonarWave->scaled(this->width(),sonarWave->height(),Qt::IgnoreAspectRatio);
//         painter.drawPixmap(QPoint(0,0),QPixmap::fromImage(tmpImage));
//         painter.setOpacity(0.7);
//         painter.setPen(Qt::white);
//         painter.drawLine(0,this->height()-1,this->width(),this->height());
//         if(whichChannel == 1) {
//             painter.drawLine(this->width()-1,0,this->width()-1,this->height()-1);
//         }
//         else if(whichChannel == 2) {
//             painter.drawLine(0,0,0,this->height()-1);
//         }
//         painter.setOpacity(1);

//         /*-bottomLine-*/
//         if(showBottomLine == true) {
//             QPen pen;
//             pen.setColor(Qt::red);
//             painter.setPen(pen);

//             if(whichChannel == 1) {
//                 painter.drawLine(this->width()-1-bottomLineIndex,0,this->width()-1-bottomLineIndex,sonarWave->height());
//             }
//             else if(whichChannel == 2) {
//                 painter.drawLine(bottomLineIndex,0,bottomLineIndex,sonarWave->height());

//             }

//         }

//     }
// }

// int ZySonarWave_SlideScan::getBottomLineIndex() const
// {
//     return bottomLineIndex;
// }

// void ZySonarWave_SlideScan::setBottomLineIndex(int value)
// {
//     bottomLineIndex = value;
// }

// void ZySonarWave_SlideScan::mouseMoveEvent(QMouseEvent *event)
// {
//     emit signal_MouseMoveY(event->pos().y());
// }

// void ZySonarWave_SlideScan::mousePressEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton)
//     {
//         emit signal_MousePressed();
//     }
// }

// void ZySonarWave_SlideScan::mouseReleaseEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton)
//     {
//         emit signal_MouseReleased();
//     }
// }

// void ZySonarWave_SlideScan::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::RightButton)
//     {
//         emit signal_MouseDoubleClickedRight();
//     }
// }

// void ZySonarWave_SlideScan::leaveEvent(QEvent *event)
// {
//     emit signal_MouseLeaved();
// }



// /*-----------------------------声呐刻度尺类----------------------------------------------*/
// ZySonarAxis::ZySonarAxis(QWidget *parent)
// {
//     //this->setFixedWidth(36);

//     loRng = 15;//刻度尺的初始值最大是15
// }
// ZySonarAxis::~ZySonarAxis(){}
// void ZySonarAxis::paintEvent(QPaintEvent *event)
// {
//     int startX = 0;
//     QPainter painter(this);

//     /*-根据lorng位数确定宽度-*/
//     QFontMetrics fm = painter.fontMetrics();
//     int field = fm.boundingRect(QString::number(loRng*10/10,'f',1)).width()+20;
//     this->setMinimumWidth(field);

//     painter.fillRect(0,0,this->width(),this->height(),ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     painter.setOpacity(0.3);
//     painter.setBrush(Qt::black);
//     painter.drawRect(0,0,this->width(),this->height());
//     painter.setOpacity(0.7);

//     QPen aixsPen;
//     aixsPen.setWidth(1);

//     /*-竖线-*/
//     aixsPen.setColor(Qt::white);
//     painter.setPen(aixsPen);
//     painter.drawLine(startX, 0, startX, this->height()-1);
//     painter.drawLine(startX+1, 0, startX+1, this->height()-1);

//     /*-横线-*/
//     painter.drawLine(startX+9,0,startX+3,0);
//     painter.drawLine(startX+7, this->height()*1/10-1, startX+3,this->height()*1/10-1);
//     painter.drawLine(startX+9, this->height()*2/10-1, startX+3,this->height()*2/10-1);
//     painter.drawLine(startX+7, this->height()*3/10-1, startX+3,this->height()*3/10-1);
//     painter.drawLine(startX+9, this->height()*4/10-1, startX+3,this->height()*4/10-1);
//     painter.drawLine(startX+7, this->height()*5/10-1, startX+3,this->height()*5/10-1);
//     painter.drawLine(startX+9, this->height()*6/10-1, startX+3,this->height()*6/10-1);
//     painter.drawLine(startX+7, this->height()*7/10-1, startX+3,this->height()*7/10-1);
//     painter.drawLine(startX+9, this->height()*8/10-1, startX+3,this->height()*8/10-1);
//     painter.drawLine(startX+7, this->height()*9/10-1, startX+3,this->height()*9/10-1);
//     painter.drawLine(startX+9, this->height()-2, startX+3,this->height()-2);

//     painter.drawLine(startX+5, this->height()*1/20, startX+3,this->height()*1/20);
//     painter.drawLine(startX+5, this->height()*3/20, startX+3,this->height()*3/20);
//     painter.drawLine(startX+5, this->height()*5/20, startX+3,this->height()*5/20);
//     painter.drawLine(startX+5, this->height()*7/20, startX+3,this->height()*7/20);
//     painter.drawLine(startX+5, this->height()*9/20, startX+3,this->height()*9/20);
//     painter.drawLine(startX+5, this->height()*11/20, startX+3,this->height()*11/20);
//     painter.drawLine(startX+5, this->height()*13/20, startX+3,this->height()*13/20);
//     painter.drawLine(startX+5, this->height()*15/20, startX+3,this->height()*15/20);
//     painter.drawLine(startX+5, this->height()*17/20, startX+3,this->height()*17/20);
//     painter.drawLine(startX+5, this->height()*19/20, startX+3,this->height()*19/20);


//     /*-深度范围的字体样式-*/
//     QFont axisFont;
//     axisFont.setBold(true);
//     axisFont.setPointSize(9);
//     painter.setFont(axisFont);
//     aixsPen.setColor(Qt::white);
//     painter.setPen(aixsPen);
//     painter.setOpacity(0.8);

//     /*-首先画出深度范围最小值0-*/
//     if(loRng < 1) {
//         painter.drawText(startX+14,fm.ascent(),"0");
//         painter.drawText(startX+14,this->height()*10/10-1,QString::number(loRng*10/10,'f',1));
//     }
//     else {
//         float diff = (loRng -upRng) /10;
//         painter.drawText(startX+14,fm.ascent(),QString::number(upRng,'f',1));
//         painter.drawText(startX+14,this->height()*2/10+fm.ascent()/2,QString::number(upRng+diff*2,'f',1));
//         painter.drawText(startX+14,this->height()*4/10+fm.ascent()/2,QString::number(upRng+diff*4,'f',1));
//         painter.drawText(startX+14,this->height()*6/10+fm.ascent()/2,QString::number(upRng+diff*6,'f',1));
//         painter.drawText(startX+14,this->height()*8/10+fm.ascent()/2,QString::number(upRng+diff*8,'f',1));
//         painter.drawText(startX+14,this->height()*10/10-1,QString::number(loRng,'f',1));
//     }

// }

// void ZySonarAxis::setLoRng(float value)
// {
//     loRng = value;
// }
// void ZySonarAxis::setUpRng(float value)
// {
//     upRng = value;
// }


// void ZySonarAxis::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::RightButton)
//     {
//         emit signal_MouseDoubleClickedRight();
//     }
// }


// /*------------------------------------------------------------------------------------------*/
// ZySonarAxis_slideScan::ZySonarAxis_slideScan(QWidget *parent)
// {
//     //this->setFixedWidth(36);

//     loRng = 15;//刻度尺的初始值最大是15
// }
// ZySonarAxis_slideScan::~ZySonarAxis_slideScan(){}
// void ZySonarAxis_slideScan::paintEvent(QPaintEvent *event)
// {
//     int startY = 0;
//     QPainter painter(this);

//     /*-根据lorng位数确定宽度-*/
//     QFontMetrics fm = painter.fontMetrics();
//     int field = fm.boundingRect(QString::number(loRng*10/10,'f',1)).height()+20;
//     this->setMinimumHeight(field);

//     painter.fillRect(0,0,this->width(),this->height(),ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
//     painter.setOpacity(0.3);
//     painter.setBrush(Qt::black);
//     painter.drawRect(0,0,this->width(),this->height());
//     painter.setOpacity(0.7);

//     QPen aixsPen;
//     aixsPen.setWidth(1);

//     /*-横线-*/
//     aixsPen.setColor(Qt::white);
//     painter.setPen(aixsPen);
//     painter.drawLine(0,startY, this->width()-1, startY);
//     painter.drawLine(0,startY+1,this->width()-1, startY+1);

//     //长竖线
//     painter.drawLine(0,startY+9,0,startY+3);
//     painter.drawLine(this->width()*1/10-1,startY+7,this->width()*1/10-1,startY+3);
//     painter.drawLine(this->width()*2/10-1,startY+9,this->width()*2/10-1,startY+3);
//     painter.drawLine(this->width()*3/10-1,startY+7,this->width()*3/10-1,startY+3);
//     painter.drawLine(this->width()*4/10-1,startY+9,this->width()*4/10-1,startY+3);
//     painter.drawLine(this->width()*5/10-1,startY+7,this->width()*5/10-1,startY+3);
//     painter.drawLine(this->width()*6/10-1,startY+9,this->width()*6/10-1,startY+3);
//     painter.drawLine(this->width()*7/10-1,startY+7,this->width()*7/10-1,startY+3);
//     painter.drawLine(this->width()*8/10-1,startY+9,this->width()*8/10-1,startY+3);
//     painter.drawLine(this->width()*9/10-1,startY+7,this->width()*9/10-1,startY+3);
//     if(slideScanChanNum == 2) {
//         painter.drawLine(this->width()-2,startY+9,this->width()-2,startY+3);
//     }

//     //短竖线
//     painter.drawLine(this->width()*1/20,startY+5,this->width()*1/20,startY+3);
//     painter.drawLine(this->width()*3/20,startY+5,this->width()*3/20,startY+3);
//     painter.drawLine(this->width()*5/20,startY+5,this->width()*5/20,startY+3);
//     painter.drawLine(this->width()*7/20,startY+5,this->width()*7/20,startY+3);
//     painter.drawLine(this->width()*9/20,startY+5,this->width()*9/20,startY+3);
//     painter.drawLine(this->width()*11/20,startY+5,this->width()*11/20,startY+3);
//     painter.drawLine(this->width()*13/20,startY+5,this->width()*13/20,startY+3);
//     painter.drawLine(this->width()*15/20,startY+5,this->width()*15/20,startY+3);
//     painter.drawLine(this->width()*17/20,startY+5,this->width()*17/20,startY+3);
//     painter.drawLine(this->width()*19/20,startY+5,this->width()*19/20,startY+3);


//     /*-深度范围的字体样式-*/
//     QFont axisFont;
//     axisFont.setBold(true);
//     axisFont.setPointSize(9);
//     painter.setFont(axisFont);
//     aixsPen.setColor(Qt::white);
//     painter.setPen(aixsPen);
//     painter.setOpacity(0.8);

//     if(slideScanChanNum == 1)
//     {
//         if(loRng < 1) {
//             painter.drawText(0,startY+24,QString::number(loRng,'f',1));
//             //            painter.drawText(this->width()-1,startY+24,"0");
//         } else {
//             float diff = (loRng - upRng)/10;
//             painter.drawText(0,startY+24,QString::number(loRng,'f',1));
//             painter.drawText(this->width()*1/5-10,startY+24,QString::number(upRng+diff*8,'f',1));
//             painter.drawText(this->width()*2/5-10,startY+24,QString::number(upRng+diff*6,'f',1));
//             painter.drawText(this->width()*3/5-10,startY+24,QString::number(upRng+diff*4,'f',1));
//             painter.drawText(this->width()*4/5-10,startY+24,QString::number(upRng+diff*2,'f',1));
//             //            painter.drawText(this->width()-20,startY+26,QString::number(upRng,'f',1));
//         }

//     }
//     else if(slideScanChanNum == 2)
//     {
//         if(loRng < 1) {
//             painter.drawText(0,startY+24,"0");
//             painter.drawText(this->width()-1,startY+24,QString::number(loRng,'f',1));
//         } else {
//             float diff = (loRng - upRng)/10;
//             painter.drawText(0,startY+24,QString::number(upRng,'f',1));
//             painter.drawText(this->width()*1/5-10,startY+24,QString::number(upRng+diff*2,'f',1));
//             painter.drawText(this->width()*2/5-10,startY+24,QString::number(upRng+diff*4,'f',1));
//             painter.drawText(this->width()*3/5-10,startY+24,QString::number(upRng+diff*6,'f',1));
//             painter.drawText(this->width()*4/5-10,startY+24,QString::number(upRng+diff*8,'f',1));
//             painter.drawText(this->width()-20,startY+24,QString::number(loRng,'f',1));
//         }
//     }

// }

// void ZySonarAxis_slideScan::setLoRng(float value)
// {
//     loRng = value;
// }
// void ZySonarAxis_slideScan::setUpRng(float value)
// {
//     upRng = value;
// }

// void ZySonarAxis_slideScan::setSlideScanChannelNum(int value)
// {
//     slideScanChanNum = value;
// }

// void ZySonarAxis_slideScan::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::RightButton)
//     {
//         emit signal_MouseDoubleClickedRight();
//     }
// }


// /*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// int ZyColorScheme::backgroundIndex = 0;
// int ZyColorScheme::background[] = {0xffffff, 0x000000, 0x104684};
// int ZyColorScheme::colorLine = 0;
// int ZyColorScheme::onlineColor = 0xff0000;

// /*--------------------------------------------------------------------------------------*/
// int ZyColorScheme::colorScheme_surface[255] = {0};
// int ZyColorScheme::colorScheme_fish[255] = {0};
// int ZyColorScheme::colorScheme_bottom[255] = {0};

// int ZyColorScheme::colorScheme_surface_2[255] = {0};
// int ZyColorScheme::colorScheme_fish_2[255] = {0};
// int ZyColorScheme::colorScheme_bottom_2[255] = {0};

// int ZyColorScheme::colorScheme_surface_channel1[255] = {0};
// int ZyColorScheme::colorScheme_fish_channel1[255] = {0};
// int ZyColorScheme::colorScheme_bottom_channel1[255] = {0};

// int ZyColorScheme::colorScheme_surface_channel2[255] = {0};
// int ZyColorScheme::colorScheme_fish_channel2[255] = {0};
// int ZyColorScheme::colorScheme_bottom_channel2[255] = {0};

// ZyColorScheme::ZyColorScheme(){}
// ZyColorScheme::~ZyColorScheme(){}

// void ZyColorScheme::loadColorScheme(QString fileName)
// {
//     /*-文件是否存在-*/
//     if(!fileName.isEmpty())
//     {
//         readColorToColorList(fileName);

//         getColorFromColorList();
//     }
// }

// void ZyColorScheme::loadColorScheme_2(QString fileName)
// {
//     /*-文件是否存在-*/
//     if(!fileName.isEmpty()) {
//         readColorToColorList_2(fileName);
//         getColorFromColorList_2();
//     }
// }

// void ZyColorScheme::loadColorScheme_channel1(QString fileName)
// {
//     /*-文件是否存在-*/
//     if(!fileName.isEmpty()) {
//         readColorToColorList_channel1(fileName);
//         getColorFromColorList_channel1();
//     }
// }

// void ZyColorScheme::loadColorScheme_channel2(QString fileName)
// {
//     /*-文件是否存在-*/
//     if(!fileName.isEmpty()) {
//         readColorToColorList_channel2(fileName);
//         getColorFromColorList_channel2();
//     }
// }


// void ZyColorScheme::saveColorScheme()
// {
//     QString fileName = qApp->applicationDirPath() + "/dcs_caise.tcs";

//     QSettings selfColor(fileName, QSettings::IniFormat);

//     selfColor.beginGroup("surface");
//     selfColor.setValue("count", colorList_surface.count());
//     for(int i=0;i<colorList_surface.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_surface.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_surface.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("fish");
//     selfColor.setValue("count", colorList_fish.count());
//     for(int i=0;i<colorList_fish.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_fish.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_fish.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("bottom");
//     selfColor.setValue("count", colorList_bottom.count());
//     for(int i=0;i<colorList_bottom.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_bottom.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_bottom.at(i).colorValue);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::saveColorScheme_2()
// {
//     QString fileName = qApp->applicationDirPath() + "/dcs_2.tcs";

//     QSettings selfColor(fileName, QSettings::IniFormat);

//     selfColor.beginGroup("surface");
//     selfColor.setValue("count", colorList_surface_2.count());
//     for(int i=0;i<colorList_surface_2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_surface_2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_surface_2.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("fish");
//     selfColor.setValue("count", colorList_fish_2.count());
//     for(int i=0;i<colorList_fish_2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_fish_2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_fish_2.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("bottom");
//     selfColor.setValue("count", colorList_bottom_2.count());
//     for(int i=0;i<colorList_bottom_2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_bottom_2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_bottom_2.at(i).colorValue);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::saveColorScheme_channel1()
// {
//     QString fileName = qApp->applicationDirPath() + "/dcs_2.tcs";

//     QSettings selfColor(fileName, QSettings::IniFormat);

//     selfColor.beginGroup("surface");
//     selfColor.setValue("count", colorList_surface_channel1.count());
//     for(int i=0;i<colorList_surface_channel1.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_surface_channel1.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_surface_channel1.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("fish");
//     selfColor.setValue("count", colorList_fish_channel1.count());
//     for(int i=0;i<colorList_fish_channel1.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_fish_channel1.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_fish_channel1.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("bottom");
//     selfColor.setValue("count", colorList_bottom_channel1.count());
//     for(int i=0;i<colorList_bottom_channel1.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_bottom_channel1.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_bottom_channel1.at(i).colorValue);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::saveColorScheme_channel2()
// {
//     QString fileName = qApp->applicationDirPath() + "/dcs_2.tcs";

//     QSettings selfColor(fileName, QSettings::IniFormat);

//     selfColor.beginGroup("surface");
//     selfColor.setValue("count", colorList_surface_channel2.count());
//     for(int i=0;i<colorList_surface_channel2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_surface_channel2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_surface_channel2.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("fish");
//     selfColor.setValue("count", colorList_fish_channel2.count());
//     for(int i=0;i<colorList_fish_channel2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_fish_channel2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_fish_channel2.at(i).colorValue);
//     }
//     selfColor.endGroup();

//     selfColor.beginGroup("bottom");
//     selfColor.setValue("count", colorList_bottom_channel2.count());
//     for(int i=0;i<colorList_bottom_channel2.count();i++)
//     {
//         selfColor.setValue("position_"+QString::number(i), colorList_bottom_channel2.at(i).colorPosition);
//         selfColor.setValue("value_"+QString::number(i), colorList_bottom_channel2.at(i).colorValue);
//     }
//     selfColor.endGroup();
// }


// void ZyColorScheme::readColorToColorList(QString fileName)
// {
//     /*-用于存取自定义色表-*/
//     QSettings selfColor(fileName, QSettings::IniFormat);
//     StructColorList tmp;

//     /*-自定义配色表读取-*/
//     colorList_surface.clear();
//     selfColor.beginGroup("surface");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++) {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_surface.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_fish.clear();
//     selfColor.beginGroup("fish");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++) {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_fish.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_bottom.clear();
//     selfColor.beginGroup("bottom");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++) {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_bottom.append(tmp);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::readColorToColorList_2(QString fileName)
// {
//     /*-用于存取自定义色表-*/
//     QSettings selfColor(fileName, QSettings::IniFormat);
//     StructColorList tmp;

//     /*-自定义配色表读取-*/
//     colorList_surface_2.clear();
//     selfColor.beginGroup("surface");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_surface_2.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_fish_2.clear();
//     selfColor.beginGroup("fish");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_fish_2.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_bottom_2.clear();
//     selfColor.beginGroup("bottom");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_bottom_2.append(tmp);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::readColorToColorList_channel1(QString fileName)
// {
//     /*-用于存取自定义色表-*/
//     QSettings selfColor(fileName, QSettings::IniFormat);
//     StructColorList tmp;

//     /*-自定义配色表读取-*/
//     colorList_surface_channel1.clear();
//     selfColor.beginGroup("surface");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_surface_channel1.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_fish_channel1.clear();
//     selfColor.beginGroup("fish");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_fish_channel1.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_bottom_channel1.clear();
//     selfColor.beginGroup("bottom");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_bottom_channel1.append(tmp);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::readColorToColorList_channel2(QString fileName)
// {
//     /*-用于存取自定义色表-*/
//     QSettings selfColor(fileName, QSettings::IniFormat);
//     StructColorList tmp;

//     /*-自定义配色表读取-*/
//     colorList_surface_channel2.clear();
//     selfColor.beginGroup("surface");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_surface_channel2.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_fish_channel2.clear();
//     selfColor.beginGroup("fish");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_fish_channel2.append(tmp);
//     }
//     selfColor.endGroup();

//     colorList_bottom_channel2.clear();
//     selfColor.beginGroup("bottom");
//     for(int i=0;i<selfColor.value("count",0).toInt();i++)
//     {
//         tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
//         tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
//         colorList_bottom_channel2.append(tmp);
//     }
//     selfColor.endGroup();
// }

// void ZyColorScheme::getColorFromColorList()
// {
//     /*-表层渐变显示信号颜色图-*/
//     QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
//     QPixmap pixmap(100,256);
//     QPainter painter;
//     QImage GradientImage;

//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_surface.count(); i++)
//     {
//         linearGradient_Surface.setColorAt(colorList_surface.at(i).colorPosition/255.0, colorList_surface.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Surface);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_surface[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_fish.count(); i++)
//     {
//         linearGradient_Fish.setColorAt(colorList_fish.at(i).colorPosition/255.0, colorList_fish.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Fish);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_fish[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_bottom.count(); i++)
//     {
//         linearGradient_Bottom.setColorAt(colorList_bottom.at(i).colorPosition/255.0, colorList_bottom.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Bottom);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_bottom[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
// }

// void ZyColorScheme::getColorFromColorList_2()
// {
//     /*-表层渐变显示信号颜色图-*/
//     QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
//     QPixmap pixmap(100,256);
//     QPainter painter;
//     QImage GradientImage;

//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_surface_2.count(); i++)
//     {
//         linearGradient_Surface.setColorAt(colorList_surface_2.at(i).colorPosition/255.0, colorList_surface_2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Surface);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_surface_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_fish_2.count(); i++)
//     {
//         linearGradient_Fish.setColorAt(colorList_fish_2.at(i).colorPosition/255.0, colorList_fish_2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Fish);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_fish_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_bottom_2.count(); i++)
//     {
//         linearGradient_Bottom.setColorAt(colorList_bottom_2.at(i).colorPosition/255.0, colorList_bottom_2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Bottom);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_bottom_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
// }


// void ZyColorScheme::getColorFromColorList_channel1()
// {
//     /*-表层渐变显示信号颜色图-*/
//     QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
//     QPixmap pixmap(100,256);
//     QPainter painter;
//     QImage GradientImage;

//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_surface_channel1.count(); i++)
//     {
//         linearGradient_Surface.setColorAt(colorList_surface_channel1.at(i).colorPosition/255.0, colorList_surface_channel1.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Surface);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_surface_channel1[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_fish_channel1.count(); i++)
//     {
//         linearGradient_Fish.setColorAt(colorList_fish_channel1.at(i).colorPosition/255.0, colorList_fish_channel1.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Fish);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_fish_channel1[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_bottom_channel1.count(); i++)
//     {
//         linearGradient_Bottom.setColorAt(colorList_bottom_channel1.at(i).colorPosition/255.0, colorList_bottom_channel1.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Bottom);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_bottom_channel1[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
// }

// void ZyColorScheme::getColorFromColorList_channel2()
// {
//     /*-表层渐变显示信号颜色图-*/
//     QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
//     QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
//     QPixmap pixmap(100,256);
//     QPainter painter;
//     QImage GradientImage;

//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_surface_channel2.count(); i++)
//     {
//         linearGradient_Surface.setColorAt(colorList_surface_channel2.at(i).colorPosition/255.0, colorList_surface_channel2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Surface);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_surface_channel2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_fish_channel2.count(); i++)
//     {
//         linearGradient_Fish.setColorAt(colorList_fish_channel2.at(i).colorPosition/255.0, colorList_fish_channel2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Fish);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_fish_channel2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
//     /*-颜色链表中按顺序进行渐变-*/
//     for(int i = 0; i<colorList_bottom_channel2.count(); i++)
//     {
//         linearGradient_Bottom.setColorAt(colorList_bottom_channel2.at(i).colorPosition/255.0, colorList_bottom_channel2.at(i).colorValue);
//     }
//     painter.begin(&pixmap);
//     painter.setBrush(linearGradient_Bottom);
//     painter.drawRect(0, 0, 100, 256);
//     painter.end();
//     GradientImage = pixmap.toImage();
//     for(int i=0;i<255;i++)
//     {
//         colorScheme_bottom_channel2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
//     }
// }


// int ZyColorScheme::getOnlineColor()
// {
//     return onlineColor;
// }
// void ZyColorScheme::setOnlineColor(int value)
// {
//     onlineColor = value;
// }

// int ZyColorScheme::getBackgroundIndex()
// {
//     return backgroundIndex;
// }
// void ZyColorScheme::setBackgroundIndex(int value)
// {
//     backgroundIndex = value;
// }




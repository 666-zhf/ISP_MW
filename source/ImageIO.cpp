#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ImageIO.h"
#include "Helper.h"
#include "LUT.h"


Frame ImageReader(const std::string &filename, const FCType FrameNum, const DType BitDepth)
{
    cv::Mat image = cv::imread(filename, cv::IMREAD_COLOR);

    if (!image.data) // Check for invalid input
    {
        std::cerr << "Could not open or find the image file: " << filename << std::endl;

        Frame src(FrameNum, PixelType::RGB, 1920, 1080, BitDepth, true);
        return src;
    }

    const PCType sw = image.cols;
    const PCType sh = image.rows;
    const PCType nCols = sw * image.channels();
    const PCType nRows = sh;

    Frame src(FrameNum, PixelType::RGB, sw, sh, BitDepth, false);

    const PCType height = src.Height();
    const PCType width = src.Width();
    const PCType stride = src.Stride();

    Plane &R = src.R();
    Plane &G = src.G();
    Plane &B = src.B();

    if (R.Floor() == 0 && R.Ceil() == 65535)
    {
        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                B[i] = static_cast<DType>(*p++) * DType(257);
                G[i] = static_cast<DType>(*p++) * DType(257);
                R[i] = static_cast<DType>(*p++) * DType(257);
            }
        }
    }
    else if (R.Floor() == 0 && R.Ceil() == 255)
    {
        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                B[i] = static_cast<DType>(*p++);
                G[i] = static_cast<DType>(*p++);
                R[i] = static_cast<DType>(*p++);
            }
        }
    }
    else
    {
        LUT<DType>::LevelType k;
        const LUT<DType>::LevelType iLevels = 256;
        LUT<DType> ConvertLUT(iLevels);

        for (k = 0; k < iLevels; k++)
        {
            ConvertLUT[k] = R.GetD(static_cast<FLType>(k) / FLType(255));
        }

        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                B[i] = ConvertLUT[*p++];
                G[i] = ConvertLUT[*p++];
                R[i] = ConvertLUT[*p++];
            }
        }
    }

    return src;
}


bool ImageWriter(const Frame &src, const std::string &filename)
{
    return ImageWriter(src, filename, CV_8UC3);
}

bool ImageWriter(const Frame &src, const std::string &filename, int _type)
{
    const PCType height = src.Height();
    const PCType width = src.Width();
    const PCType stride = src.Stride();

    const Plane &R = src.R();
    const Plane &G = src.G();
    const Plane &B = src.B();

    cv::Mat image(height, width, _type);

    if (R.Floor() == 0 && R.Ceil() == 65535)
    {
        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                *p++ = static_cast<uchar>(Round_Div(B[i], DType(257)));
                *p++ = static_cast<uchar>(Round_Div(G[i], DType(257)));
                *p++ = static_cast<uchar>(Round_Div(R[i], DType(257)));
            }
        }
    }
    else if (R.Floor() == 0 && R.Ceil() == 255)
    {
        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                *p++ = static_cast<uchar>(B[i]);
                *p++ = static_cast<uchar>(G[i]);
                *p++ = static_cast<uchar>(R[i]);
            }
        }
    }
    else
    {
        LUT<uchar>::LevelType k;
        LUT<uchar> ConvertLUT(R);

        for (k = R.Floor(); k < R.Ceil(); k++)
        {
            ConvertLUT.Set(R, k, static_cast<uchar>(R.GetFL(k) * 255. + 0.5));
        }

        for (PCType j = 0; j < height; ++j)
        {
            auto p = image.ptr<uchar>(j);
            PCType i = j * stride;

            for (const PCType upper = i + width; i < upper; ++i)
            {
                *p++ = ConvertLUT.Lookup(B, B[i]);
                *p++ = ConvertLUT.Lookup(G, G[i]);
                *p++ = ConvertLUT.Lookup(R, R[i]);
            }
        }
    }
    
    cv::imwrite(filename, image);

    return true;
}

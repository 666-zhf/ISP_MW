﻿#include <iostream>
#include <cmath>
#include "include\Bilateral.h"
#include "include\Type_Conv.h"
#include "include\Image_Type.h"
#include "include\IO.h"


// Implementation of specular highilght removal algorithm from "Qingxiong Yang, ShengnanWang, and Narendra Ahuja - Real-Time Specular Highlight Removal Using Bilateral Filtering"
Frame_RGB & Specular_Highlight_Removal(Frame_RGB & output, const Frame_RGB & input, const double thr, const double sigmaS, const double sigmaR, const DType PBFICnum)
{
    PCType i, flag = 1;
    PCType pcount = input.PixelCount();
    DType ValueRange = input.R().ValueRange();

    DType R, G, B;
    FLType RGBsum, sigma_R, sigma_G, sigma_B, sigmaMax, sigmaMin;
    Plane PsigmaMax(input.R(), false);
    Plane PlambdaMax(input.R(), false);
    Plane PsigmaMaxF(input.R(), false);

    PsigmaMax.Floor(0);
    PsigmaMax.Ceil(ValueRange);
    PlambdaMax.Floor(0);
    PlambdaMax.Ceil(ValueRange);
    PsigmaMaxF.Floor(0);
    PsigmaMaxF.Ceil(ValueRange);

    DType diffthr = (DType)(thr * ValueRange + 0.5);
    FLType specular_component;

    // Compute σ_max at every pixel using the input image and store it as a grayscale image.
    // Compute λ_max at every pixel using the input image and store it as a grayscale image.
    for (i = 0; i < pcount; i++)
    {
        R = input.R()[i];
        G = input.G()[i];
        B = input.B()[i];

        RGBsum = (FLType)(R + G + B);
        if (RGBsum == 0)
        {
            sigma_R = sigma_G = sigma_B = 1. / 3.;
        }
        else
        {
            sigma_R = (FLType)R / RGBsum;
            sigma_G = (FLType)G / RGBsum;
            sigma_B = (FLType)B / RGBsum;
        }

        sigmaMax = Max(Max(sigma_R, sigma_G), sigma_B);
        sigmaMin = Min(Min(sigma_R, sigma_G), sigma_B);

        PsigmaMax[i] = (DType)(sigmaMax * ValueRange + 0.5);
        PlambdaMax[i] = sigmaMax == sigmaMin ? 0 : (DType)((sigmaMax - sigmaMin) / (1 - 3 * sigmaMin) * ValueRange + 0.5);
    }

    // repeat until σ_maxF − σ_max < 0.03 at every pixel.
    while (flag)
    {
        // Apply joint bilateral filter to image σmax using λ_max as the guidance image, store the filtered image as σF σ_maxF
        Bilateral2D(PsigmaMaxF, PsigmaMax, PlambdaMax, sigmaS, sigmaR, PBFICnum);

        // For each pixel p, σ_max(p) = max(σ_max(p), σ_maxF(p))
        for (i = 0, flag = 0; i < pcount; i++)
        {
            if (PsigmaMaxF[i] > PsigmaMax[i])
            {
                if (PsigmaMaxF[i] - PsigmaMax[i] > diffthr)
                {
                    flag++;
                }

                PsigmaMax[i] = PsigmaMaxF[i];
            }
        }
    }

    // Compute diffuse component
    for (i = 0; i < pcount; i++)
    {
        R = input.R()[i];
        G = input.G()[i];
        B = input.B()[i];
        sigmaMax = (FLType)PsigmaMax[i] / ValueRange;

        if (PsigmaMax[i] * 3 <= ValueRange)
        {
            output.R()[i] = R;
            output.G()[i] = G;
            output.B()[i] = B;
        }
        else
        {
            specular_component = (Max(Max(R, G), B) - sigmaMax*(R + G + B)) / (1 - 3 * sigmaMax);
            output.R()[i] = Quantize(R - specular_component, output.R());
            output.G()[i] = Quantize(G - specular_component, output.G());
            output.B()[i] = Quantize(B - specular_component, output.B());
        }
        /*output.R()[i] = PsigmaMax[i];
        output.G()[i] = PsigmaMax[i];
        output.B()[i] = PsigmaMax[i];*/
        /*output.R()[i] = PlambdaMax[i];
        output.G()[i] = PlambdaMax[i];
        output.B()[i] = PlambdaMax[i];*/
    }

    // Output
    return output;
}
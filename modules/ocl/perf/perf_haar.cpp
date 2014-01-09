/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Multicoreware, Inc., all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Fangfang Bai, fangfang@multicorewareinc.com
//    Jin Ma,       jin@multicorewareinc.com
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors as is and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/
#include "perf_precomp.hpp"

using namespace perf;

///////////// Haar ////////////////////////
PERF_TEST(HaarFixture, Haar)
{
    vector<Rect> faces;

    Mat img = imread(getDataPath("gpu/haarcascade/basketball1.png"), CV_LOAD_IMAGE_GRAYSCALE);
    ASSERT_TRUE(!img.empty()) << "can't open basketball1.png";
    declare.in(img);

    if (RUN_PLAIN_IMPL)
    {
        CascadeClassifier faceCascade;
        ASSERT_TRUE(faceCascade.load(getDataPath("gpu/haarcascade/haarcascade_frontalface_alt.xml")))
                << "can't load haarcascade_frontalface_alt.xml";

        TEST_CYCLE() faceCascade.detectMultiScale(img, faces,
                                                     1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

        SANITY_CHECK(faces, 4 + 1e-4);
    }
    else if (RUN_OCL_IMPL)
    {
        ocl::OclCascadeClassifier faceCascade;
        ocl::oclMat oclImg(img);

        ASSERT_TRUE(faceCascade.load(getDataPath("gpu/haarcascade/haarcascade_frontalface_alt.xml")))
                << "can't load haarcascade_frontalface_alt.xml";

        OCL_TEST_CYCLE() faceCascade.detectMultiScale(oclImg, faces,
                                     1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

        SANITY_CHECK(faces, 4 + 1e-4);
    }
    else
        OCL_PERF_ELSE
}

using namespace std;
using namespace cv;
using namespace perf;
using std::tr1::make_tuple;
using std::tr1::get;

typedef std::tr1::tuple<std::string, std::string, int> Cascade_Image_MinSize_t;
typedef perf::TestBaseWithParam<Cascade_Image_MinSize_t> Cascade_Image_MinSize;

PERF_TEST_P( Cascade_Image_MinSize, CascadeClassifier_UMat,
             testing::Combine(
                testing::Values( string("cv/cascadeandhog/cascades/haarcascade_frontalface_alt.xml") ),
                testing::Values( string("cv/shared/lena.png"),
                                 string("cv/cascadeandhog/images/bttf301.png"),
                                 string("cv/cascadeandhog/images/class57.png") ),
                testing::Values(30, 64, 90) ) )
{
    const string cascasePath = get<0>(GetParam());
    const string imagePath   = get<1>(GetParam());
    const int min_size = get<2>(GetParam());
    Size minSize(min_size, min_size);

    ocl::OclCascadeClassifier cc;
    if (!cc.load( getDataPath(cascasePath) ))
        FAIL() << "Can't load cascade file: " << getDataPath(cascasePath);

    Mat img = imread(getDataPath(imagePath), IMREAD_GRAYSCALE);
    if (img.empty())
        FAIL() << "Can't load source image: " << getDataPath(imagePath);

    vector<Rect> faces;

    equalizeHist(img, img);
    declare.in(img).time(60);

    ocl::oclMat uimg(img);

    while(next())
    {
        faces.clear();

        startTimer();
        cc.detectMultiScale(uimg, faces, 1.1, 3, 0, minSize);
        stopTimer();
    }

    //sort(faces.begin(), faces.end(), comparators::RectLess());
    SANITY_CHECK_NOTHING();//(faces, min_size/5);
}

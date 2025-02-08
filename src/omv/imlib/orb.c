/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2009, Willow Garage, Inc.
 * Copyright (C) 2013-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. Neither the name of the Willow Garage nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ORB keypoints descriptor based on OpenCV ORB detector.
 */
#include "imlib.h"
#if defined(IMLIB_ENABLE_FIND_KEYPOINTS)
#include "fmath.h"
#include "arm_math.h"
#include "xalloc.h"
#include "fb_alloc.h"
#include "file_utils.h"

#define PATCH_SIZE     (31) // 31x31 pixels
#define KDESC_SIZE     (32) // 32 bytes
#define MAX_KP_DIST    (KDESC_SIZE * 8)

typedef struct {
    int x;
    int y;
} sample_point_t;

const static int u_max[] = {
    15, 15, 15, 15, 14, 14, 14, 13,
    13, 12, 11, 10, 9, 8, 6, 3, 0
};

const static int sample_pattern[256 * 4] = {
    8, -3, 9, 5 /*mean (0), correlation (0)*/,
    4, 2, 7, -12 /*mean (1.12461e-05), correlation (0.0437584)*/,
    -11, 9, -8, 2 /*mean (3.37382e-05), correlation (0.0617409)*/,
    7, -12, 12, -13 /*mean (5.62303e-05), correlation (0.0636977)*/,
    2, -13, 2, 12 /*mean (0.000134953), correlation (0.085099)*/,
    1, -7, 1, 6 /*mean (0.000528565), correlation (0.0857175)*/,
    -2, -10, -2, -4 /*mean (0.0188821), correlation (0.0985774)*/,
    -13, -13, -11, -8 /*mean (0.0363135), correlation (0.0899616)*/,
    -13, -3, -12, -9 /*mean (0.121806), correlation (0.099849)*/,
    10, 4, 11, 9 /*mean (0.122065), correlation (0.093285)*/,
    -13, -8, -8, -9 /*mean (0.162787), correlation (0.0942748)*/,
    -11, 7, -9, 12 /*mean (0.21561), correlation (0.0974438)*/,
    7, 7, 12, 6 /*mean (0.160583), correlation (0.130064)*/,
    -4, -5, -3, 0 /*mean (0.228171), correlation (0.132998)*/,
    -13, 2, -12, -3 /*mean (0.00997526), correlation (0.145926)*/,
    -9, 0, -7, 5 /*mean (0.198234), correlation (0.143636)*/,
    12, -6, 12, -1 /*mean (0.0676226), correlation (0.16689)*/,
    -3, 6, -2, 12 /*mean (0.166847), correlation (0.171682)*/,
    -6, -13, -4, -8 /*mean (0.101215), correlation (0.179716)*/,
    11, -13, 12, -8 /*mean (0.200641), correlation (0.192279)*/,
    4, 7, 5, 1 /*mean (0.205106), correlation (0.186848)*/,
    5, -3, 10, -3 /*mean (0.234908), correlation (0.192319)*/,
    3, -7, 6, 12 /*mean (0.0709964), correlation (0.210872)*/,
    -8, -7, -6, -2 /*mean (0.0939834), correlation (0.212589)*/,
    -2, 11, -1, -10 /*mean (0.127778), correlation (0.20866)*/,
    -13, 12, -8, 10 /*mean (0.14783), correlation (0.206356)*/,
    -7, 3, -5, -3 /*mean (0.182141), correlation (0.198942)*/,
    -4, 2, -3, 7 /*mean (0.188237), correlation (0.21384)*/,
    -10, -12, -6, 11 /*mean (0.14865), correlation (0.23571)*/,
    5, -12, 6, -7 /*mean (0.222312), correlation (0.23324)*/,
    5, -6, 7, -1 /*mean (0.229082), correlation (0.23389)*/,
    1, 0, 4, -5 /*mean (0.241577), correlation (0.215286)*/,
    9, 11, 11, -13 /*mean (0.00338507), correlation (0.251373)*/,
    4, 7, 4, 12 /*mean (0.131005), correlation (0.257622)*/,
    2, -1, 4, 4 /*mean (0.152755), correlation (0.255205)*/,
    -4, -12, -2, 7 /*mean (0.182771), correlation (0.244867)*/,
    -8, -5, -7, -10 /*mean (0.186898), correlation (0.23901)*/,
    4, 11, 9, 12 /*mean (0.226226), correlation (0.258255)*/,
    0, -8, 1, -13 /*mean (0.0897886), correlation (0.274827)*/,
    -13, -2, -8, 2 /*mean (0.148774), correlation (0.28065)*/,
    -3, -2, -2, 3 /*mean (0.153048), correlation (0.283063)*/,
    -6, 9, -4, -9 /*mean (0.169523), correlation (0.278248)*/,
    8, 12, 10, 7 /*mean (0.225337), correlation (0.282851)*/,
    0, 9, 1, 3 /*mean (0.226687), correlation (0.278734)*/,
    7, -5, 11, -10 /*mean (0.00693882), correlation (0.305161)*/,
    -13, -6, -11, 0 /*mean (0.0227283), correlation (0.300181)*/,
    10, 7, 12, 1 /*mean (0.125517), correlation (0.31089)*/,
    -6, -3, -6, 12 /*mean (0.131748), correlation (0.312779)*/,
    10, -9, 12, -4 /*mean (0.144827), correlation (0.292797)*/,
    -13, 8, -8, -12 /*mean (0.149202), correlation (0.308918)*/,
    -13, 0, -8, -4 /*mean (0.160909), correlation (0.310013)*/,
    3, 3, 7, 8 /*mean (0.177755), correlation (0.309394)*/,
    5, 7, 10, -7 /*mean (0.212337), correlation (0.310315)*/,
    -1, 7, 1, -12 /*mean (0.214429), correlation (0.311933)*/,
    3, -10, 5, 6 /*mean (0.235807), correlation (0.313104)*/,
    2, -4, 3, -10 /*mean (0.00494827), correlation (0.344948)*/,
    -13, 0, -13, 5 /*mean (0.0549145), correlation (0.344675)*/,
    -13, -7, -12, 12 /*mean (0.103385), correlation (0.342715)*/,
    -13, 3, -11, 8 /*mean (0.134222), correlation (0.322922)*/,
    -7, 12, -4, 7 /*mean (0.153284), correlation (0.337061)*/,
    6, -10, 12, 8 /*mean (0.154881), correlation (0.329257)*/,
    -9, -1, -7, -6 /*mean (0.200967), correlation (0.33312)*/,
    -2, -5, 0, 12 /*mean (0.201518), correlation (0.340635)*/,
    -12, 5, -7, 5 /*mean (0.207805), correlation (0.335631)*/,
    3, -10, 8, -13 /*mean (0.224438), correlation (0.34504)*/,
    -7, -7, -4, 5 /*mean (0.239361), correlation (0.338053)*/,
    -3, -2, -1, -7 /*mean (0.240744), correlation (0.344322)*/,
    2, 9, 5, -11 /*mean (0.242949), correlation (0.34145)*/,
    -11, -13, -5, -13 /*mean (0.244028), correlation (0.336861)*/,
    -1, 6, 0, -1 /*mean (0.247571), correlation (0.343684)*/,
    5, -3, 5, 2 /*mean (0.000697256), correlation (0.357265)*/,
    -4, -13, -4, 12 /*mean (0.00213675), correlation (0.373827)*/,
    -9, -6, -9, 6 /*mean (0.0126856), correlation (0.373938)*/,
    -12, -10, -8, -4 /*mean (0.0152497), correlation (0.364237)*/,
    10, 2, 12, -3 /*mean (0.0299933), correlation (0.345292)*/,
    7, 12, 12, 12 /*mean (0.0307242), correlation (0.366299)*/,
    -7, -13, -6, 5 /*mean (0.0534975), correlation (0.368357)*/,
    -4, 9, -3, 4 /*mean (0.099865), correlation (0.372276)*/,
    7, -1, 12, 2 /*mean (0.117083), correlation (0.364529)*/,
    -7, 6, -5, 1 /*mean (0.126125), correlation (0.369606)*/,
    -13, 11, -12, 5 /*mean (0.130364), correlation (0.358502)*/,
    -3, 7, -2, -6 /*mean (0.131691), correlation (0.375531)*/,
    7, -8, 12, -7 /*mean (0.160166), correlation (0.379508)*/,
    -13, -7, -11, -12 /*mean (0.167848), correlation (0.353343)*/,
    1, -3, 12, 12 /*mean (0.183378), correlation (0.371916)*/,
    2, -6, 3, 0 /*mean (0.228711), correlation (0.371761)*/,
    -4, 3, -2, -13 /*mean (0.247211), correlation (0.364063)*/,
    -1, -13, 1, 9 /*mean (0.249325), correlation (0.378139)*/,
    7, 1, 8, -6 /*mean (0.000652272), correlation (0.411682)*/,
    1, -1, 3, 12 /*mean (0.00248538), correlation (0.392988)*/,
    9, 1, 12, 6 /*mean (0.0206815), correlation (0.386106)*/,
    -1, -9, -1, 3 /*mean (0.0364485), correlation (0.410752)*/,
    -13, -13, -10, 5 /*mean (0.0376068), correlation (0.398374)*/,
    7, 7, 10, 12 /*mean (0.0424202), correlation (0.405663)*/,
    12, -5, 12, 9 /*mean (0.0942645), correlation (0.410422)*/,
    6, 3, 7, 11 /*mean (0.1074), correlation (0.413224)*/,
    5, -13, 6, 10 /*mean (0.109256), correlation (0.408646)*/,
    2, -12, 2, 3 /*mean (0.131691), correlation (0.416076)*/,
    3, 8, 4, -6 /*mean (0.165081), correlation (0.417569)*/,
    2, 6, 12, -13 /*mean (0.171874), correlation (0.408471)*/,
    9, -12, 10, 3 /*mean (0.175146), correlation (0.41296)*/,
    -8, 4, -7, 9 /*mean (0.183682), correlation (0.402956)*/,
    -11, 12, -4, -6 /*mean (0.184672), correlation (0.416125)*/,
    1, 12, 2, -8 /*mean (0.191487), correlation (0.386696)*/,
    6, -9, 7, -4 /*mean (0.192668), correlation (0.394771)*/,
    2, 3, 3, -2 /*mean (0.200157), correlation (0.408303)*/,
    6, 3, 11, 0 /*mean (0.204588), correlation (0.411762)*/,
    3, -3, 8, -8 /*mean (0.205904), correlation (0.416294)*/,
    7, 8, 9, 3 /*mean (0.213237), correlation (0.409306)*/,
    -11, -5, -6, -4 /*mean (0.243444), correlation (0.395069)*/,
    -10, 11, -5, 10 /*mean (0.247672), correlation (0.413392)*/,
    -5, -8, -3, 12 /*mean (0.24774), correlation (0.411416)*/,
    -10, 5, -9, 0 /*mean (0.00213675), correlation (0.454003)*/,
    8, -1, 12, -6 /*mean (0.0293635), correlation (0.455368)*/,
    4, -6, 6, -11 /*mean (0.0404971), correlation (0.457393)*/,
    -10, 12, -8, 7 /*mean (0.0481107), correlation (0.448364)*/,
    4, -2, 6, 7 /*mean (0.050641), correlation (0.455019)*/,
    -2, 0, -2, 12 /*mean (0.0525978), correlation (0.44338)*/,
    -5, -8, -5, 2 /*mean (0.0629667), correlation (0.457096)*/,
    7, -6, 10, 12 /*mean (0.0653846), correlation (0.445623)*/,
    -9, -13, -8, -8 /*mean (0.0858749), correlation (0.449789)*/,
    -5, -13, -5, -2 /*mean (0.122402), correlation (0.450201)*/,
    8, -8, 9, -13 /*mean (0.125416), correlation (0.453224)*/,
    -9, -11, -9, 0 /*mean (0.130128), correlation (0.458724)*/,
    1, -8, 1, -2 /*mean (0.132467), correlation (0.440133)*/,
    7, -4, 9, 1 /*mean (0.132692), correlation (0.454)*/,
    -2, 1, -1, -4 /*mean (0.135695), correlation (0.455739)*/,
    11, -6, 12, -11 /*mean (0.142904), correlation (0.446114)*/,
    -12, -9, -6, 4 /*mean (0.146165), correlation (0.451473)*/,
    3, 7, 7, 12 /*mean (0.147627), correlation (0.456643)*/,
    5, 5, 10, 8 /*mean (0.152901), correlation (0.455036)*/,
    0, -4, 2, 8 /*mean (0.167083), correlation (0.459315)*/,
    -9, 12, -5, -13 /*mean (0.173234), correlation (0.454706)*/,
    0, 7, 2, 12 /*mean (0.18312), correlation (0.433855)*/,
    -1, 2, 1, 7 /*mean (0.185504), correlation (0.443838)*/,
    5, 11, 7, -9 /*mean (0.185706), correlation (0.451123)*/,
    3, 5, 6, -8 /*mean (0.188968), correlation (0.455808)*/,
    -13, -4, -8, 9 /*mean (0.191667), correlation (0.459128)*/,
    -5, 9, -3, -3 /*mean (0.193196), correlation (0.458364)*/,
    -4, -7, -3, -12 /*mean (0.196536), correlation (0.455782)*/,
    6, 5, 8, 0 /*mean (0.1972), correlation (0.450481)*/,
    -7, 6, -6, 12 /*mean (0.199438), correlation (0.458156)*/,
    -13, 6, -5, -2 /*mean (0.211224), correlation (0.449548)*/,
    1, -10, 3, 10 /*mean (0.211718), correlation (0.440606)*/,
    4, 1, 8, -4 /*mean (0.213034), correlation (0.443177)*/,
    -2, -2, 2, -13 /*mean (0.234334), correlation (0.455304)*/,
    2, -12, 12, 12 /*mean (0.235684), correlation (0.443436)*/,
    -2, -13, 0, -6 /*mean (0.237674), correlation (0.452525)*/,
    4, 1, 9, 3 /*mean (0.23962), correlation (0.444824)*/,
    -6, -10, -3, -5 /*mean (0.248459), correlation (0.439621)*/,
    -3, -13, -1, 1 /*mean (0.249505), correlation (0.456666)*/,
    7, 5, 12, -11 /*mean (0.00119208), correlation (0.495466)*/,
    4, -2, 5, -7 /*mean (0.00372245), correlation (0.484214)*/,
    -13, 9, -9, -5 /*mean (0.00741116), correlation (0.499854)*/,
    7, 1, 8, 6 /*mean (0.0208952), correlation (0.499773)*/,
    7, -8, 7, 6 /*mean (0.0220085), correlation (0.501609)*/,
    -7, -4, -7, 1 /*mean (0.0233806), correlation (0.496568)*/,
    -8, 11, -7, -8 /*mean (0.0236505), correlation (0.489719)*/,
    -13, 6, -12, -8 /*mean (0.0268781), correlation (0.503487)*/,
    2, 4, 3, 9 /*mean (0.0323324), correlation (0.501938)*/,
    10, -5, 12, 3 /*mean (0.0399235), correlation (0.494029)*/,
    -6, -5, -6, 7 /*mean (0.0420153), correlation (0.486579)*/,
    8, -3, 9, -8 /*mean (0.0548021), correlation (0.484237)*/,
    2, -12, 2, 8 /*mean (0.0616622), correlation (0.496642)*/,
    -11, -2, -10, 3 /*mean (0.0627755), correlation (0.498563)*/,
    -12, -13, -7, -9 /*mean (0.0829622), correlation (0.495491)*/,
    -11, 0, -10, -5 /*mean (0.0843342), correlation (0.487146)*/,
    5, -3, 11, 8 /*mean (0.0929937), correlation (0.502315)*/,
    -2, -13, -1, 12 /*mean (0.113327), correlation (0.48941)*/,
    -1, -8, 0, 9 /*mean (0.132119), correlation (0.467268)*/,
    -13, -11, -12, -5 /*mean (0.136269), correlation (0.498771)*/,
    -10, -2, -10, 11 /*mean (0.142173), correlation (0.498714)*/,
    -3, 9, -2, -13 /*mean (0.144141), correlation (0.491973)*/,
    2, -3, 3, 2 /*mean (0.14892), correlation (0.500782)*/,
    -9, -13, -4, 0 /*mean (0.150371), correlation (0.498211)*/,
    -4, 6, -3, -10 /*mean (0.152159), correlation (0.495547)*/,
    -4, 12, -2, -7 /*mean (0.156152), correlation (0.496925)*/,
    -6, -11, -4, 9 /*mean (0.15749), correlation (0.499222)*/,
    6, -3, 6, 11 /*mean (0.159211), correlation (0.503821)*/,
    -13, 11, -5, 5 /*mean (0.162427), correlation (0.501907)*/,
    11, 11, 12, 6 /*mean (0.16652), correlation (0.497632)*/,
    7, -5, 12, -2 /*mean (0.169141), correlation (0.484474)*/,
    -1, 12, 0, 7 /*mean (0.169456), correlation (0.495339)*/,
    -4, -8, -3, -2 /*mean (0.171457), correlation (0.487251)*/,
    -7, 1, -6, 7 /*mean (0.175), correlation (0.500024)*/,
    -13, -12, -8, -13 /*mean (0.175866), correlation (0.497523)*/,
    -7, -2, -6, -8 /*mean (0.178273), correlation (0.501854)*/,
    -8, 5, -6, -9 /*mean (0.181107), correlation (0.494888)*/,
    -5, -1, -4, 5 /*mean (0.190227), correlation (0.482557)*/,
    -13, 7, -8, 10 /*mean (0.196739), correlation (0.496503)*/,
    1, 5, 5, -13 /*mean (0.19973), correlation (0.499759)*/,
    1, 0, 10, -13 /*mean (0.204465), correlation (0.49873)*/,
    9, 12, 10, -1 /*mean (0.209334), correlation (0.49063)*/,
    5, -8, 10, -9 /*mean (0.211134), correlation (0.503011)*/,
    -1, 11, 1, -13 /*mean (0.212), correlation (0.499414)*/,
    -9, -3, -6, 2 /*mean (0.212168), correlation (0.480739)*/,
    -1, -10, 1, 12 /*mean (0.212731), correlation (0.502523)*/,
    -13, 1, -8, -10 /*mean (0.21327), correlation (0.489786)*/,
    8, -11, 10, -6 /*mean (0.214159), correlation (0.488246)*/,
    2, -13, 3, -6 /*mean (0.216993), correlation (0.50287)*/,
    7, -13, 12, -9 /*mean (0.223639), correlation (0.470502)*/,
    -10, -10, -5, -7 /*mean (0.224089), correlation (0.500852)*/,
    -10, -8, -8, -13 /*mean (0.228666), correlation (0.502629)*/,
    4, -6, 8, 5 /*mean (0.22906), correlation (0.498305)*/,
    3, 12, 8, -13 /*mean (0.233378), correlation (0.503825)*/,
    -4, 2, -3, -3 /*mean (0.234323), correlation (0.476692)*/,
    5, -13, 10, -12 /*mean (0.236392), correlation (0.475462)*/,
    4, -13, 5, -1 /*mean (0.236842), correlation (0.504132)*/,
    -9, 9, -4, 3 /*mean (0.236977), correlation (0.497739)*/,
    0, 3, 3, -9 /*mean (0.24314), correlation (0.499398)*/,
    -12, 1, -6, 1 /*mean (0.243297), correlation (0.489447)*/,
    3, 2, 4, -8 /*mean (0.00155196), correlation (0.553496)*/,
    -10, -10, -10, 9 /*mean (0.00239541), correlation (0.54297)*/,
    8, -13, 12, 12 /*mean (0.0034413), correlation (0.544361)*/,
    -8, -12, -6, -5 /*mean (0.003565), correlation (0.551225)*/,
    2, 2, 3, 7 /*mean (0.00835583), correlation (0.55285)*/,
    10, 6, 11, -8 /*mean (0.00885065), correlation (0.540913)*/,
    6, 8, 8, -12 /*mean (0.0101552), correlation (0.551085)*/,
    -7, 10, -6, 5 /*mean (0.0102227), correlation (0.533635)*/,
    -3, -9, -3, 9 /*mean (0.0110211), correlation (0.543121)*/,
    -1, -13, -1, 5 /*mean (0.0113473), correlation (0.550173)*/,
    -3, -7, -3, 4 /*mean (0.0140913), correlation (0.554774)*/,
    -8, -2, -8, 3 /*mean (0.017049), correlation (0.55461)*/,
    4, 2, 12, 12 /*mean (0.01778), correlation (0.546921)*/,
    2, -5, 3, 11 /*mean (0.0224022), correlation (0.549667)*/,
    6, -9, 11, -13 /*mean (0.029161), correlation (0.546295)*/,
    3, -1, 7, 12 /*mean (0.0303081), correlation (0.548599)*/,
    11, -1, 12, 4 /*mean (0.0355151), correlation (0.523943)*/,
    -3, 0, -3, 6 /*mean (0.0417904), correlation (0.543395)*/,
    4, -11, 4, 12 /*mean (0.0487292), correlation (0.542818)*/,
    2, -4, 2, 1 /*mean (0.0575124), correlation (0.554888)*/,
    -10, -6, -8, 1 /*mean (0.0594242), correlation (0.544026)*/,
    -13, 7, -11, 1 /*mean (0.0597391), correlation (0.550524)*/,
    -13, 12, -11, -13 /*mean (0.0608974), correlation (0.55383)*/,
    6, 0, 11, -13 /*mean (0.065126), correlation (0.552006)*/,
    0, -1, 1, 4 /*mean (0.074224), correlation (0.546372)*/,
    -13, 3, -9, -2 /*mean (0.0808592), correlation (0.554875)*/,
    -9, 8, -6, -3 /*mean (0.0883378), correlation (0.551178)*/,
    -13, -6, -8, -2 /*mean (0.0901035), correlation (0.548446)*/,
    5, -9, 8, 10 /*mean (0.0949843), correlation (0.554694)*/,
    2, 7, 3, -9 /*mean (0.0994152), correlation (0.550979)*/,
    -1, -6, -1, -1 /*mean (0.10045), correlation (0.552714)*/,
    9, 5, 11, -2 /*mean (0.100686), correlation (0.552594)*/,
    11, -3, 12, -8 /*mean (0.101091), correlation (0.532394)*/,
    3, 0, 3, 5 /*mean (0.101147), correlation (0.525576)*/,
    -1, 4, 0, 10 /*mean (0.105263), correlation (0.531498)*/,
    3, -6, 4, 5 /*mean (0.110785), correlation (0.540491)*/,
    -13, 0, -10, 5 /*mean (0.112798), correlation (0.536582)*/,
    5, 8, 12, 11 /*mean (0.114181), correlation (0.555793)*/,
    8, 9, 9, -6 /*mean (0.117431), correlation (0.553763)*/,
    7, -4, 8, -12 /*mean (0.118522), correlation (0.553452)*/,
    -10, 4, -10, 9 /*mean (0.12094), correlation (0.554785)*/,
    7, 3, 12, 4 /*mean (0.122582), correlation (0.555825)*/,
    9, -7, 10, -2 /*mean (0.124978), correlation (0.549846)*/,
    7, 0, 12, -2 /*mean (0.127002), correlation (0.537452)*/,
    -1, -6, 0, -11/*mean (0.127148), correlation (0.547401)*/
};

static int kpt_comp(const kp_t *kp1, const kp_t *kp2) {
    // Descending order
    return kp2->score - kp1->score;
}

static int comp_angle(image_t *img, kp_t *kp, float *a, float *b) {
    int step = img->w;
    int half_k = 31 / 2;
    int m_01 = 0, m_10 = 0;
    uint8_t *center = img->pixels + (kp->y * img->w + kp->x);

    // Treat the center line differently, v=0
    for (int u = -half_k; u <= half_k; ++u) {
        m_10 += u * center[u];
    }

    // Go line by line in the circular patch
    for (int v = 1; v <= half_k; ++v) {
        // Proceed over the two lines
        int v_sum = 0;
        int d = u_max[v];
        for (int u = -d; u <= d; ++u) {
            int val_plus = center[u + v * step], val_minus = center[u - v * step];
            v_sum += (val_plus - val_minus);
            m_10 += u * (val_plus + val_minus);
        }
        m_01 += v * v_sum;
    }

    int angle = (int) (atan2f((float) m_01, (float) m_10) * (180.0f / M_PI));
    if (angle < 0) {
        angle += 360;
    }

    // Quantize angle to 15 degrees
    angle = angle - (angle % 15);

    *a = cos_table[angle];
    *b = sin_table[angle];
    return angle;
}

static void image_scale(image_t *src, image_t *dst) {
    int x_ratio = (int) ((src->w << 16) / dst->w) + 1;
    int y_ratio = (int) ((src->h << 16) / dst->h) + 1;

    for (int y = 0; y < dst->h; y++) {
        int sy = (y * y_ratio) >> 16;
        for (int x = 0; x < dst->w; x++) {
            int sx = (x * x_ratio) >> 16;
            dst->pixels[y * dst->w + x] = IM_TO_GS_PIXEL(src, sx, sy);
        }
    }
}

array_t *orb_find_keypoints(image_t *img, bool normalized, int threshold,
                            float scale_factor, int max_keypoints, corner_detector_t corner_detector, rectangle_t *roi) {
    array_t *kpts;
    array_alloc(&kpts, xfree);

    int octave = 1;
    int kpts_index = 0;
    rectangle_t roi_scaled;

    for (float scale = 1.0f; ; scale *= scale_factor, octave++) {
        image_t img_scaled = {
            .w = (int) roundf(img->w / scale),
            .h = (int) roundf(img->h / scale),
            .pixfmt = PIXFORMAT_GRAYSCALE,
            .pixels = NULL
        };

        // Add patch size to ROI
        roi_scaled.x = (int) roundf(roi->x / scale) + (PATCH_SIZE);
        roi_scaled.y = (int) roundf(roi->y / scale) + (PATCH_SIZE);
        roi_scaled.w = (int) roundf(roi->w / scale) - (PATCH_SIZE * 2);
        roi_scaled.h = (int) roundf(roi->h / scale) - (PATCH_SIZE * 2);

        if (roi_scaled.w <= (PATCH_SIZE * 2) ||
            roi_scaled.h <= (PATCH_SIZE * 2)) {
            break;
        }

        img_scaled.pixels = fb_alloc(img_scaled.w * img_scaled.h, 0);
        // Down scale image
        image_scale(img, &img_scaled);

        // Gaussian smooth the image before extracting keypoints
        imlib_sepconv3(&img_scaled, kernel_gauss_3, 1.0f / 16.0f, 0.0f);

        // Find kpts
        #ifdef IMLIB_ENABLE_FAST
        if (corner_detector == CORNER_FAST) {
            fast_detect(&img_scaled, kpts, threshold, &roi_scaled);
        }
        #endif
        #ifdef IMLIB_ENABLE_AGAST
        if (corner_detector == CORNER_AGAST) {
            agast_detect(&img_scaled, kpts, threshold, &roi_scaled);
        }
        #endif

        for (int k = kpts_index; k < array_length(kpts); k++, kpts_index++) {
            // Set keypoint octave/scale
            kp_t *kpt = array_at(kpts, k);
            kpt->octave = octave;

            int x, y;
            float a, b;
            sample_point_t *pattern = (sample_point_t *) sample_pattern;
            kpt->angle = comp_angle(&img_scaled, kpt, &a, &b);

            #if 1
#define GET_VALUE(idx)                                          \
    (x = (int) roundf(pattern[idx].x * a - pattern[idx].y * b), \
     y = (int) roundf(pattern[idx].x * b + pattern[idx].y * a), \
     img_scaled.pixels[((kpt->y + y) * img_scaled.w) + (kpt->x + x)])
            #else
#define GET_VALUE(idx) \
    (img_scaled.pixels[((kpt->y + pattern[idx].y) * img_scaled.w) + (kpt->x + pattern[idx].x)])
            #endif

            for (int i = 0; i < KDESC_SIZE; ++i, pattern += 16) {
                int t0, t1, t2, t3, u, v, k, val;
                t0 = GET_VALUE(0); t1 = GET_VALUE(1);
                t2 = GET_VALUE(2); t3 = GET_VALUE(3);
                u = 0, v = 2;
                if (t1 > t0) {
                    t0 = t1, u = 1;
                }
                if (t3 > t2) {
                    t2 = t3, v = 3;
                }
                k = t0 > t2 ? u : v;
                val = k;

                t0 = GET_VALUE(4); t1 = GET_VALUE(5);
                t2 = GET_VALUE(6); t3 = GET_VALUE(7);
                u = 0, v = 2;
                if (t1 > t0) {
                    t0 = t1, u = 1;
                }
                if (t3 > t2) {
                    t2 = t3, v = 3;
                }
                k = t0 > t2 ? u : v;
                val |= k << 2;

                t0 = GET_VALUE(8); t1 = GET_VALUE(9);
                t2 = GET_VALUE(10); t3 = GET_VALUE(11);
                u = 0, v = 2;
                if (t1 > t0) {
                    t0 = t1, u = 1;
                }
                if (t3 > t2) {
                    t2 = t3, v = 3;
                }
                k = t0 > t2 ? u : v;
                val |= k << 4;

                t0 = GET_VALUE(12); t1 = GET_VALUE(13);
                t2 = GET_VALUE(14); t3 = GET_VALUE(15);
                u = 0, v = 2;
                if (t1 > t0) {
                    t0 = t1, u = 1;
                }
                if (t3 > t2) {
                    t2 = t3, v = 3;
                }
                k = t0 > t2 ? u : v;
                val |= k << 6;

                kpt->desc[i] = (uint8_t) val;
            }

            kpt->x = (int) floorf(kpt->x * scale);
            kpt->y = (int) floorf(kpt->y * scale);
        }

        // Free current scale
        fb_free();

        if (normalized) {
            break;
        }
    }

    // Sort keypoints by score and return top n keypoints
    array_sort(kpts, (array_comp_t) kpt_comp);
    if (array_length(kpts) > max_keypoints) {
        array_resize(kpts, max_keypoints);
    }

    return kpts;
}

// This is a modified popcount that counts every 2 different bits as 1.
// This is what should actually be used with wta_k == 3 or 4.
static inline uint32_t popcount(uint32_t i) {
    i = i - ((i >> 1) & 0x55555555);
    i = ((i & 0xAAAAAAAA) >> 1) | (i & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static kp_t *find_best_match(kp_t *kp1, array_t *kpts, int *dist_out1, int *dist_out2, int *index) {
    kp_t *min_kp = NULL;
    int min_dist1 = MAX_KP_DIST;
    int min_dist2 = MAX_KP_DIST;
    int kpts_size = array_length(kpts);

    for (int i = 0; i < kpts_size; i++) {
        int dist = 0;
        kp_t *kp2 = array_at(kpts, i);

        if (kp2->matched == 0) {
            for (int m = 0; m < (KDESC_SIZE / 4); m++) {
                dist += popcount(((uint32_t *) (kp1->desc))[m] ^ ((uint32_t *) (kp2->desc))[m]);
            }

            if (dist < min_dist1) {
                *index = i;
                min_kp = kp2;
                min_dist2 = min_dist1;
                min_dist1 = dist;
            }
        }
    }

    *dist_out1 = min_dist1;
    *dist_out2 = min_dist2;
    return min_kp;
}

int orb_match_keypoints(array_t *kpts1, array_t *kpts2, int *match, int threshold, rectangle_t *r, point_t *c, int *angle) {
    int matches = 0;
    int cx = 0, cy = 0;
    uint16_t angles[360] = {0};
    int kpts1_size = array_length(kpts1);

    r->w = r->h = 0;
    r->x = r->y = 20000;

    // Match keypoints and find "good matches" This runs 2/3 tests found in the RobustMatcher from the OpenCV programming cookbook.
    // The first test is based on the distance ratio between the two best matches for a feature, to remove ambiguous matches.
    // Second test is the symmetry test (corss-matching) both points in a match must be the best matching feature of each other.
    for (int i = 0; i < kpts1_size; i++) {
        int kp_index1 = 0;
        int kp_index2 = 0;
        int min_dist1 = 0;
        int min_dist2 = 0;
        kp_t *min_kp = NULL;
        kp_t *kp1 = array_at(kpts1, i);

        // Find the best match in second set
        min_kp = find_best_match(kp1, kpts2, &min_dist1, &min_dist2, &kp_index2);
        // Test the distance ratio between the best two matches
        if ((min_dist1 * 100 / min_dist2) > threshold) {
            continue;
        }

        // Cross-match the keypoint in the first set
        kp_t *kp2 = find_best_match(min_kp, kpts1, &min_dist1, &min_dist2, &kp_index1);
        // Test the distance ratio between the best two matches
        if ((min_dist1 * 100 / min_dist2) > threshold) {
            continue;
        }

        // Cross-match test
        if (kp1 == kp2) {
            int x, y;
            matches++;
            min_kp->matched = 1;
            cx += x = min_kp->x;
            cy += y = min_kp->y;
            rectangle_expand(r, x, y);
            int angle = (int) abs(min_kp->angle - kp1->angle);
            if (angle >= 0 && angle < 360) {
                angles[angle]++;
            }
            *match++ = kp_index1;
            *match++ = kp_index2;
        }
    }

    if (matches == 0) {
        r->x = r->y = 0;
        return 0;
    }

    // Fix centroid x/y
    c->x = cx / matches;
    c->y = cy / matches;

    // Fix rectangle w/h
    r->w = r->w - r->x;
    r->h = r->h - r->y;

    int max_angle = 0;
    for (int i = 0; i < 360; i++) {
        if (angles[i] > max_angle) {
            max_angle = angles[i];
            *angle = i;
        }
    }

    return matches;
}

int orb_filter_keypoints(array_t *kpts, rectangle_t *r, point_t *c) {
    int matches = 0;
    int cx = 0, cy = 0;
    int kpts_size    = array_length(kpts);

    r->w = r->h = 0;
    r->x = r->y = 20000;

    float *kpts_dist = fb_alloc(kpts_size * sizeof(float), 0);

    // Find centroid
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);
        if (kp->matched) {
            matches++;
            cx += kp->x;
            cy += kp->y;
        }
    }

    // Centroid
    cx /= matches;
    cy /= matches;

    // Find mean distance from centroid
    float mdist = 0.0f;
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);
        if (kp->matched) {
            kpts_dist[i] = orb_cluster_dist(cx, cy, kp);
            mdist += kpts_dist[i];
        }
    }
    // Mean distance from centroid
    mdist /= matches;

    // Find variance
    float var = 0.0f;
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);
        if (kp->matched) {
            float dist = kpts_dist[i];
            var += (mdist - dist) * (mdist - dist);
        }
    }

    // Find standard deviation
    float stdist = fast_sqrtf(var / matches);

    // Reset centroid
    matches = 0;
    cx = cy = 0;

    // Remove outliers and get new centroid
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);
        if (kp->matched) {
            float dist = fabs(mdist - kpts_dist[i]);
            if (dist > stdist) {
                kp->matched = 0;
            } else {
                int x, y;
                matches++;
                cx += x = kp->x;
                cy += y = kp->y;
                rectangle_expand(r, x, y);
            }
        }
    }

    // Fix centroid x/y
    c->x = cx / matches;
    c->y = cy / matches;

    // Fix rectangle w/h
    r->w = r->w - r->x;
    r->h = r->h - r->y;

    // Free distance array
    fb_free();
    return matches;
}

#if defined(IMLIB_ENABLE_IMAGE_FILE_IO)
int orb_save_descriptor(FIL *fp, array_t *kpts) {
    UINT bytes;
    FRESULT res;

    int kpts_size = array_length(kpts);

    // Write the number of keypoints
    res = file_ll_write(fp, &kpts_size, sizeof(kpts_size), &bytes);
    if (res != FR_OK || bytes != sizeof(kpts_size)) {
        goto error;
    }

    // Write keypoints
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = array_at(kpts, i);

        // Write X
        res = file_ll_write(fp, &kp->x, sizeof(kp->x), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->x)) {
            goto error;
        }

        // Write Y
        res = file_ll_write(fp, &kp->y, sizeof(kp->y), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->y)) {
            goto error;
        }

        // Write Score
        res = file_ll_write(fp, &kp->score, sizeof(kp->score), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->score)) {
            goto error;
        }

        // Write Octave
        res = file_ll_write(fp, &kp->octave, sizeof(kp->octave), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->octave)) {
            goto error;
        }

        // Write Angle
        res = file_ll_write(fp, &kp->angle, sizeof(kp->angle), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->angle)) {
            goto error;
        }

        // Write descriptor
        res = file_ll_write(fp, kp->desc, KDESC_SIZE, &bytes);
        if (res != FR_OK || bytes != KDESC_SIZE) {
            goto error;
        }
    }

error:
    return res;
}

int orb_load_descriptor(FIL *fp, array_t *kpts) {
    UINT bytes;
    FRESULT res = FR_OK;

    int kpts_size = 0;

    // Read number of keypoints
    res = file_ll_read(fp, &kpts_size, sizeof(kpts_size), &bytes);
    if (res != FR_OK || bytes != sizeof(kpts_size)) {
        goto error;
    }

    // Read keypoints
    for (int i = 0; i < kpts_size; i++) {
        kp_t *kp = xalloc(sizeof(*kp));
        kp->matched = 0;

        // Read X
        res = file_ll_read(fp, &kp->x, sizeof(kp->x), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->x)) {
            goto error;
        }

        // Read Y
        res = file_ll_read(fp, &kp->y, sizeof(kp->y), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->y)) {
            goto error;
        }

        // Read Score
        res = file_ll_read(fp, &kp->score, sizeof(kp->score), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->score)) {
            goto error;
        }

        // Read Octave
        res = file_ll_read(fp, &kp->octave, sizeof(kp->octave), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->octave)) {
            goto error;
        }

        // Read Angle
        res = file_ll_read(fp, &kp->angle, sizeof(kp->angle), &bytes);
        if (res != FR_OK || bytes != sizeof(kp->angle)) {
            goto error;
        }

        // Read descriptor
        res = file_ll_read(fp, kp->desc,  KDESC_SIZE, &bytes);
        if (res != FR_OK || bytes != KDESC_SIZE) {
            goto error;
        }

        // Add keypoint to array
        array_push_back(kpts, kp);
    }

error:
    return res;
}
#endif  //IMLIB_ENABLE_IMAGE_FILE_IO

float orb_cluster_dist(int cx, int cy, void *kp_in) {
    float sum = 0.0f;
    kp_t *kp = kp_in;
    sum += (cx - kp->x) * (cx - kp->x);
    sum += (cy - kp->y) * (cy - kp->y);
    return fast_sqrtf(sum);

}
#endif // IMLIB_ENABLE_FIND_KEYPOINTS

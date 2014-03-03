/***********************************************************
*  --- OpenSURF ---                                       *
*  This library is distributed under the GNU GPL. Please   *
*  use the contact form at http://www.chrisevansdev.com    *
*  for more information.                                   *
*                                                          *
*  C. Evans, Research Into Robust Visual Features,         *
*  MSc University of Bristol, 2008.                        *
*                                                          *
************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include "array.h"
#include "xalloc.h"
#include "imlib.h"
#include "arm_math.h"
#define OCTAVES     5
#define INTERVALS   4
#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

static const float pi = 3.14159f;

//! lookup table for 2d gaussian (sigma = 2.5) where (0,0) is top left and (6,6) is bottom right
static const float gauss25 [7][7] = {
    {0.02546481f, 0.02350698f, 0.01849125f, 0.01239505f, 0.00708017, 0.00344629f, 0.00142946f},
    {0.02350698f, 0.02169968f, 0.01706957f, 0.01144208f, 0.00653582, 0.00318132f, 0.00131956f},
    {0.01849125f, 0.01706957f, 0.01342740f, 0.00900066f, 0.00514126, 0.00250252f, 0.00103800f},
    {0.01239505f, 0.01144208f, 0.00900066f, 0.00603332f, 0.00344629, 0.00167749f, 0.00069579f},
    {0.00708017f, 0.00653582f, 0.00514126f, 0.00344629f, 0.00196855, 0.00095820f, 0.00039744f},
    {0.00344629f, 0.00318132f, 0.00250252f, 0.00167749f, 0.00095820, 0.00046640f, 0.00019346f},
    {0.00142946f, 0.00131956f, 0.00103800f, 0.00069579f, 0.00039744, 0.00019346f, 0.00008024f},
};

void arm_mat_set(const arm_matrix_instance_f32 * pSrc, int r, int c, float32_t v)
{
    pSrc->pData[r*pSrc->numCols+c] = v;
}

arm_matrix_instance_f32 *arm_mat_new(int r, int c)
{
    arm_matrix_instance_f32 *m=(arm_matrix_instance_f32 *)xalloc(sizeof(*m));
    arm_mat_init_f32(m, r, c, (float32_t*)xalloc(r*c*sizeof(float32_t)));
    return m;
}

void arm_mat_free(arm_matrix_instance_f32 *m)
{
    xfree(m->pData);
    xfree(m);
}

/* Computes the sum of pixels within the rectangle
   specified by the top-left start co-ordinate and size. */
static float box_integral(i_image_t *img, int row, int col, int rows, int cols)
{
  int step = img->w;
  uint32_t *data = img->data;

  // The subtraction by one for row/col is because row/col is inclusive.
  int r1 = MIN(row,          img->h) - 1;
  int c1 = MIN(col,          img->w) - 1;
  int r2 = MIN(row + rows,   img->h) - 1;
  int c2 = MIN(col + cols,   img->w) - 1;

  float A=0.0f;
  float B=0.0f;
  float C=0.0f;
  float D=0.0f;
  if (r1 >= 0 && c1 >= 0) A = data[r1 * step + c1]/255.0f;
  if (r1 >= 0 && c2 >= 0) B = data[r1 * step + c2]/255.0f;
  if (r2 >= 0 && c1 >= 0) C = data[r2 * step + c1]/255.0f;
  if (r2 >= 0 && c2 >= 0) D = data[r2 * step + c2]/255.0f;

  return MAX(0, (A - B - C + D));
}

//! Calculate the value of the 2d gaussian at x,y
static inline float gaussian(int x, int y, float sig)
{
  return (1.0f/(2.0f*pi*sig*sig)) * fast_expf(-(x*x+y*y)/(2.0f*sig*sig));
}

//! Calculate the value of the 2d gaussian at x,y
static inline float gaussianf(float x, float y, float sig)
{
  return 1.0f/(2.0f*pi*sig*sig) * fast_expf(-(x*x+y*y)/(2.0f*sig*sig));
}

//! Calculate Haar wavelet responses in x direction
static inline float haar_x(surf_t *surf, int row, int column, int s)
{
  return box_integral(surf->i_img, row-s/2, column, s, s/2)
    -1 * box_integral(surf->i_img, row-s/2, column-s/2, s, s/2);
}

//! Calculate Haar wavelet responses in y direction
static inline float haar_y(surf_t *surf, int row, int column, int s)
{
  return box_integral(surf->i_img, row, column-s/2, s/2, s)
    -1 * box_integral(surf->i_img, row-s/2, column-s/2, s/2, s);
}

//! Get the angle from the +ve x-axis of the vector given by (X Y)
static float get_angle(float x, float y)
{
  if(x > 0 && y >= 0)
    return fast_atanf(y/x);

  if(x < 0 && y >= 0)
    return pi - fast_atanf(-y/x);

  if(x < 0 && y < 0)
    return pi + fast_atanf(y/x);

  if(x > 0 && y < 0)
    return 2*pi - fast_atanf(-y/x);

  return 0;
}

//! Assign the supplied Ipoint an orientation
static void get_orientation(surf_t *surf, i_point_t *ipt)
{
  float gauss = 0.f, scale = ipt->scale;
  const int s = fast_roundf(scale);
  const int r = fast_roundf(ipt->y);
  const int c = fast_roundf(ipt->x);
  float resX[109];
  float resY[109];
  float Ang[109];
  const int id[] = {6,5,4,3,2,1,0,1,2,3,4,5,6};

  int idx = 0;
  int rad = 6;
  // calculate haar responses for points within radius of 6*scale
  for(int i = -rad; i <= rad; ++i) {
    for(int j = -rad; j <= rad; ++j) {
      if(i*i + j*j < rad*rad) {
        gauss = (float)(gauss25[id[i+rad]][id[j+rad]]);  // could use abs() rather than id lookup, but this way is faster
        resX[idx] = gauss * haar_x(surf, r+j*s, c+i*s, 4*s);
        resY[idx] = gauss * haar_y(surf, r+j*s, c+i*s, 4*s);
        Ang[idx] = get_angle(resX[idx], resY[idx]);
        ++idx;
      }
    }
  }

  // calculate the dominant direction
  float sumX=0.f, sumY=0.f;
  float max=0.f, orientation = 0.f;
  float ang1=0.f, ang2=0.f;

  // loop slides pi/3 window around feature point
  for(ang1 = 0; ang1 < 2*pi;  ang1+=0.15f) {
    ang2 = ( ang1+pi/3.0f > 2*pi ? ang1-5.0f*pi/3.0f : ang1+pi/3.0f);
    sumX = sumY = 0.f;
    for(unsigned int k = 0; k < idx; ++k) {
      // get angle from the x-axis of the sample point
      float ang = Ang[k];

      // determine whether the point is within the window
      if (ang1 < ang2 && ang1 < ang && ang < ang2) {
        sumX+=resX[k];
        sumY+=resY[k];
      } else if (ang2 < ang1 && ((ang > 0 && ang < ang2) || (ang > ang1 && ang < 2*pi) )) {
        sumX+=resX[k];
        sumY+=resY[k];
      }
    }

    // if the vector produced from this window is longer than all
    // previous vectors then this forms the new dominant direction
    if (sumX*sumX + sumY*sumY > max) {
      // store largest orientation
      max = sumX*sumX + sumY*sumY;
      orientation = get_angle(sumX, sumY);
    }
  }

  // assign orientation of the dominant response vector
  ipt->orientation = orientation;
}

//-------------------------------------------------------

//! Get the modified descriptor. See Agrawal ECCV 08
//! Modified descriptor contributed by Pablo Fernandez
static void get_descriptor(surf_t *surf, i_point_t *ipt, bool bUpright)
{
  int y, x, sample_x, sample_y, count=0;
  int i = 0, ix = 0, j = 0, jx = 0, xs = 0, ys = 0;
  float scale, *desc, dx, dy, mdx, mdy, co, si;
  float gauss_s1 = 0.f, gauss_s2 = 0.f;
  float rx = 0.f, ry = 0.f, rrx = 0.f, rry = 0.f, len = 0.f;
  float cx = -0.5f, cy = 0.f; //Subregion centers for the 4x4 gaussian weighting

  scale = ipt->scale;
  x = fast_roundf(ipt->x);
  y = fast_roundf(ipt->y);
  desc = ipt->descriptor;

  if (bUpright) {
    co = 1.0f;
    si = 0.0f;
  } else {
    co = arm_cos_f32(ipt->orientation);
    si = arm_sin_f32(ipt->orientation);
  }

  i = -8;

  //Calculate descriptor for this interest point
  while (i < 12) {
    j = -8;
    i = i-4;

    cx += 1.f;
    cy = -0.5f;

    while (j < 12) {
      dx=dy=mdx=mdy=0.f;
      cy += 1.f;

      j = j - 4;

      ix = i + 5;
      jx = j + 5;

      xs = fast_roundf(x + ( -jx*scale*si + ix*scale*co));
      ys = fast_roundf(y + ( jx*scale*co + ix*scale*si));

      for (int k = i; k < i + 9; ++k) {
        for (int l = j; l < j + 9; ++l) {
          //Get coords of sample point on the rotated axis
          sample_x = fast_roundf(x + (-l*scale*si + k*scale*co));
          sample_y = fast_roundf(y + ( l*scale*co + k*scale*si));

          //Get the gaussian weighted x and y responses
          gauss_s1 = gaussian(xs-sample_x,ys-sample_y,2.5f*scale);
          rx = haar_x(surf, sample_y, sample_x, 2*fast_roundf(scale));
          ry = haar_y(surf, sample_y, sample_x, 2*fast_roundf(scale));

          //Get the gaussian weighted x and y responses on rotated axis
          rrx = gauss_s1*(-rx*si + ry*co);
          rry = gauss_s1*(rx*co + ry*si);

          dx += rrx;
          dy += rry;
          mdx += fast_fabsf(rrx);
          mdy += fast_fabsf(rry);

        }
      }

      //Add the values to the descriptor vector
      gauss_s2 = gaussian(cx-2.0f,cy-2.0f,1.5f);

      desc[count++] = dx*gauss_s2;
      desc[count++] = dy*gauss_s2;
      desc[count++] = mdx*gauss_s2;
      desc[count++] = mdy*gauss_s2;

      len += (dx*dx + dy*dy + mdx*mdx + mdy*mdy) * gauss_s2*gauss_s2;

      j += 9;

      if (count == SURF_DESC_SIZE) {
          goto done;
      }

    }
    i += 9;
  }

done:
  //Convert to Unit Vector
  len = fast_sqrtf(len);
  for(int i = 0; i <SURF_DESC_SIZE; ++i)
    desc[i] /= len;
}

static response_layer_t *response_layer_new(int width, int height, int step, int filter)
{
    response_layer_t *layer = NULL;

    layer = xalloc(sizeof(*layer));
    layer->width = width;
    layer->height = height;
    layer->step = step;
    layer->filter = filter;
    return layer;
}

float surf_get_laplacian(surf_t *surf, response_layer_t *rl, unsigned int r, unsigned int c)
{
  int step = rl->step;                      // step size for this filter
  int b = (rl->filter - 1) / 2;             // border for this filter
  int l = rl->filter / 3;                   // lobe for this filter (filter size / 3)
  int w = rl->filter;                       // filter size
  float inverse_area = 1.f/(w*w);           // normalisation factor
  float Dxx, Dyy, Dxy;

  r *= step;
  c *= step;

  // Compute response components
  Dxx = box_integral(surf->i_img, r - l + 1, c - b, 2*l - 1, w)
      - box_integral(surf->i_img, r - l + 1, c - l / 2, 2*l - 1, l)*3;
  Dyy = box_integral(surf->i_img, r - b, c - l + 1, w, 2*l - 1)
      - box_integral(surf->i_img, r - l / 2, c - l + 1, l, 2*l - 1)*3;
  Dxy = + box_integral(surf->i_img, r - l, c + 1, l, l)
        + box_integral(surf->i_img, r + 1, c - l, l, l)
        - box_integral(surf->i_img, r - l, c - l, l, l)
        - box_integral(surf->i_img, r + 1, c + 1, l, l);

  // Normalise the filter responses with respect to their size
  Dxx *= inverse_area;
  Dyy *= inverse_area;
  Dxy *= inverse_area;

  // Get the determinant of hessian response & laplacian sign
  return (Dxx + Dyy >= 0 ? 1 : 0);
}

//! Calculate DoH responses for supplied layer
float surf_get_response(surf_t *surf, response_layer_t *rl, response_layer_t *src, unsigned int r, unsigned int c)
{
  int step = rl->step;                      // step size for this filter
  int b = (rl->filter - 1) / 2;             // border for this filter
  int l = rl->filter / 3;                   // lobe for this filter (filter size / 3)
  int w = rl->filter;                       // filter size
  float inverse_area = 1.f/(w*w);           // normalisation factor
  float Dxx, Dyy, Dxy;

  int scale = rl->width / src->width;

  r *= step*scale;
  c *= step*scale;

  // Compute response components
  Dxx = box_integral(surf->i_img, r - l + 1, c - b, 2*l - 1, w)
      - box_integral(surf->i_img, r - l + 1, c - l / 2, 2*l - 1, l)*3;
  Dyy = box_integral(surf->i_img, r - b, c - l + 1, w, 2*l - 1)
      - box_integral(surf->i_img, r - l / 2, c - l + 1, l, 2*l - 1)*3;
  Dxy = + box_integral(surf->i_img, r - l, c + 1, l, l)
        + box_integral(surf->i_img, r + 1, c - l, l, l)
        - box_integral(surf->i_img, r - l, c - l, l, l)
        - box_integral(surf->i_img, r + 1, c + 1, l, l);

  // Normalise the filter responses with respect to their size
  Dxx *= inverse_area;
  Dyy *= inverse_area;
  Dxy *= inverse_area;

  // Get the determinant of hessian response & laplacian sign
  return (Dxx * Dyy - 0.81f * Dxy * Dxy);
}

//! Computes the partial derivatives in x, y, and scale of a pixel.
static void surf_deriv3D(surf_t *surf, arm_matrix_instance_f32 *m1, int r, int c, response_layer_t *t, response_layer_t *m, response_layer_t *b)
{
  float32_t dx, dy, ds;

  dx = (surf_get_response(surf, m, t, r, c + 1) - surf_get_response(surf, m, t, r, c - 1)) / 2.0;
  dy = (surf_get_response(surf, m, t, r + 1, c) - surf_get_response(surf, m, t, r - 1, c)) / 2.0;
  ds = (surf_get_response(surf, t, t, r, c) - surf_get_response(surf, b, t, r, c)) / 2.0;

  arm_mat_set(m1, 0, 0, dx );
  arm_mat_set(m1, 1, 0, dy );
  arm_mat_set(m1, 2, 0, ds );
}

//! Computes the 3D Hessian matrix for a pixel.
void surf_hessian3D(surf_t *surf, arm_matrix_instance_f32 *m1, int r, int c, response_layer_t *t, response_layer_t *m, response_layer_t *b)
{
  float32_t v, dxx, dyy, dss, dxy, dxs, dys;

  v = surf_get_response(surf, m,t, r, c);
  dxx = surf_get_response(surf, m,t, r, c + 1) + surf_get_response(surf, m, t,r, c - 1) - 2 * v;
  dyy = surf_get_response(surf, m,t,r + 1, c) + surf_get_response(surf, m, t, r - 1, c) - 2 * v;
  dss = surf_get_response(surf, t, t, r, c) + surf_get_response(surf, b, t, r, c) - 2 * v;
  dxy = ( surf_get_response(surf, m, t, r + 1, c + 1) - surf_get_response(surf, m, t, r + 1, c - 1) -
          surf_get_response(surf, m, t, r - 1, c + 1) + surf_get_response(surf, m, t, r - 1, c - 1) ) / 4.0;
  dxs = ( surf_get_response(surf, t, t, r, c + 1) - surf_get_response(surf, t, t, r, c - 1) -
          surf_get_response(surf, b, t, r, c + 1) + surf_get_response(surf, b, t, r, c - 1) ) / 4.0;
  dys = ( surf_get_response(surf, t, t, r + 1, c) - surf_get_response(surf, t, t, r - 1, c) -
          surf_get_response(surf, b, t, r + 1, c) + surf_get_response(surf, b, t, r - 1, c) ) / 4.0;

  arm_mat_set(m1, 0, 0, dxx);
  arm_mat_set(m1, 0, 1, dxy);
  arm_mat_set(m1, 0, 2, dxs);
  arm_mat_set(m1, 1, 0, dxy);
  arm_mat_set(m1, 1, 1, dyy);
  arm_mat_set(m1, 1, 2, dys);
  arm_mat_set(m1, 2, 0, dxs);
  arm_mat_set(m1, 2, 1, dys);
  arm_mat_set(m1, 2, 2, dss);
}

//! Performs one step of extremum interpolation.
static void surf_interpolate_step(surf_t *surf, int r, int c, response_layer_t *t, response_layer_t *m, response_layer_t *b, float32_t* xi, float32_t* xr, float32_t* xc )
{
  arm_matrix_instance_f32 *H=arm_mat_new(3,3);
  arm_matrix_instance_f32 *H_t=arm_mat_new(3,3);
  arm_matrix_instance_f32 *H_inv=arm_mat_new(3,3);
  arm_matrix_instance_f32 *dD=arm_mat_new(3, 1);
  arm_matrix_instance_f32 *X=arm_mat_new(3, 1);

  surf_hessian3D(surf, H, r, c, t, m, b);
  surf_deriv3D(surf, dD, r, c, t, m, b);

  arm_mat_inverse_f32(H, H_inv);
  arm_mat_trans_f32(H_inv, H_t);
  arm_mat_scale_f32(H_t, -1, H);
  arm_mat_mult_f32(H, dD, X);

  *xi = X->pData[2];
  *xr = X->pData[1];
  *xc = X->pData[0];

  arm_mat_free(H);
  arm_mat_free(H_t);
  arm_mat_free(H_inv);
  arm_mat_free(dD);
  arm_mat_free(X);
}

//! Interpolate scale-space extrema to subpixel accuracy to form an image feature.
static void interpolate_extremum(surf_t *surf, int r, int c, response_layer_t *t, response_layer_t *m, response_layer_t *b)
{
  // get the step distance between filters
  // check the middle filter is mid way between top and bottom
  int filterStep = (m->filter - b->filter);
  //assert(filterStep > 0 && t->filter - m->filter == m->filter - b->filter);

  // Get the offsets to the actual location of the extremum
  float32_t xi = 0, xr = 0, xc = 0;
  surf_interpolate_step(surf, r, c, t, m, b, &xi, &xr, &xc );

  // If point is sufficiently close to the actual extremum
  if(fast_fabsf( xi ) < 0.5f  &&  fast_fabsf( xr ) < 0.5f  &&  fast_fabsf( xc ) < 0.5f ) {
    i_point_t *ipt = xalloc(sizeof(*ipt));
    ipt->x = (float)((c + xc) * t->step);
    ipt->y = (float)((r + xr) * t->step);
    ipt->scale = (float)((0.1333f) * (m->filter + xi * filterStep));
    ipt->laplacian = (int)(surf_get_laplacian(surf, m, r, c));
    array_push_back(surf->ipts, ipt);
  }
}

//! Non Maximal Suppression function
static int is_extremum(surf_t *surf, int r, int c, response_layer_t *t, response_layer_t *m, response_layer_t *b)
{
  // bounds check
  int layerBorder = (t->filter + 1) / (2 * t->step);
  if (r <= layerBorder || r >= t->height - layerBorder || c <= layerBorder || c >= t->width - layerBorder)
    return 0;

  // check the candidate point in the middle layer is above thresh
  float candidate = surf_get_response(surf, m, t, r, c);
  if (candidate < surf->thresh)
    return 0;

  for (int rr = -1; rr <=1; ++rr) {
    for (int cc = -1; cc <=1; ++cc) {
      // if any response in 3x3x3 is greater candidate not maximum
      if (surf_get_response(surf, t, t, r+rr, c+cc) >= candidate ||
         ((rr != 0 || cc != 0) && surf_get_response(surf, m, t, r+rr, c+cc) >= candidate) ||
            surf_get_response(surf, b, t, r+rr, c+cc) >= candidate)
        return 0;
    }
  }

  return 1;
}

//! Build map of DoH responses
static void build_response_map(surf_t *surf)
{
  // Calculate responses for the first 4 octaves:
  // Oct1: 9,  15, 21, 27
  // Oct2: 15, 27, 39, 51
  // Oct3: 27, 51, 75, 99
  // Oct4: 51, 99, 147,195
  // Oct5: 99, 195,291,387

  // Get image attributes
  int w = (surf->i_img->w / surf->init_sample);
  int h = (surf->i_img->h / surf->init_sample);
  int s = (surf->init_sample);

  // Calculate approximated determinant of hessian values
  if (surf->octaves >= 1) {
    array_push_back(surf->rmap, response_layer_new(w,   h,   s,   9));
    array_push_back(surf->rmap, response_layer_new(w,   h,   s,   15));
    array_push_back(surf->rmap, response_layer_new(w,   h,   s,   21));
    array_push_back(surf->rmap, response_layer_new(w,   h,   s,   27));
  }

  if (surf->octaves >= 2) {
    array_push_back(surf->rmap, response_layer_new(w/2, h/2, s*2, 39));
    array_push_back(surf->rmap, response_layer_new(w/2, h/2, s*2, 51));
  }

  if (surf->octaves >= 3) {
    array_push_back(surf->rmap, response_layer_new(w/4, h/4, s*4, 75));
    array_push_back(surf->rmap, response_layer_new(w/4, h/4, s*4, 99));
  }

  if (surf->octaves >= 4) {
    array_push_back(surf->rmap, response_layer_new(w/8, h/8, s*8, 147));
    array_push_back(surf->rmap, response_layer_new(w/8, h/8, s*8, 195));
  }

  if (surf->octaves >= 5) {
    array_push_back(surf->rmap,response_layer_new(w/16, h/16, s*16, 291));
    array_push_back(surf->rmap,response_layer_new(w/16, h/16, s*16, 387));
  }

}

//! Find the image features and write into vector of features
static void get_ipoints(surf_t *surf)
{
    // filter index map
    static const int filter_map[OCTAVES][INTERVALS] = {
        {0,1,2,3},
        {1,3,4,5},
        {3,5,6,7},
        {5,7,8,9},
        {7,9,10,11}
    };

    // Build the response map
    build_response_map(surf);

    // Get the response layers
    response_layer_t *b, *m, *t;
    for (int o=0; o<surf->octaves; ++o) {
        for (int i=0; i<=1; ++i) {
            b = array_at(surf->rmap, filter_map[o][i]);
            m = array_at(surf->rmap, filter_map[o][i+1]);
            t = array_at(surf->rmap, filter_map[o][i+2]);

            // loop over middle response layer at density of the most
            // sparse layer (always top), to find maxima across scale and space
            for (int r=0; r<t->height; ++r) {
                for (int c=0; c<t->width; ++c) {
                    if (is_extremum(surf, r, c, t, m, b)) {
                        interpolate_extremum(surf, r, c, t, m, b);
                    }
                }
            }
        }
    }
}

float i_point_sub(i_point_t *lhs, i_point_t *rhs) {
    float sum=0.0f;
    for (int i=0; i<SURF_DESC_SIZE; ++i) {
        sum += (lhs->descriptor[i] - rhs->descriptor[i])*(lhs->descriptor[i] - rhs->descriptor[i]);
    }
    return fast_sqrtf(sum);
};

static array_t *get_matches(array_t *ipts1, array_t *ipts2)
{
  float d1;
  float d2;
  float dist;

  i_point_t *match;
  array_t *matches;

  /* Allocate interest points array */
  array_alloc(&matches, NULL); /* elements won't be free'd */

  for (int i=0; i< array_length(ipts1); i++) {
    d1 = d2 = FLT_MAX;
    i_point_t *pt1 = (i_point_t *) array_at(ipts1, i);
    for (int j=0; j<array_length(ipts2); j++) {
      i_point_t *pt2 = (i_point_t *) array_at(ipts2, j);
      dist = i_point_sub(pt1, pt2);

      if(dist<d1) { /* if this feature matches better than current best */
        d2 = d1;
        d1 = dist;
        match = pt2;
      } else if(dist<d2) { /* this feature matches better than second best */
        d2 = dist;
      }
    }

    // If match has a d1:d2 ratio < 0.65 ipoints are a match
    if(d1/d2 < 0.65) {
      // Store the change in position
      pt1->dx = match->x - pt1->x;
      pt1->dy = match->y - pt1->y;
      array_push_back(matches, match);
    }
  }
  return matches;
}

void surf_detector(surf_t *surf)
{
    // Extract interest points and store in vector ipts
    get_ipoints(surf);

    // Get the size of the vector for fixed loop bounds
    int ipts_size = array_length(surf->ipts);

//    printf("points %d\n", ipts_size);
    // Extract the descriptors for the ipts
    if (surf->upright) {
        // U-SURF loop just gets descriptors
        for (int i=0; i<ipts_size; ++i) {
            // Extract upright (i.e. not rotation invariant) descriptors
            get_descriptor(surf, array_at(surf->ipts, i), true);
        }
    } else {
        // Main SURF-64 loop assigns orientations and gets descriptors
        for (int i = 0; i < ipts_size; ++i) {
            // Assign Orientations and extract rotation invariant descriptors
            get_orientation(surf, array_at(surf->ipts, i));
            get_descriptor(surf, array_at(surf->ipts, i), false);
        }
    }

}

void test_surf(image_t *img)
{
    surf_t surf = {
        .upright=true,
        .octaves=5,
        .intervals=4,
        .init_sample=2,
//        .thresh=0.0004f,
        .thresh=0.004f,

    };

    /* Allocate interest points array */
    array_alloc(&surf.ipts, xfree);

    /* Allocate response map array */
    array_alloc(&surf.rmap, xfree);

    // Create integral-image representation of the image
    i_image_t *i_img = surf.i_img = xalloc(sizeof(*surf.i_img));
    i_img->w = img->w;
    i_img->h = img->h;
    i_img->data= (uint32_t*) (img->data+(img->w * img->h)*2);


    imlib_integral_image(img, surf.i_img);
    surf_detector(&surf);

    for (int i=0; i<array_length(surf.ipts); i++) {
        i_point_t *pt = array_at(surf.ipts, i);
        int w = 4*(int)pt->scale;
        rectangle_t r ={pt->x-w/2, pt->y-w/2, w, w};
        imlib_draw_rectangle(img, &r);
    }
    array_free(surf.ipts);
    xfree(surf.i_img);
}

#if 0
void test_surf_match(image_t *t, image_t *f)
{
    i_image_t *i_img;

    surf_t surf = {
        .upright=true, /* run in rotation invariant mode */
        .octaves=1,    /* number of octaves */
        .intervals=4,
        .init_sample=2,
        .thresh=0.0004f,
    };

    /* Allocate interest points array */
    array_alloc(&surf.ipts, xfree);

    /* Allocate response map array */
    array_alloc(&surf.rmap, xfree);

    /* compute integral from template */
    i_img = surf.i_img = xalloc(sizeof(*surf.i_img));
    i_img->w = t->w;
    i_img->h = t->h;
    i_img->data=xalloc(sizeof(*i_img->data)*i_img->w*i_img->h);
    imlib_integral_image(t, surf.i_img);

    /* run SURF detector */
    surf_detector(&surf);

    /* free some stuff */
    free(surf.i_img);
    array_free(surf.rmap);
//    array_free(surf.ipts);
    /* keep ipts */
    array_t *ipts1=surf.ipts;

    /* Allocate second interest points array */
    array_alloc(&surf.ipts, xfree);

    /* Allocate second response map array */
    array_alloc(&surf.rmap, xfree);

    /* compute integral from image */
    i_img = surf.i_img = xalloc(sizeof(*surf.i_img));
    i_img->w = f->w;
    i_img->h = f->h;
    i_img->data=xalloc(sizeof(*i_img->data)*i_img->w*i_img->h);
    imlib_integral_image(f, surf.i_img);

    /* run SURF detector */
    surf_detector(&surf);

    /* get second ipts array */
    array_t *ipts2=surf.ipts;

    /* match ipts  */
    array_t *match = get_matches(ipts1, ipts2);

    printf ("t ipts: %d\n", array_length(ipts1));
    printf ("f ipts: %d\n", array_length(ipts2));
    printf ("matches: %d\n", array_length(match));
    for (int i=0; i<array_length(match); i++) {
        i_point_t *pt = array_at(match, i);
        int w = 6*(int)pt->scale;
        imlib_draw_rectangle(pt->x-w/2, pt->y-w/2, w , f->w, 0x00, f->data);
    }
    imlib_write_image(f->data, f->w, f->h, "test.tga");
}
#endif

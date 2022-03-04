//=======================================================================
/*
 *               Copyright (C) 2021 Thomas Michels
 *                  
 *                  GNU GENERAL PUBLIC LICENSE
 *                  Version 3, 29 June 2007
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//=======================================================================
#pragma once
// Color calculations:
// https : //www.niwa.nu/2013/05/math-behind-colorspace-conversions-rgb-hsl/

#include "plugin.hpp"
#include <cmath>

#ifdef ARCH_WIN
using uint = unsigned int;
#endif

namespace thm
{
  //Rect object used by RGBData
  struct Rect 
  {
    float x{0}, y{0}, w{50}, h{25};
    float imagewidth;
    void zoom(float zx, float zy){
      x /= zx;
      y /= zy;
      w /= zx;
      h /= zy;
    }
  };

  struct RGB
  { // red green and blue
    uint8_t r, g, b;
  };
  
  //Encapsulate working with the rgb-data of an Image
  struct RGBData
  {
    RGB color{};
    Rect selectBox{};
    void clear()
    {
      vrgb.clear();
      vrgb.reserve(0);
      yDelta = 0;
    }
    void addColor()
    {
      vrgb.emplace_back(color);
      //vrgb.push_back(color);
    }
    void resetPosition()
    {
      imgWidth = std::round(r.imagewidth);
      rx = std::round(r.x);
      ry = std::round(r.y);
      rw = std::round(r.w);
      rh = std::round(r.h);
      // Left upper pixel of the selectbox
      pixelindex = rx + ry * imgWidth;
      // Right upper pixel of the selectbox
      rightTop = pixelindex + rw;
      rightPos = rightTop;
      yDelta = 0;
    }
    void resetPosition(float imageWidth)
    {
      selectBox.imagewidth = imageWidth;
      resetPosition();
    }
    //Navigate through the vector inside the boundaries of the selectBox
    void nextPixel() // called by module->process()
    {
      //DEBUG(string::f("Thm: pixindex %d red %d", pixelindex, vrgb[pixelindex].r).c_str());
      if (++pixelindex >= vrgb.size())
        resetPosition();
      if (pixelindex == rightPos)
      {
        pixelindex += imgWidth - rw;
        rightPos += imgWidth;
        if (yDelta++ == rh)
          resetPosition();
      }
    }
    bool isEmpty()
    {
      return vrgb.empty();
    }
    void reserve(size_t size)
    {
      vrgb.reserve(size);
    }
    const RGB &getColor() const
    {
      return vrgb[pixelindex];
    }

  private:
    std::vector<RGB> vrgb{};
    const Rect &r = selectBox;
    uint pixelindex{};
    uint yDelta{};
    uint rightTop{};
    uint rightPos{};
    uint imgWidth{};
    uint rx{};
    uint ry{};
    uint rw{};
    uint rh{};
  };

  // Calculate and hold rgb- and hsv values
  struct ColorSpace
  {
    float red, green, blue;
    float hue, sat, lum;
    void calc(const thm::RGB& color)
    {
      red = color.r;
      green = color.g;
      blue = color.b;
      float r = red / 255.f;
      float g = green / 255.f;
      float b = blue / 255.f;
      float min = std::min({r, g, b});
      float max = std::max({r, g, b});
      //Luminance in percent
      lum = max + min / 2.f; 
      //Saturation in percent
      if (lum <= 0.5f)
        sat = (max - min) / (max + min);
      else
        sat = (max - min) / (2.f - max - min);
      sat = clamp(sat, 0.f, 1.f);
      // Grey hue. This avoids NaN issues
      if (r == g && g == b && b == r)
      {
        hue = 0.f;
        return;
      }
      // Hue in degrees 0..360 DEG
      if (r == max)
        hue = (g - b) / (max - min);
      if (g == max)
        hue = 2.f + (b - r) / (max - min);
      if (b == max)
        hue = 4.f + (r - g) / (max - min);
      hue *= 60.f;
      if (hue < 0.f)
        hue += 360.f;
    }
  };

  /*
    Drawing a box on the loaded image that serves as
    an area to choose pixels from.
  */
  struct SelectBoxView
  {
    using DrawArgs = rack::widget::Widget::DrawArgs;
    bool changed{false};
    void draw(const DrawArgs &args)
    {
      float w = args.clipBox.size.x;
      float h = args.clipBox.size.y;
      if(w<=1 || h<=1) //Seems to be true if draw is called the very first time?!
        return;
      // Selection should stay inside the parent box
      if (r.x + r.w >= w) r.x = w - r.w;
      if (r.y + r.h >= h) r.y = h - r.h;
      if (r.x < 0) r.x = 0;
      if (r.y < 0) r.y = 0;
      //DEBUG(string::f("Thm r.x: %f r.y: %f", r.x, r.y).c_str());
      drawBox(args, Grey, mStrokewidth, 0.f);
      drawBox(args, DGrey, mStrokewidth, mStrokewidth);
    }
    void hide()
    {
      mStrokewidth = 0.f;
    }
    void show()
    {
      mStrokewidth = 2.f;
    }
    void moveTo(float newx, float newy)
    {
      // r.x = newx - r.w;
      // r.y = newy - r.h;
      r.x = newx;
      r.y = newy;
    }
    void SetEndPoint(float x, float y)
    {
      // x1=mx+mw; mw=x1-mx; x1=x;
      r.w = abs(x - r.x);
      r.h = abs(y - r.y);
      if (r.w < 1) r.w = 1;
      if (r.h < 1) r.h = 1;
    }
    void setSize (float width, float height)
    {
      r.w = width;
      r.h = height;
    }
    void setBox (const thm::Rect& rp)
    {
      //r = rp;
      r.x = rp.x;
      r.y = rp.y;
      r.w = rp.w;
      r.h = rp.h;
      r.imagewidth = rp.imagewidth;
    }
    const Rect& getBox() const
    {
      return rect;
    }

  private:
    NVGcolor DGrey{nvgRGBA(22, 34, 22, 255)};
    NVGcolor Grey{nvgRGBA(200, 200, 200, 255)};
    float mStrokewidth{2.f};
    Rect rect{};  //Holds the rect points of this Selectionbox
    Rect &r = rect;

    void drawBox(const DrawArgs& args, const NVGcolor& c, float sw, float offset)
    {
      nvgStrokeColor(args.vg, c);
      nvgBeginPath(args.vg);
      nvgStrokeWidth(args.vg, sw);
      nvgRect(args.vg, r.x - offset, r.y - offset,
              r.w + offset * 2, r.h + offset * 2);
      nvgClosePath(args.vg);
      nvgStroke(args.vg);
    }
  };
};

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
#include "plugin.hpp"
#include "dep/lodepng/lodepng.h"
#include "osdialog.h"
#include <iostream>
#include "pictogramtools.hpp"

struct Pictogram : Module
{
  enum ParamId
  {
    SCALE_PARAM,
    OFFSET_PARAM,
    PARAMS_LEN
  };
  enum InputId
  {
    RESET_INPUT,
    CLOCK_INPUT,
    INPUTS_LEN
  };
  enum OutputId
  {
    RED_OUTPUT,
    GREEN_OUTPUT,
    BLUE_OUTPUT,
    HUE_OUTPUT,
    SAT_OUTPUT,
    LUM_OUTPUT,
    OUTPUTS_LEN
  };
  enum LightId
  {
    LIGHTS_LEN
  };

  std::string imagePath{};
  dsp::SchmittTrigger sTrigClock{};
  dsp::SchmittTrigger sTrigReset{};
  thm::ColorSpace clrSpace{};
  thm::RGBData rgbData{};
  thm::Rect slctView{};
  bool loading{false};
  bool hasLoadedImage{false};
  bool existJsonData{false};
  unsigned image_width{};
  unsigned image_height{};

  Pictogram()
  {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configParam(SCALE_PARAM, 1.f, 10.f, 1.f, "Scale", " V");
    configParam(OFFSET_PARAM, -5.f, 5.f, 0.5f, "Offset", " V");
    configInput(RESET_INPUT, "Reset");
    configInput(CLOCK_INPUT, "Clock");
    configOutput(RED_OUTPUT, "Red");
    configOutput(GREEN_OUTPUT, "Green");
    configOutput(BLUE_OUTPUT, "Blue");
    configOutput(HUE_OUTPUT, "Hue");
    configOutput(SAT_OUTPUT, "Saturation");
    configOutput(LUM_OUTPUT, "Luminance");
  }
  void process(const ProcessArgs& args) override
  {
    if (rgbData.isEmpty())
      return;
    if (sTrigReset.process(inputs[RESET_INPUT].getVoltage()))
      rgbData.resetPosition();
    if (!sTrigClock.process(inputs[CLOCK_INPUT].getVoltage()))
      return;
    clrSpace.calc(rgbData.getColor());
    rgbData.nextPixel();
    float red = rescale(clrSpace.red, 0.f, 255.f, 0.f, 10.f);
    float green = rescale(clrSpace.green, 0.f, 255.f, 0.f, 10.f);
    float blue = rescale(clrSpace.blue, 0.f, 255.f, 0.f, 10.f);
    float hue = rescale(clrSpace.hue, 0.f, 360.f, 0.f, 10.f);
    float sat = rescale(clrSpace.sat, 0.f, 1.f, 0.f, 10.f);
    float lum = rescale(clrSpace.lum, 0.f, 1.f, 0.f, 10.f);

    float scale = params[SCALE_PARAM].getValue();
    float offset = params[OFFSET_PARAM].getValue();

    auto transform = [&](float data)
    {
      float halfscl = scale / 2.f;
      return rescale(data, 0.f, 10.f, halfscl, -halfscl) + offset;
    };
    outputs[RED_OUTPUT].setVoltage(transform(red));
    outputs[GREEN_OUTPUT].setVoltage(transform(green));
    outputs[BLUE_OUTPUT].setVoltage(transform(blue));
    outputs[HUE_OUTPUT].setVoltage(transform(hue));
    outputs[SAT_OUTPUT].setVoltage(transform(sat));
    outputs[LUM_OUTPUT].setVoltage(transform(lum));
  }
  void loadSample(std::string path)
  {
    std::vector<uint8_t> image{};
    loading = true;
    rgbData.clear();
    unsigned error = lodepng::decode(image, image_width, image_height, path, LCT_RGB);
    if (error != 0)
    { //Todo: error logging
      std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
      imagePath.clear();
      loading = false;
      hasLoadedImage = false;
      return;
    }
    //DEBUG(string::f("ThmImageSize: %d", (int)image.size()).c_str());
    //DEBUG(string::f("ThmImageDim w,h: %d %d", image_width,image_height).c_str());
    rgbData.reserve(image_width * image_height);
    uint i = 0;
    do
    {
      rgbData.color.r = image[i++];
      rgbData.color.g = image[i++];
      rgbData.color.b = image[i++];
      rgbData.addColor();
    } while (i < image.size());
    rgbData.resetPosition(image_width);
    imagePath = path;
    loading = false;
    hasLoadedImage = true;
  }
  json_t *dataToJson() override
  {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "imagePath", json_string(imagePath.c_str()));
    json_object_set_new(rootJ, "SelBoxX", json_real(rgbData.selectBox.x));
    json_object_set_new(rootJ, "SelBoxY", json_real(rgbData.selectBox.y));
    json_object_set_new(rootJ, "SelBoxW", json_real(rgbData.selectBox.w));
    json_object_set_new(rootJ, "SelBoxH", json_real(rgbData.selectBox.h));

    json_object_set_new(rootJ, "SelectViewX", json_real(slctView.x));
    json_object_set_new(rootJ, "SelectViewY", json_real(slctView.y));
    json_object_set_new(rootJ, "SelectViewW", json_real(slctView.w));
    json_object_set_new(rootJ, "SelectViewH", json_real(slctView.h));

    json_object_set_new(rootJ, "existJsonData", json_boolean(existJsonData));
    return rootJ;
  }
  void dataFromJson(json_t *rootJ) override
  {
    auto imagePathJ = json_object_get(rootJ, "imagePath");
    if (imagePathJ)
      loadSample(json_string_value(imagePathJ));
    auto SelBoxX = json_object_get(rootJ, "SelBoxX");
    if(SelBoxX)
      rgbData.selectBox.x = json_real_value(SelBoxX);
    auto SelBoxY = json_object_get(rootJ, "SelBoxY");
    if (SelBoxY)
      rgbData.selectBox.y = json_real_value(SelBoxY);
    auto SelBoxW = json_object_get(rootJ, "SelBoxW");
    if (SelBoxW)
      rgbData.selectBox.w = json_real_value(SelBoxW);
    auto SelBoxH = json_object_get(rootJ, "SelBoxH");
    if (SelBoxH)
      rgbData.selectBox.h = json_real_value(SelBoxH);

    auto slctViewX = json_object_get(rootJ, "SelectViewX");
    if (slctViewX)
      slctView.x = json_real_value(slctViewX);
    auto slctViewY = json_object_get(rootJ, "SelectViewY");
    if (slctViewY)
      slctView.y = json_real_value(slctViewY);
    auto slctViewW = json_object_get(rootJ, "SelectViewW");
    if (slctViewW)
      slctView.w = json_real_value(slctViewW);
    auto slctViewH = json_object_get(rootJ, "SelectViewH");
    if (slctViewH)
      slctView.h = json_real_value(slctViewH);

    auto pExistJsonData = json_object_get(rootJ, "existJsonData");
    if (pExistJsonData)
      existJsonData = json_boolean_value(pExistJsonData);
    else
      existJsonData = false;
  }
};
        
struct PictogramDisplay : OpaqueWidget
{
  Pictogram *module{nullptr};
  int imgHandle {0};
  const int sizex {346};
//  const int sizex{330};
  const int sizey{330};
  thm::SelectBoxView boxView{};

  void onHoverKey(const HoverKeyEvent& e) override 
  {
    OpaqueWidget::onHoverKey(e);
    int modKey = e.mods;
    int key = e.key;
    bool isShift = (modKey & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT;
    if (isShift && key == GLFW_KEY_SPACE)
      return;
    int w = boxView.getBox().w / 2;
    int h = boxView.getBox().h / 2;
    if (isShift)
      boxView.moveTo(e.pos.x-w, e.pos.y-h);
    if(key == GLFW_KEY_SPACE)
      boxView.SetEndPoint(e.pos.x, e.pos.y);
    boxView.changed = true;
  }
  void drawLayer(const DrawArgs& args, int layer) override
  {
    OpaqueWidget::drawLayer(args, layer);
    if (!module)
      return;
    // If module is blank full size box would block mouse dragging!
    box.pos = Vec(0,0); 
    box.size = Vec(1,1);
    if (module->loading || layer != 1 || module->imagePath.empty())
      return;
    float width = sizex;
    float height = sizey;
    float imagew = module->image_width;
    float imageh = module->image_height;
    float ratio = imagew / imageh;
    if (ratio > 1)
      height /= ratio;
    else
      width *= ratio;
    float marginx = (parent->box.size.x - width) / 2;
    float marginy = (parent->box.size.y - height) / 2;
    float zoomx = width / imagew;
    float zoomy = height / imageh;
    float izx = imagew * zoomx;
    float izy = imageh * zoomy;
    box.pos = Vec(marginx, marginy);
    box.size = Vec(izx, izy);
    nvgSave(args.vg);
    nvgBeginPath(args.vg);
    // Make sure image is created only once after it was loaded
    // with module->loadSample(...). DrawLayer runs in a loop
    // at FPS-speed e.g. 60 frames per second!
    if (module->hasLoadedImage)
    {
      // Should not run outside this "if" statement. It's too slow for that!
      imgHandle = nvgCreateImage(args.vg, module->imagePath.c_str(), 0);
      if (!module->existJsonData)
      {
        boxView.setSize(30, 30);
        boxView.moveTo(box.size.x / 2, box.size.y / 2);
        module->existJsonData = true;
      }else
        boxView.setBox(module->slctView);
      SetRgbDataSelectBox(imagew, zoomx, zoomy);
      module->hasLoadedImage = false;
    }
    NVGpaint imgPaint = nvgImagePattern(args.vg, 0, 0, izx, izy,
                                        0, imgHandle, 1.0f);
    nvgRect(args.vg, 0, 0, izx, izy);
    nvgFillPaint(args.vg, imgPaint);
    nvgFill(args.vg);
    boxView.draw(args);
    nvgClosePath(args.vg);
    nvgRestore(args.vg);
    // Adjusting the select box in module after moving or resizing
    if (boxView.changed)
    {
      module->slctView = boxView.getBox();
      SetRgbDataSelectBox(imagew, zoomx, zoomy);
      boxView.changed = false;
    }
  }
  void SetRgbDataSelectBox(float imagewidth, float zx, float zy)
  {
    thm::Rect &rt = module->rgbData.selectBox;
    rt = boxView.getBox();
    rt.imagewidth = imagewidth;
    rt.zoom(zx, zy);
    module->rgbData.resetPosition();
  }
};

struct PictogramWidget : ModuleWidget
{
  Pictogram *myModule{nullptr};
 
  PictogramWidget(Pictogram *module)
  {
    myModule = module;
    setModule(myModule);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Pictogram.svg")));
    PictogramDisplay *display = new PictogramDisplay();
    display->module = myModule;
    addChild(display);
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(143.84, 13.584)), module, Pictogram::SCALE_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(143.84, 28.06)), module, Pictogram::OFFSET_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 42.536)), module, Pictogram::RESET_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(9.0, 57.012)), module, Pictogram::CLOCK_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 42.536)), module, Pictogram::RED_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 57.012)), module, Pictogram::GREEN_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 71.488)), module, Pictogram::BLUE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 85.964)), module, Pictogram::HUE_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 100.44)), module, Pictogram::SAT_OUTPUT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(143.84, 114.916)), module, Pictogram::LUM_OUTPUT));
  }
  void onPathDrop(const PathDropEvent& e) override
  {
    Widget::onPathDrop(e);
    this->myModule->loadSample(e.paths[0]);
  }

  struct MenuItemLoadpng : MenuItem
  {
    Pictogram *module{nullptr};
 
    void onAction(const event::Action& e) override
    {
      std::string dir = module->imagePath.empty() ? asset::user("") : rack::system::getDirectory(module->imagePath);
      char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), nullptr, nullptr);
      if (path)
        module->loadSample(path);
    }
  };

  void appendContextMenu(Menu *menu) override
{
    menu->addChild(new MenuSeparator);
    MenuItemLoadpng *ItemLoadpng = createMenuItem<MenuItemLoadpng>("Load image(PNG)");
    ItemLoadpng->module = this->myModule;
    menu->addChild(ItemLoadpng);
  }
};

Model *modelPictogram = createModel<Pictogram, PictogramWidget>("Pictogram");
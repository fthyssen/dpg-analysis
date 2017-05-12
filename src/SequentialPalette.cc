#include "DPGAnalysis/Tools/interface/SequentialPalette.h"

#include <algorithm>

SequentialPalette::SequentialPalette(bool _reverse)
{
    colors_.reserve(9);
    colors_.push_back(new TColor(255./255., 255./255., 229./255.));
    colors_.push_back(new TColor(247./255., 252./255., 185./255.));
    colors_.push_back(new TColor(217./255., 240./255., 163./255.));
    colors_.push_back(new TColor(173./255., 221./255., 142./255.));
    colors_.push_back(new TColor(120./255., 198./255., 121./255.));
    colors_.push_back(new TColor(65./255., 171./255., 93./255.));
    colors_.push_back(new TColor(35./255., 132./255., 67./255.));
    colors_.push_back(new TColor(0./255., 104./255., 55./255.));
    colors_.push_back(new TColor(0./255., 69./255., 41./255.));

    palette_.reserve(colors_.size());
    for (std::vector<TColor *>::const_iterator _color = colors_.begin()
             ; _color != colors_.end() ; ++_color)
        palette_.push_back((*_color)->GetNumber());
    if (_reverse)
        std::reverse(palette_.begin(), palette_.end());
}

SequentialPalette::~SequentialPalette()
{
    // TColor owned by gRoot
}

void SequentialPalette::write() const
{
    for (std::vector<TColor *>::const_iterator _color = colors_.begin()
             ; _color != colors_.end() ; ++_color)
        (*_color)->Write();
}

void SequentialPalette::applyPalette(TStyle * _tstyle, float _alpha)
{
    _tstyle->SetPalette(palette_.size(), (int *)(&(palette_.at(0))), _alpha);
    _tstyle->SetNumberContours(palette_.size());
}

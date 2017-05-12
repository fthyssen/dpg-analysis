#include "DPGAnalysis/Tools/interface/DivergingPalette.h"

#include <algorithm>

DivergingPalette::DivergingPalette(bool _reverse)
{
    colors_.reserve(9);
    colors_.push_back(new TColor(49./255., 54./255., 149./255.));
    colors_.push_back(new TColor(69./255., 117./255., 180./255.));
    colors_.push_back(new TColor(116./255., 173./255., 209./255.));
    colors_.push_back(new TColor(171./255., 217./255., 233./255.));
    colors_.push_back(new TColor(254./255., 224./255., 144./255.));
    colors_.push_back(new TColor(253./255., 174./255., 97./255.));
    colors_.push_back(new TColor(244./255., 109./255., 67./255.));
    colors_.push_back(new TColor(215./255., 48./255., 39./255.));
    colors_.push_back(new TColor(165./255., 0./255., 38./255.));

    palette_.reserve(colors_.size());
    for (std::vector<TColor *>::const_iterator _color = colors_.begin()
             ; _color != colors_.end() ; ++_color)
        palette_.push_back((*_color)->GetNumber());
    if (_reverse)
        std::reverse(palette_.begin(), palette_.end());
}

DivergingPalette::~DivergingPalette()
{
    // TColor owned by gRoot
}

void DivergingPalette::write() const
{
    for (std::vector<TColor *>::const_iterator _color = colors_.begin()
             ; _color != colors_.end() ; ++_color)
        (*_color)->Write();
}

void DivergingPalette::applyPalette(TStyle * _tstyle, float _alpha)
{
    _tstyle->SetPalette(palette_.size(), (int *)(&(palette_.at(0))), _alpha);
    _tstyle->SetNumberContours(palette_.size());
}

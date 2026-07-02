#pragma once

#include "plot2D_plot_layer.h"


class Plot2DGrid : public PlotLayer, public QObject
{
public:
    Plot2DGrid();
    bool draw(Plot2D* parent, Dataset* dataset);

    void setAngleVisibility(bool state);
    // void setVetricalNumber(int grids) { _lines = grids; }
    void setVelocityVisible(bool visible) { _velocityVisible = visible; }
    void setRangeFinderVisible(bool visible) { _rangeFinderLastVisible = visible; }
    void setTemperatureVisible(bool state) { temperatureVisible_ = state; };
    bool isFillWidth() const { return fillWidth_; }
    void setFillWidth(bool state) { fillWidth_ = state; }
    bool isInvert() const { return invert_; }
    void setInvert(bool state) { invert_ = state; }

    void setLoRngRange(int minLoRng, int maxLoRng);

protected:
    bool angleVisibility_;
    bool _velocityVisible = true;
    bool _rangeFinderLastVisible = true;
    bool temperatureVisible_ = true;
    bool fillWidth_ = false;
    bool invert_ = false;

    int loRngMin_ = 0, loRngMax_ = 1600;
};

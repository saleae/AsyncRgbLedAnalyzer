#include "AsyncRgbLedHelpers.h"

void RGBValue::ConvertToControllerOrder(ColorLayout layout, U16* values) const
{
    switch (layout) {
    case LAYOUT_GRB:
        values[0] = green;
        values[1] = red;
        values[2] = blue;
        break;

    case LAYOUT_RGB:
        values[0] = red;
        values[1] = green;
        values[2] = blue;
        break;
    }
}

RGBValue RGBValue::CreateFromControllerOrder(ColorLayout layout, U16* values)
{
    switch (layout) {
    case LAYOUT_GRB:
        return RGBValue{values[1], values[0], values[2]};
    
    case LAYOUT_RGB:
        return RGBValue{values[0], values[1], values[2]};
    }
}

bool TimingTolerance::WithinTolerance(const double t) const
{
    return (t >= mMinimumSec) && (t <= mMaximumSec);
}

bool BitTiming::WithinTolerance(const double positiveTime, const double negativeTime) const
{
    return mPositiveTiming.WithinTolerance(positiveTime) &&
            mNegativeTiming.WithinTolerance(negativeTime);
}

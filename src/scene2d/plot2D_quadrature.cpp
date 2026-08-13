#include "plot2D_quadrature.h"
#include "plot2D.h"


Plot2DQuadrature::Plot2DQuadrature()
{}

bool Plot2DQuadrature::draw(Plot2D* parent, Dataset* dataset)
{
    auto &canvas = parent->canvas();
    auto &cursor = parent->cursor();

    Q_UNUSED(dataset);

    if (!isVisible() || !cursor.distance.isValid()) {
        return false;
    }

    QVector<float> real1(canvas.width());
    QVector<float> imag1(canvas.width());
    real1.fill(NAN);
    imag1.fill(NAN);

    QVector<float> real2(canvas.width());
    QVector<float> imag2(canvas.width());
    real2.fill(NAN);
    imag2.fill(NAN);

    QVector<float> real3(canvas.width());
    QVector<float> imag3(canvas.width());
    real3.fill(NAN);
    imag3.fill(NAN);

    QVector<float> real4(canvas.width());
    QVector<float> imag4(canvas.width());
    real4.fill(NAN);
    imag4.fill(NAN);


    return true;
}

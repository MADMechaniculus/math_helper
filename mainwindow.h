#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileInfo>
#include <QFileDialog>
#include <QClipboard>

#include <cstdint>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <complex>
#include <cmath>
#include <iterator>
#include <cmath>
#include <queue>
#include <cstring>

typedef struct {
    int16_t I;
    int16_t Q;
} iIQ_t;

#define pow2(x) (uint32_t)(0x1 << x)

// =============================================================================
// Fast Furier Transform impl
// =============================================================================
unsigned int bitReverse(unsigned int x, int log2n);

template<class Iter_T>
void fft(Iter_T a, Iter_T b, int log2n)
{
    typedef typename std::iterator_traits<Iter_T>::value_type complex;
    const complex J(0, 1);
    int n = 1 << log2n;
    for (unsigned int i=0; i < n; ++i) {
        b[bitReverse(i, log2n)] = a[i];
    }
    for (int s = 1; s <= log2n; ++s) {
        int m = 1 << s;
        int m2 = m >> 1;
        complex w(1, 0);
        complex wm = exp(-J * (M_PIf32 / m2));
        for (int j=0; j < m2; ++j) {
            for (int k=j; k < n; k += m) {
                complex t = w * b[k + m2];
                complex u = b[k];
                b[k] = u + t;
                b[k + m2] = u - t;
            }
            w *= wm;
        }
    }
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QClipboard * clipboard;
    QString selectedFilePath;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

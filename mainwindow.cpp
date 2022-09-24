#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->clipboard = QGuiApplication::clipboard();

    this->ui->pushButton_2->setEnabled(false);
    this->ui->pushButton_3->setEnabled(false);
    this->ui->pushButton_4->setEnabled(false);
    this->ui->pushButton_5->setEnabled(false);
    this->ui->pushButton_6->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    this->selectedFilePath = QFileDialog::getOpenFileName(this, tr("Выберите данные для обработки"), \
                                                          QApplication::applicationDirPath(), \
                                                          tr("Binary files (*.bin *.dat *.pcm)"));
    if (!this->selectedFilePath.isEmpty()) {
        this->ui->selectedFile->setText(this->selectedFilePath);

        this->ui->pushButton_2->setEnabled(true);
        this->ui->pushButton_3->setEnabled(true);
        this->ui->pushButton_4->setEnabled(true);
        this->ui->pushButton_5->setEnabled(true);
        this->ui->pushButton_6->setEnabled(true);
    } else {
        this->ui->selectedFile->setText("");

        this->ui->pushButton_2->setEnabled(false);
        this->ui->pushButton_3->setEnabled(false);
        this->ui->pushButton_4->setEnabled(false);
        this->ui->pushButton_5->setEnabled(false);
        this->ui->pushButton_6->setEnabled(false);
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    QFileInfo fileInfo(this->selectedFilePath);

    uint64_t fileSize = fileInfo.size();

    std::vector<iIQ_t> buffer(fileSize / sizeof(iIQ_t));
    std::vector<float> ampt(buffer.size());

    std::ifstream inputStream(this->selectedFilePath.toStdString(), std::ios::binary);
    if (inputStream.is_open()) {

        inputStream.read((char*)buffer.data(), fileSize);
        inputStream.close();

        QString resultFile = fileInfo.dir().absolutePath() + "/" + fileInfo.fileName() + ".ampt";
        clipboard->setText(resultFile);

        std::transform(std::begin(buffer), std::end(buffer), std::begin(ampt), [](const iIQ_t & item) -> float {
            std::complex<float> tmp((float)item.I, (float)item.Q);
            return std::abs(tmp);
        });

        std::ofstream outputStream(resultFile.toStdString(), std::ios::binary);
        if (outputStream.is_open()) {

            outputStream.write((char*)ampt.data(), ampt.size() * sizeof (float));
            outputStream.close();

            this->ui->statusbar->showMessage("The resulting path is placed in clipboard");

        } else {
            this->ui->statusbar->showMessage("Error on opening result file!");
        }

    } else {
        this->ui->statusbar->showMessage("Error on opening target file!");
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    QFileInfo fileInfo(this->selectedFilePath);

    uint64_t fileSize = fileInfo.size();

    uint32_t fftScale = 18;
    while (fileSize < (0x1 << fftScale)) {
        fftScale--;
    }

    std::vector<iIQ_t> buffer(fileSize / sizeof(iIQ_t));
    std::vector<std::complex<float>> complexBuffer(buffer.size());
    std::vector<std::complex<float>> fftBuffer((0x1 << fftScale));
    std::vector<float> outputBuffer((0x1 << fftScale));
    std::vector<float> half_l, half_r;

    std::ifstream inputStream(this->selectedFilePath.toStdString(), std::ios::binary);
    if (inputStream.is_open()) {

        inputStream.read((char*)buffer.data(), fileSize);
        inputStream.close();

        QString resultFile = fileInfo.dir().absolutePath() + "/" + fileInfo.fileName() + ".fft";
        clipboard->setText(resultFile);

        std::transform(std::begin(buffer), std::end(buffer), std::begin(complexBuffer), [](const iIQ_t & item) -> std::complex<float> {
            return std::complex<float>((float)item.I, (float)item.Q);
        });

        fft(std::begin(complexBuffer), std::begin(fftBuffer), fftScale);
        std::transform(std::begin(fftBuffer), std::end(fftBuffer), std::begin(outputBuffer), [](const std::complex<float> & item) {
            return std::abs(item);
        });

        half_l = std::vector<float>{outputBuffer.begin(), outputBuffer.begin() + outputBuffer.size() / 2};
        half_r = std::vector<float>{outputBuffer.begin() + outputBuffer.size() / 2, outputBuffer.end()};

        std::copy(std::begin(half_r), std::end(half_r), outputBuffer.begin());
        std::copy(std::begin(half_l), std::end(half_l), outputBuffer.begin() + half_r.size());

        std::ofstream outputStream(resultFile.toStdString(), std::ios::binary);
        if (outputStream.is_open()) {

            outputStream.write((char*)outputBuffer.data(), outputBuffer.size() * sizeof (float));
            outputStream.close();

            this->ui->statusbar->showMessage("The resulting path is placed in clipboard");

        } else {
            this->ui->statusbar->showMessage("Error on opening result file!");
        }

    } else {
        this->ui->statusbar->showMessage("Error on opening target file!");
    }
}


unsigned int bitReverse(unsigned int x, int log2n) {
    int n = 0;
    int mask = 0x1;
    for (int i=0; i < log2n; i++) {
        n <<= 1;
        n |= (x & 1);
        x >>= 1;
    }
    return n;
}

void MainWindow::on_pushButton_4_clicked()
{
    QFileInfo fileInfo(this->selectedFilePath);

    uint64_t fileSize = fileInfo.size();

    std::vector<iIQ_t> buffer(fileSize / sizeof(iIQ_t));
    std::vector<std::complex<float>> ampt(buffer.size());

    std::ifstream inputStream(this->selectedFilePath.toStdString(), std::ios::binary);
    if (inputStream.is_open()) {

        inputStream.read((char*)buffer.data(), fileSize);
        inputStream.close();

        QString resultFile = fileInfo.dir().absolutePath() + "/" + fileInfo.fileName() + ".mult";
        clipboard->setText(resultFile);

        std::transform(std::begin(buffer), std::end(buffer), std::begin(ampt), [](const iIQ_t & item) {
            return std::complex<float>((float)item.I, (float)item.Q) * std::complex<float>((float)item.I, (float)item.Q);
        });
        std::transform(std::begin(ampt), std::end(ampt), std::begin(buffer), [](const std::complex<float> & item) {
            return iIQ_t{(int16_t)item.real(), (int16_t)item.imag()};
        });

        std::ofstream outputStream(resultFile.toStdString(), std::ios::binary);
        if (outputStream.is_open()) {

            outputStream.write((char*)buffer.data(), buffer.size() * sizeof (iIQ_t));
            outputStream.close();

            this->ui->statusbar->showMessage("The resulting path is placed in clipboard");

        } else {
            this->ui->statusbar->showMessage("Error on opening result file!");
        }

    } else {
        this->ui->statusbar->showMessage("Error on opening target file!");
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    QFileInfo fileInfo(this->selectedFilePath);

    uint64_t fileSize = fileInfo.size();

    std::vector<float> amptBuffer(fileSize / sizeof(float));
    std::vector<float> diffAmptResult(amptBuffer.size());

    auto getDiff = [] (const std::vector<float> & data, uint32_t idx, uint32_t step) -> float {
        if (idx < step)
            return 0;
        if (idx + step > data.size())
            return 0;
        return (data[idx + step] - data[idx - step]) / (2 * (float)step);
    };

    std::ifstream inputStream(this->selectedFilePath.toStdString(), std::ios::binary);
    if (inputStream.is_open()) {

        inputStream.read((char*)amptBuffer.data(), fileSize);
        inputStream.close();

        QString resultFile = fileInfo.dir().absolutePath() + "/" + fileInfo.fileName() + ".diff";
        clipboard->setText(resultFile);

        for (uint32_t index = 0; index < diffAmptResult.size(); index++) {
            diffAmptResult[index] = std::abs(getDiff(amptBuffer, index, this->ui->spinBox->value()));
        }

        std::ofstream outputStream(resultFile.toStdString(), std::ios::binary);
        if (outputStream.is_open()) {

            outputStream.write((char*)diffAmptResult.data(), diffAmptResult.size() * sizeof (float));
            outputStream.close();

            this->ui->statusbar->showMessage("The resulting path is placed in clipboard");

        } else {
            this->ui->statusbar->showMessage("Error on opening result file!");
        }

    } else {
        this->ui->statusbar->showMessage("Error on opening target file!");
    }
}


void MainWindow::on_pushButton_6_clicked()
{
    QFileInfo fileInfo(this->selectedFilePath);

    uint64_t fileSize = fileInfo.size();

    std::vector<float> amptBuffer(fileSize / sizeof(float));
    std::vector<float> filteredAmptResult(amptBuffer.size());

    auto avgFilter = [] (std::vector<float> & source, std::vector<float> & dest, uint32_t window) {
        std::list<float> windowBuffer;
        while (windowBuffer.size() != window)
            windowBuffer.push_back(0.0f);

        for (size_t i = 0; i < source.size(); i++) {
            windowBuffer.pop_front();
            windowBuffer.push_back(source[i]);

            dest[i] = std::accumulate(std::begin(windowBuffer), std::end(windowBuffer), 0.0f) / (float)window;
        }
    };

    std::ifstream inputStream(this->selectedFilePath.toStdString(), std::ios::binary);
    if (inputStream.is_open()) {

        inputStream.read((char*)amptBuffer.data(), fileSize);
        inputStream.close();

        QString resultFile = fileInfo.dir().absolutePath() + "/" + fileInfo.fileName() + ".filt";
        clipboard->setText(resultFile);

        avgFilter(amptBuffer, filteredAmptResult, this->ui->spinBox_2->value());

        std::ofstream outputStream(resultFile.toStdString(), std::ios::binary);
        if (outputStream.is_open()) {

            outputStream.write((char*)filteredAmptResult.data(), filteredAmptResult.size() * sizeof (float));
            outputStream.close();

            this->ui->statusbar->showMessage("The resulting path is placed in clipboard");

        } else {
            this->ui->statusbar->showMessage("Error on opening result file!");
        }

    } else {
        this->ui->statusbar->showMessage("Error on opening target file!");
    }
}


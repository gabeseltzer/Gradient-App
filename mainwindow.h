#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QVector>

#include <memory>

namespace Ui {
    // Forward declaring Ui::MainWindows that is defined in `ui_mainwindow.h`
    // which is a pre-build-time generated file from mainwindow.ui.
    class MainWindow; 
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void on_openFileButton_clicked();

    void on_gradientStartSlider_sliderMoved(int position);

    void on_gradientEndSlider_sliderMoved(int position);

    void on_gradientStartTextField_editingFinished();

    void on_gradientStartTextField_returnPressed();

    void on_gradientEndTextField_editingFinished();

    void on_gradientEndTextField_returnPressed();

    void on_writeGcodeButton_clicked();

    void on_gradientStartColorButton_clicked();

    void on_gradientEndColorButton_clicked();

    void on_printerBox_currentIndexChanged(const QString &arg1);
    
private:
    std::unique_ptr<Ui::MainWindow> ui;

    void openFile();
    void processGcode();
    int countLayers();
    int calculateGradientShifts(int start, int end, int startPercent, int endPercent);
};

#endif // MAINWINDOW_H

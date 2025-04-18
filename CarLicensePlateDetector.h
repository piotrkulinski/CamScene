#ifndef CARLICENSEPLATEDETECTOR_H
#define CARLICENSEPLATEDETECTOR_H

#include <IFrameAnalyzer.h>
#include <QDebug>
#include <opencv2/core/mat.hpp>

class CarLicensePlateDetector : public IFrameAnalyzer {
    Q_OBJECT
public:
    CarLicensePlateDetector(QObject *parent = nullptr) : IFrameAnalyzer(parent) {}
    void analyze(const cv::Mat &base_frame, const cv::Mat &frame) override {
        qDebug() << "[CarLicensePlateDetector] Analyzing frame of size:" << frame.cols << "x" << frame.rows;
        QString plate = "XYZ1234";
        emit plateRecognized(plate);
        emit onAnalysisComplete();
    }

signals:
    void plateRecognized(const QString &plate);
};
#endif // CARLICENSEPLATEDETECTOR_H

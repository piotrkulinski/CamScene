#ifndef MOTIONDETECTOR_H
#define MOTIONDETECTOR_H
#include "opencv2/core.hpp"
#pragma once

#include "IFrameAnalyzer.h"
#include <QDebug>
#include <opencv2/core/mat.hpp>

class MotionDetector : public IFrameAnalyzer {
    Q_OBJECT
public:
    MotionDetector(QObject *parent = nullptr) : IFrameAnalyzer(parent) {}
private:
    /*
   * PSNR (Peak Signal-to-Noise Ratio) – im wyższy, tym obrazy bardziej podobne
   * PSNR > 30 → bardzo podobne obrazy
   * PSNR ~ 20 → średnia jakość
   * PSNR < 10 → duże różnice
   */
    double getPSNR(const cv::Mat &I1, const cv::Mat &I2) {
        cv::Mat s1;
        cv::absdiff(I1, I2, s1);  // różnica pikseli
        s1.convertTo(s1, CV_32F); // konwersja do float
        s1 = s1.mul(s1);          // kwadrat

        cv::Scalar s = cv::sum(s1); // suma wartości

        double sse = s.val[0] + s.val[1] + s.val[2]; // suma błędów
        if (sse <= 1e-10)
            return INFINITY; // identyczne obrazy
        double mse = sse / (double)(I1.channels() * I1.total());
        double psnr = 10.0 * log10((255 * 255) / mse);
        return psnr;
    }

    /*
   * MSE (Mean Squared Error) – im mniejszy, tym obrazy bardziej podobne
   * MSE ≈ 0 → obrazy niemal identyczne
   * większa wartość → większe różnice
   */
    double getMSE(const cv::Mat &I1, const cv::Mat &I2) {
        cv::Mat s1;
        cv::absdiff(I1, I2, s1);  // różnica pikseli
        s1.convertTo(s1, CV_32F); // float
        s1 = s1.mul(s1);          // kwadrat

        cv::Scalar s = cv::sum(s1); // suma
        double mse =
            (s.val[0] + s.val[1] + s.val[2]) / (double)(I1.channels() * I1.total());
        return mse;
    }

public:
    void analyze(const cv::Mat& base_frame, const cv::Mat& frame) override {
        //qDebug() << "[MotionDetector] Analyzing frame of size:" << frame.cols << "x" << frame.rows;
        if (base_frame.size() == frame.size()) { // && base_frame.type() == roi.type()) {
            double psnr = getPSNR(base_frame, frame);
            //double mse = getMSE(base_frame, frame);
            //qDebug() << "\tMSE:" << Qt::dec << mse << "\tPSNR:" << Qt::dec << psnr << "/" << mse;
            if (psnr < 20.000f) {
                const bool movementDetected {true};  // przykładowa logika detekcji
                emit onMotionDetected(movementDetected);
                emit onAnalysisComplete();
            }
        } else {
            qDebug() << "Obrazy mają różne rozmiary lub typy!";
        }
    }

signals:
    void onMotionDetected(bool movement);
};
#endif // MOTIONDETECTOR_H

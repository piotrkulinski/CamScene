#ifndef IFRAMEANALYZER_H
#define IFRAMEANALYZER_H

#pragma once
#include <opencv2/core/mat.hpp>
#include <QObject>

class IFrameAnalyzer  : public QObject {
    Q_OBJECT
public:
    //using QObject::QObject;
    explicit IFrameAnalyzer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IFrameAnalyzer() = default;
    virtual void analyze(const cv::Mat& base_frame, const cv::Mat& frame) = 0; //ATD
signals:
    void onAnalysisComplete();
};

#endif // IFRAMEANALYZER_H

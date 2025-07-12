#pragma once

#include <QDialog>
#include <QThread>

namespace Ui {
class DRFXProgressDialog;
}

class DRFXProgressDialog : public QDialog
{
    Q_OBJECT

public:
    typedef std::function<void(DRFXProgressDialog *p, QThread *t)> TWorkerCallback;
    typedef std::function<void(DRFXProgressDialog *p)> TCompleteCallback;
    explicit DRFXProgressDialog(QWidget *parent = nullptr);
    ~DRFXProgressDialog();
    void run(const TWorkerCallback &worker, const TCompleteCallback &completion);

signals:
    void progressReset();
    void updateRange(int minimum, int maximum);
    void updateMinimum(int minimum);
    void updateMaximum(int maximum);
    void updateValue(int value);
    void updateText(const QString &message);
    void showError(const QString &message);

public slots:
    void reset();
    void setRange(int minimum, int maximum);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int value);
    void setMessage(const QString &message);
    void setError(const QString &message);

private:
    Ui::DRFXProgressDialog *ui;
};

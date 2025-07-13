#include "drfxprogressdialog.h"
#include "ui_drfxprogressdialog.h"
#include <QMessageBox>
#include <QMessageBox>
#include <QPushButton>
#include <QScreen>

DRFXProgressDialog::DRFXProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DRFXProgressDialog)
    , m_isError(false)
{
    ui->setupUi(this);
    ui->progressBar->reset();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
   
    QScreen *screen = qApp->primaryScreen();
    int width = geometry().width();
    int hight = geometry().height();
    if (width > 0 && width < screen->size().width() && hight > 0 && hight < screen->size().height()) {
        uint centerX = screen->size().width() / 2 - width / 2;
        uint centerY = screen->size().height() / 2 - hight / 2;
        setGeometry(centerX, centerY, width, hight);
    }

    // call all events in main thread context!ss
    connect(this, &DRFXProgressDialog::showComplete, this, &DRFXProgressDialog::onComplete, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::progressReset, ui->progressBar, &QProgressBar::reset, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::updateRange, ui->progressBar, &QProgressBar::setRange, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::updateMinimum, ui->progressBar, &QProgressBar::setMinimum, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::updateMaximum, ui->progressBar, &QProgressBar::setMaximum, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::updateValue, ui->progressBar, &QProgressBar::setValue, Qt::QueuedConnection);
    connect(this, &DRFXProgressDialog::updateText, ui->label, &QLabel::setText, Qt::QueuedConnection);
    connect(
        this,
        &DRFXProgressDialog::showError,
        this,
        [parent](const QString &message) { //
            QMessageBox::critical(parent, qApp->applicationDisplayName(), message);
        },
        Qt::QueuedConnection);
}

DRFXProgressDialog::~DRFXProgressDialog()
{
    delete ui;
}

void DRFXProgressDialog::complete(const QString& message) {
    emit showComplete(message);
}

void DRFXProgressDialog::run(const TWorkerCallback &worker, const TCompleteCallback &completion)
{
    show();
    QThread *t = new QThread(this);
    connect(
        ui->buttonBox,
        &QDialogButtonBox::rejected,
        this,
        [this, t]() {
            QPushButton *b;
            if ((b = ui->buttonBox->button(QDialogButtonBox::Cancel))) {
                b->setEnabled(false);
            }
            t->requestInterruption();
            t->wait(5000);
        },
        Qt::QueuedConnection);
    connect(
        t,
        &QThread::started,
        this,
        [this, worker, t]() {
            worker(this, t);
            t->exit(0);
        },
        Qt::DirectConnection);
    connect(
        t,
        &QThread::finished,
        this,
        [t]() { //
            t->deleteLater();
        },
        Qt::DirectConnection);
    connect(
        t,
        &QThread::destroyed,
        this,
        [this, completion](QObject *) {
            completion(this);
            this->reject();
        },
        Qt::QueuedConnection);
    t->start(QThread::HighPriority);
}

void DRFXProgressDialog::reset()
{
    m_isError = false;
    emit progressReset();
}

void DRFXProgressDialog::setRange(int minimum, int maximum)
{
    emit updateRange(minimum, maximum);
}

void DRFXProgressDialog::setMinimum(int minimum)
{
    emit updateMinimum(minimum);
}

void DRFXProgressDialog::setMaximum(int maximum)
{
    emit updateMaximum(maximum);
}

void DRFXProgressDialog::setValue(int value)
{
    emit updateValue(value);
}

void DRFXProgressDialog::setMessage(const QString &message)
{
    emit updateText(message);
}

void DRFXProgressDialog::setError(const QString &message)
{
    m_isError = true;
    emit showError(message);
}

void DRFXProgressDialog::onComplete(const QString &message)
{
    QMessageBox::information(this, qApp->applicationDisplayName(), message);
}

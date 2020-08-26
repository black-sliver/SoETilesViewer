#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QTextDocument>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr, const QString& text="", QTextDocument::FindFlags flags=QTextDocument::FindFlags(), bool regex=false);
    ~FindDialog();

    QString text() const;
    QTextDocument::FindFlags flags() const;
    bool regex() const;

private:
    Ui::FindDialog *ui;
    QTextDocument::FindFlags _flags;
};

#endif // FINDDIALOG_H

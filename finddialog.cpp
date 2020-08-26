#include "finddialog.h"
#include "ui_finddialog.h"
#include <QPushButton>

FindDialog::FindDialog(QWidget *parent, const QString& text, QTextDocument::FindFlags flags, bool regex) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Find");
    ui->lineEdit->setText(text);
    ui->lineEdit->setSelection(0, ui->lineEdit->text().length());
    ui->chkRegex->setChecked(regex);
    this->_flags = flags;
    ui->chkCaseSensitive->setChecked(flags & QTextDocument::FindCaseSensitively);
    setWindowFlag(Qt::Tool);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::text() const
{
    return ui->lineEdit->text();
}

QTextDocument::FindFlags FindDialog::flags() const
{
    if (ui->chkCaseSensitive->isChecked())
        return _flags|QTextDocument::FindCaseSensitively;
    else
        return _flags&(~QTextDocument::FindCaseSensitively);
}

bool FindDialog::regex() const
{
    return ui->chkRegex->isChecked();
}

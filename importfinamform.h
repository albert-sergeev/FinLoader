#ifndef IMPORTFINAMFORM_H
#define IMPORTFINAMFORM_H

#include <QWidget>
#include<filesystem>



namespace Ui {
class ImportFinamForm;
}

class ImportFinamForm : public QWidget
{
    Q_OBJECT

private:
    //std::filesystem::directory_entry drCurr;
    std::filesystem::path pathFile;
    std::filesystem::path pathDir;
    char cDelimiter{','};

public:
    explicit ImportFinamForm(QWidget *parent = nullptr);
    ~ImportFinamForm();

public:
    void SetDefaultOpenDir(QString &s);
    void SetDelimiter(char c);

signals:
    void OpenImportFilePathChanged(QString &);
    void DelimiterHasChanged(char c);

private slots:

    void slotBtnOpenClicked();
    void slotBtnCreateClicked();
    void slotBtnTestClicked();
    void slotBtnImportClicked();
    void slotEditDelimiterWgtChanged(const QString &);

    void slotPreparseImportFile();

private:
    Ui::ImportFinamForm *ui;
};

#endif // IMPORTFINAMFORM_H

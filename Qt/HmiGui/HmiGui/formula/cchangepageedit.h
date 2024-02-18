#ifndef CCHANGEPAGEEDIT_H
#define CCHANGEPAGEEDIT_H

#include <QLineEdit>

class CChangePageEdit:public QLineEdit
{
    Q_OBJECT
public:
    CChangePageEdit(const int nTotalPage = 0, const int nCurrentPage = 0, QWidget *parent = nullptr);
    int setCurrentPage(const int nPage);
    int setTotalPage(const int nPage);
	int getCurrentPage();
	int getTotalPage();
	int UpdataEdit();
public slots:
    int finishPageEdit();
	int editTextChange();
signals:
	void OnEditFinish(int nCurPageNum);
protected:
	bool eventFilter(QObject *obj, QEvent *ev) override;
private:
	bool m_bIsEdit;
    int m_nTotalPage;
    int m_nCurrentPage;
};

#endif // CCHANGEPAGEEDIT_H

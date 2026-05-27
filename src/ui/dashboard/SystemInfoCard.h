#ifndef SYSTEMINFOCARD_H
#define SYSTEMINFOCARD_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class SystemInfoCard : public QWidget
{
    Q_OBJECT
    
public:
    explicit SystemInfoCard(const QString& title, const QString& iconName, QWidget *parent = nullptr);
    ~SystemInfoCard() override = default;

    void setValue(const QString& value);
    void setPercentage(int percentage);
    
private slots:
    void refreshInfo();
    
private:
    void createLayout();
    void setupWidget(const QString& title, const QString& iconName);
    
    QLabel *m_titleLabel;
    QLabel *m_iconLabel;
    QLabel *m_valueLabel;
    QLabel *m_percentageLabel;
};

#endif // SYSTEMINFOCARD_H
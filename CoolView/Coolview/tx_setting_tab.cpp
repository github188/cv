#include "tx_setting_tab.h"

#include "WeblibUploader/WeblibUIComponont.h"

TxSettingTab::TxSettingTab(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);
}

TxSettingTab::~TxSettingTab()
{

}

void TxSettingTab::Initialize()
{
  QWidget * weblib_setting_widget = WeblibUIComponont::getSettingWidget();
  if (!weblib_setting_widget) {
    return;
  }
  //TODO:Ϊ�˷����Ժ���չ���½���TX���ô��ڣ���ĿǰΪ�����ݣ�ֻ��ҪWeblib����
  layout()->addWidget(weblib_setting_widget);
}

#include "file_browser_widget.h"

#include <QFileSystemModel> 

#include "util\ini\TxConfig.h"
#include "util.h"


FileBrowserWidget::FileBrowserWidget(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);
}

FileBrowserWidget::~FileBrowserWidget()
{

}

void FileBrowserWidget::Initialize()
{
  QFileSystemModel *file_system_model = new QFileSystemModel;
  file_system_model->setReadOnly(true);

  ui.fileView->setModel(file_system_model);
  //ui.fileView->header()->setSortIndicator(0, Qt::AscendingOrder);
  //ui.fileView->header()->setSortIndicatorShown(true);
  //ui.fileView->header()->setSectionsClickable(true);
  ui.fileView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui.fileView->setSelectionBehavior(QAbstractItemView::SelectRows);

  //����Ŀ¼��Ϊ¼���ļ����λ�ã���ֹ�û��ҿ�
  library_path_ = GetAbsolutePath(
    QString::fromLocal8Bit(CTxConfig::getInstance().GetUploadPath()));
  MakeDirRecursive(qPrintable(library_path_)); //ȷ��·����Ч
  ui.fileView->setRootIndex(file_system_model->setRootPath(library_path_));

  connect(ui.fileView, &QTreeView::expanded,
    this, &FileBrowserWidget::HandleDirectoryExpandedSlot);
}

void FileBrowserWidget::ExpandToPath( 
  const QString &file )
{
  QFileSystemModel *file_system_model = dynamic_cast<QFileSystemModel *>(ui.fileView->model());
  if (!file_system_model) {
    return;
  }
  QModelIndex index = file_system_model->index(file);
  if (index.isValid()) {
    if (index.parent().isValid() && 
        ui.fileView->isExpanded(index.parent())) {
      ui.fileView->scrollTo(index); //ȷ���ɼ���ѡ��
      ui.fileView->selectionModel()->select(index, 
        QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    } else {
      //scrollTo�Ǵ��ϼ�Ŀ¼��ʼ����Ŀ¼��չ����������״�չ������QFileSystemModel��δ����Ŀ¼��
      //�ᵼ��ѡ����Ч����������Ҫ�Լ��Ӹ�Ŀ¼��ʼ��չ������
      //�����״μ���Ŀ¼������scrollTo��Ч������
      //ui.fileView->scrollTo(index);
      //��¼�Ա��ļ���չ����ѡ��
      index_to_select_ = index;
      QStack<QModelIndex> stack;
      while (index.isValid()) {
        stack.push(index);
        index = index.parent();
      }
      while (!stack.isEmpty()) {
        ui.fileView->expand(stack.pop());
      }
    }
  }
}

void FileBrowserWidget::HandleDirectoryExpandedSlot(const QModelIndex & index)
{
  if (index_to_select_.isValid() && index_to_select_.parent() == index) {
    ui.fileView->selectionModel()->select(index_to_select_, 
      QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    ui.fileView->scrollTo(index_to_select_);
    index_to_select_ = QModelIndex();
  }
}
